#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "py/mpconfig.h"
#include "py/builtin.h"
#include "py/compile.h"
#include "py/runtime.h"
#include "py/repl.h"
#include "py/gc.h"
#include "py/mperrno.h"
#include "shared/runtime/pyexec.h"
#include "extmod/vfs.h"
#include "extmod/vfs_posix.h"
#include "rpi.h"

// Command line options, with their defaults
bool mp_compile_only = false;

#if MICROPY_ENABLE_GC
// Heap size of GC heap (if enabled)
// Make it larger on a 64 bit machine, because pointers are larger.
long heap_size = 1024 * 1024 * (sizeof(mp_uint_t) / 4);
#endif

// Number of heaps to assign by default if MICROPY_GC_SPLIT_HEAP=1
#ifndef MICROPY_GC_SPLIT_HEAP_N_HEAPS
#define MICROPY_GC_SPLIT_HEAP_N_HEAPS (1)
#endif

static void stderr_print_strn(void *env, const char *str, size_t len) {
    (void)env;
    ssize_t ret;
    MP_HAL_RETRY_SYSCALL(ret, write(STDERR_FILENO, str, len), {});
}

const mp_print_t mp_stderr_print = {NULL, stderr_print_strn};

static inline int convert_pyexec_result(int ret) {
    #if MICROPY_PYEXEC_ENABLE_EXIT_CODE_HANDLING
    // With exit code handling enabled:
    // pyexec returns exit code with PYEXEC_FORCED_EXIT flag set for SystemExit
    // Unix port expects: 0 for success, non-zero for error/exit
    if (ret & PYEXEC_FORCED_EXIT) {
        // SystemExit: extract exit code from lower bits
        return ret & 0xFF;
    }
    // Normal execution or exception: return as-is (0 for success, 1 for exception)
    return ret;
    #else
    // pyexec returns 1 for success, 0 for exception, PYEXEC_FORCED_EXIT for SystemExit
    // Convert to unix port's expected codes: 0 for success, 1 for exception, FORCED_EXIT|val for SystemExit
    if (ret == 1) {
        return 0; // success
    } else if (ret & PYEXEC_FORCED_EXIT) {
        return ret; // SystemExit with exit value in lower 8 bits
    } else {
        return 1; // exception
    }
    #endif
}

static int do_file(const char *file) {
    return convert_pyexec_result(pyexec_file(file));
}

int do_str(const char *src) {
    int ret = 0;
    nlr_buf_t nlr;
    if (nlr_push(&nlr) == 0) {
        mp_lexer_t *lex = mp_lexer_new_from_str_len(MP_QSTR__lt_stdin_gt_, src, strlen(src), 0);
        qstr source_name = lex->source_name;
        mp_parse_tree_t parse_tree = mp_parse(lex, MP_PARSE_FILE_INPUT);
        mp_obj_t module_fun = mp_compile(&parse_tree, source_name, true);
        mp_call_function_0(module_fun);
        nlr_pop();
    } else {
        // uncaught exception
        mp_obj_print_exception(&mp_plat_print, (mp_obj_t)nlr.ret_val);
        ret = 1;
    }
    return ret;
}

static void print_help(char **argv) {
    printf(
        "usage: %s [<opts>] [-X <implopt>] [-c <command> | <filename>]\n"
        "Options:\n"
        "--version : show version information\n"
        "-h : print this help message\n"
        "-i : enable inspection via REPL after running command/module/file\n"
        "Implementation specific options (-X):\n"
        "  heapsize=<n>[w][K|M] -- set the heap size for the GC (default %ld)\n"
        , argv[0], heap_size);
}

static int invalid_args(void) {
    fprintf(stderr, "Invalid command line arguments. Use -h option for help.\n");
    return 1;
}

// Process options which set interpreter init options
static void pre_process_options(int argc, char **argv) {
    for (int a = 1; a < argc; a++) {
        if (argv[a][0] == '-') {
            if (strcmp(argv[a], "-h") == 0) {
                print_help(argv);
                exit(0);
            }
            if (strcmp(argv[a], "--version") == 0) {
                printf("micropython-1.29.0; AROS AARCH64\n");
                exit(0);
            }
            if (strcmp(argv[a], "-X") == 0) {
                if (a + 1 >= argc) {
                    exit(invalid_args());
                }
                if (0) {

                } else if (strncmp(argv[a + 1], "heapsize=", sizeof("heapsize=") - 1) == 0) {
                    char *end;
                    heap_size = strtol(argv[a + 1] + sizeof("heapsize=") - 1, &end, 0);
                    // Don't bring unneeded libc dependencies like tolower()
                    // If there's 'w' immediately after number, adjust it for
                    // target word size. Note that it should be *before* size
                    // suffix like K or M, to avoid confusion with kilowords,
                    // etc. the size is still in bytes, just can be adjusted
                    // for word size (taking 32bit as baseline).
                    bool word_adjust = false;
                    if ((*end | 0x20) == 'w') {
                        word_adjust = true;
                        end++;
                    }
                    if ((*end | 0x20) == 'k') {
                        heap_size *= 1024;
                    } else if ((*end | 0x20) == 'm') {
                        heap_size *= 1024 * 1024;
                    } else {
                        // Compensate for ++ below
                        --end;
                    }
                    if (*++end != 0) {
                        goto invalid_arg;
                    }
                    if (word_adjust) {
                        heap_size = heap_size * MP_BYTES_PER_OBJ_WORD / 4;
                    }
                    // If requested size too small, we'll crash anyway
                    if (heap_size < 700) {
                        goto invalid_arg;
                    }
                } else {
                invalid_arg:
                    exit(invalid_args());
                }
                a++;
            }
        } else {
            break; // Not an option but a file
        }
    }
}

#define PATHLIST_SEP_CHAR ';'

MP_NOINLINE int main_(int argc, char **argv);

int main(int argc, char **argv) {
    #if MICROPY_PY_THREAD
    mp_thread_init();
    #endif

    // Define a reasonable stack limit to detect stack overflow.
    mp_uint_t stack_size = 40000 * UNIX_STACK_MULTIPLIER;

    // We should capture stack top ASAP after start, and it should be
    // captured guaranteedly before any other stack variables are allocated.
    // For this, actual main (renamed main_) should not be inlined into
    // this function. main_() itself may have other functions inlined (with
    // their own stack variables), that's why we need this main/main_ split.
    mp_cstack_init_with_sp_here(stack_size);
    return main_(argc, argv);
}

MP_NOINLINE int main_(int argc, char **argv) {
    pre_process_options(argc, argv);

    #if !MICROPY_GC_SPLIT_HEAP
    char *heap = malloc(heap_size);
    gc_init(heap, heap + heap_size);
    #else
    assert(MICROPY_GC_SPLIT_HEAP_N_HEAPS > 0);
    char *heaps[MICROPY_GC_SPLIT_HEAP_N_HEAPS];
    long multi_heap_size = heap_size / MICROPY_GC_SPLIT_HEAP_N_HEAPS;
    for (size_t i = 0; i < MICROPY_GC_SPLIT_HEAP_N_HEAPS; i++) {
        heaps[i] = malloc(multi_heap_size);
        if (i == 0) {
            gc_init(heaps[i], heaps[i] + multi_heap_size);
        } else {
            gc_add(heaps[i], heaps[i] + multi_heap_size);
        }
    }
    #endif

    #if MICROPY_ENABLE_PYSTACK
    static mp_obj_t pystack[1024];
    mp_pystack_init(pystack, &pystack[MP_ARRAY_SIZE(pystack)]);
    #endif

    mp_init();
    rpi_init();

    #if MICROPY_VFS_POSIX
    {
        // Mount the host FS at the root of our internal VFS
        mp_obj_t args[2] = {
            MP_OBJ_TYPE_GET_SLOT(&mp_type_vfs_posix, make_new)(&mp_type_vfs_posix, 0, 0, NULL),
            MP_OBJ_NEW_QSTR(MP_QSTR__slash_),
        };
        mp_vfs_mount(2, args, (mp_map_t *)&mp_const_empty_map);

        // Make sure the root that was just mounted is the current VFS (it's always at
        // the end of the linked list).  Can't use chdir('/') because that will change
        // the current path within the VfsPosix object.
        MP_STATE_VM(vfs_cur) = MP_STATE_VM(vfs_mount_table);
        while (MP_STATE_VM(vfs_cur)->next != NULL) {
            MP_STATE_VM(vfs_cur) = MP_STATE_VM(vfs_cur)->next;
        }
    }
    #endif

    {
        // sys.path starts as [""]
        mp_sys_path = mp_obj_new_list(0, NULL);
        mp_obj_list_append(mp_sys_path, MP_OBJ_NEW_QSTR(MP_QSTR_));

        // Add colon-separated entries from MICROPYPATH.
        char *path = getenv("MICROPYPATH");
        if (path == NULL) {
            path = MICROPY_PY_SYS_PATH_DEFAULT;
        }
        if (*path == PATHLIST_SEP_CHAR) {
            // First entry is empty. We've already added an empty entry to sys.path, so skip it.
            ++path;
        }
        // GCC targeting RISC-V 64 reports a warning about `path_remaining` being clobbered by
        // either setjmp or vfork if that variable it is allocated on the stack.  This may
        // probably be a compiler error as it occurs on a few recent GCC releases (up to 14.1.0)
        // but LLVM doesn't report any warnings.
        static bool path_remaining;
        path_remaining = *path;
        while (path_remaining) {
            char *path_entry_end = strchr(path, PATHLIST_SEP_CHAR);
            if (path_entry_end == NULL) {
                path_entry_end = path + strlen(path);
                path_remaining = false;
            }
            mp_obj_list_append(mp_sys_path, mp_obj_new_str_via_qstr(path, path_entry_end - path));
            path = path_entry_end + 1;
        }
    }

    const int NOTHING_EXECUTED = -2;
    int ret = NOTHING_EXECUTED;
    bool inspect = false;
    for (int a = 1; a < argc; a++) {
        if (argv[a][0] == '-') {
            if (strcmp(argv[a], "-i") == 0) {
                inspect = true;
            } else if (strcmp(argv[a], "-c") == 0) {
                if (a + 1 >= argc) {
                    return invalid_args();
                }
                ret = do_str(argv[a + 1]);
                break;
            }
        } else {
            char *basedir = realpath(argv[a], NULL);
            if (basedir == NULL) {
                mp_printf(&mp_stderr_print, "%s: can't open file '%s': [Errno %d] %s\n", argv[0], argv[a], errno, strerror(errno));
                // CPython exits with 2 in such case
                ret = 2;
                break;
            }

            // Set base dir of the script as first entry in sys.path.
            char *p = strrchr(basedir, '/');
            mp_obj_list_store(mp_sys_path, MP_OBJ_NEW_SMALL_INT(0), mp_obj_new_str_via_qstr(basedir, p - basedir));
            free(basedir);

            ret = do_file(argv[a]);
            break;
         }
    }

    if (ret == NOTHING_EXECUTED || inspect) {
        ret = pyexec_friendly_repl();
    }

    #if MICROPY_PY_SYS_ATEXIT
    // Beware, the sys.settrace callback should be disabled before running sys.atexit.
    if (mp_obj_is_callable(MP_STATE_VM(sys_exitfunc))) {
        mp_call_function_0(MP_STATE_VM(sys_exitfunc));
    }
    #endif

    rpi_deinit();
    mp_deinit();

     #if MICROPY_ENABLE_GC && !defined(NDEBUG)
    // We don't really need to free memory since we are about to exit the
    // process, but doing so helps to find memory leaks.
    #if !MICROPY_GC_SPLIT_HEAP
    free(heap);
    #else
    for (size_t i = 0; i < MICROPY_GC_SPLIT_HEAP_N_HEAPS; i++) {
        free(heaps[i]);
    }
    #endif
    #endif

    return ret & 0xff;
}

mp_lexer_t *mp_lexer_new_from_file(qstr filename) {
    mp_raise_OSError(MP_ENOENT);
}

void nlr_jump_fail(void *val) {
    fprintf(stderr, "FATAL: uncaught NLR %p\n", val);
    exit(1);
}

#ifndef NDEBUG
void MP_WEAK __assert_func(const char *file, int line, const char *func, const char *expr) {
    printf("Assertion '%s' failed, at file %s:%d\n", expr, file, line);
    __fatal_error("Assertion failed");
}
#endif

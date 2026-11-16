#include <stdint.h>
#include <stddef.h>

#ifndef MICROPY_PY_SYS_PATH_DEFAULT
#define MICROPY_PY_SYS_PATH_DEFAULT "ENVARC:Micropython"
#endif

// options to control how MicroPython is built

// Use the minimal starting configuration (disables all optional features).
//#define MICROPY_CONFIG_ROM_LEVEL (MICROPY_CONFIG_ROM_LEVEL_MINIMUM)

// Disable native emitters.
#define MICROPY_EMIT_X86    (0)
#define MICROPY_EMIT_X64    (0)
#define MICROPY_EMIT_THUMB  (0)
#define MICROPY_EMIT_ARM    (0)
#define MICROPY_EMIT_RV32   (0)

#define MICROPY_PERSISTENT_CODE_LOAD        (1)
#define MICROPY_COMP_MODULE_CONST           (1)
#define MICROPY_COMP_TRIPLE_TUPLE_ASSIGN    (1)
#define MICROPY_COMP_RETURN_IF_EXPR         (1)
#define MICROPY_ENABLE_GC                   (1)
#define MICROPY_GCREGS_SETJMP               (1)
#define MICROPY_ENABLE_FINALISER            (1)
#define MICROPY_ENABLE_PYSTACK              (1)
#define MICROPY_STACK_CHECK                 (1)
#define MICROPY_MALLOC_USES_ALLOCATED_SIZE  (1)
#define MICROPY_MEM_STATS                   (1)
#define MICROPY_HELPER_REPL                 (1)
#define MICROPY_REPL_EMACS_KEYS             (1)
#define MICROPY_REPL_AUTO_INDENT            (1)
#define MICROPY_HELPER_LEXER_UNIX           (1)
#define MICROPY_ENABLE_SOURCE_LINE          (1)
#define MICROPY_ENABLE_EXTERNAL_IMPORT      (1)
#define MICROPY_ALLOC_PATH_MAX              (PATH_MAX)
#ifndef MICROPY_FLOAT_IMPL
#define MICROPY_FLOAT_IMPL                  (MICROPY_FLOAT_IMPL_DOUBLE)
#endif
#define MICROPY_LONGINT_IMPL                (MICROPY_LONGINT_IMPL_MPZ)
#define MICROPY_READER_POSIX                (1)
#define MICROPY_READER_VFS                  (1)
#ifndef MICROPY_STREAMS_NON_BLOCK
#define MICROPY_STREAMS_NON_BLOCK           (1)
#endif
#define MICROPY_STREAMS_POSIX_API           (1)
#define MICROPY_VFS                         (1)
#define MICROPY_VFS_POSIX                   (1)
#ifndef MICROPY_ENABLE_SCHEDULER
#define MICROPY_ENABLE_SCHEDULER    (1)
#endif
#define MICROPY_PY_FUNCTION_ATTRS           (1)
#define MICROPY_PY_DESCRIPTORS              (1)
#define MICROPY_PY_DELATTR_SETATTR          (1)
#define MICROPY_PY_FSTRINGS                 (1)
#define MICROPY_PY_BUILTINS_BYTES_HEX       (1)
#define MICROPY_PY_BUILTINS_STR_UNICODE     (1)
#define MICROPY_PY_BUILTINS_STR_CENTER      (1)
#define MICROPY_PY_BUILTINS_STR_PARTITION   (1)
#define MICROPY_PY_BUILTINS_STR_SPLITLINES  (1)
#define MICROPY_PY_BUILTINS_MEMORYVIEW      (1)
#define MICROPY_PY_BUILTINS_FROZENSET       (1)
#define MICROPY_PY_BUILTINS_COMPILE         (1)
#define MICROPY_PY_BUILTINS_NOTIMPLEMENTED  (1)
#define MICROPY_PY_BUILTINS_INPUT           (1)
#define MICROPY_PY_BUILTINS_POW3            (1)
#ifndef MICROPY_PY_BUILTINS_HELP
#define MICROPY_PY_BUILTINS_HELP            (1)
#endif
#ifndef MICROPY_PY_BUILTINS_HELP_MODULES
#define MICROPY_PY_BUILTINS_HELP_MODULES    (1)
#endif
#define MICROPY_PY_BUILTINS_ROUND_INT       (1)
#define MICROPY_PY_MICROPYTHON_MEM_INFO     (1)
#define MICROPY_PY_ALL_SPECIAL_METHODS      (1)
#define MICROPY_PY_REVERSE_SPECIAL_METHODS  (1)
#define MICROPY_PY_ARRAY_SLICE_ASSIGN       (1)
#define MICROPY_PY_BUILTINS_SLICE_ATTRS     (1)
#define MICROPY_PY_SYS_PATH_ARGV_DEFAULTS   (1) // Crash if 0
#define MICROPY_PY_SYS_EXIT                 (1)
#define MICROPY_PY_SYS_ATEXIT               (1)
#define MICROPY_PY_SYS_PLATFORM             "AROS"
#define MICROPY_PY_SYS_MAXSIZE              (1)
#define MICROPY_PY_SYS_STDFILES             (1)
#define MICROPY_PY_SYS_EXC_INFO             (1)
#define MICROPY_PY_COLLECTIONS_DEQUE        (1)
#define MICROPY_PY_COLLECTIONS_DEQUE_ITER   (1)
#define MICROPY_PY_COLLECTIONS_DEQUE_SUBSCR (1)
#define MICROPY_PY_COLLECTIONS_ORDEREDDICT  (1)
#ifndef MICROPY_PY_MATH_SPECIAL_FUNCTIONS
#define MICROPY_PY_MATH_SPECIAL_FUNCTIONS   (1)
#endif
#define MICROPY_PY_MATH_ISCLOSE             (MICROPY_PY_MATH_SPECIAL_FUNCTIONS)
#define MICROPY_PY_CMATH                    (1)
#define MICROPY_PY_IO_IOBASE                (1)
#define MICROPY_PY_GC_COLLECT_RETVAL        (1)
#ifndef MICROPY_STACKLESS
#define MICROPY_STACKLESS                   (0)
#define MICROPY_STACKLESS_STRICT            (0)
#endif

#define MICROPY_PY_OS                       (1)
#define MICROPY_PY_OS_INCLUDEFILE           "ports/unix/modos.c"
#define MICROPY_PY_OS_ERRNO                 (1)
#define MICROPY_PY_OS_STATVFS               (1)
#define MICROPY_PY_OS_SYSTEM                (1)
#define MICROPY_PY_OS_URANDOM               (1)
#define MICROPY_PY_ERRNO                    (1)
#define MICROPY_PY_UCTYPES                  (1)
#define MICROPY_PY_DEFLATE                  (1)
#define MICROPY_PY_DEFLATE_COMPRESS         (1)
#define MICROPY_PY_JSON                     (1)
#define MICROPY_PY_RE                       (1)
#define MICROPY_PY_HEAPQ                    (1)
#define MICROPY_PY_HASHLIB                  (1)
#define MICROPY_PY_BINASCII                 (1)
#define MICROPY_PY_BINASCII_CRC32           (1)
#define MICROPY_PY_RANDOM                   (1)
#define MICROPY_PY_RANDOM_EXTRA_FUNCS       (1)
#define MICROPY_PY_SOCKET                   (1)
#define MICROPY_PY_TIME                     (1)
#define MICROPY_PY_TIME_CUSTOM_SLEEP        (1)
#define MICROPY_PY_TIME_TIME_TIME_NS        (0)
#define MICROPY_PY_TIME_INCLUDEFILE         "../modtime.c"
#define MICROPY_PY_MACHINE                  (1)
#define MICROPY_PY_MACHINE_INCLUDEFILE      "../modmachine.c"
#define MICROPY_PY_MACHINE_PIN_MAKE_NEW     mp_pin_make_new
#define MICROPY_PY_MACHINE_UART             (1)
#define MICROPY_PY_MACHINE_UART_INCLUDEFILE "../machine_uart.c"
#define MICROPY_PY_MACHINE_PWM              (1)
#define MICROPY_PY_MACHINE_PWM_INCLUDEFILE  "../machine_pwm.c"
#define MICROPY_PY_ONEWIRE                  (1)

#define MICROPY_PY_BUILTINS_HELP            (1)
#define MICROPY_PY_BUILTINS_HELP_MODULES    (1)


// Enable handling of sys.exit() exit codes.
#define MICROPY_PYEXEC_ENABLE_EXIT_CODE_HANDLING (1)

// VFS stat functions should return time values relative to 1970/1/1
#define MICROPY_EPOCH_IS_1970       (1)

// Bare-metal ports don't have stderr. Printing debug to stderr may give tests
// which check stdout a chance to pass, etc.
extern const struct _mp_print_t mp_stderr_print;
#define MICROPY_DEBUG_PRINTER (&mp_stderr_print)
#define MICROPY_ERROR_PRINTER (&mp_stderr_print)

// This macro is used to implement PEP 475 to retry specified syscalls on EINTR
#define MP_HAL_RETRY_SYSCALL(ret, syscall, raise) { \
        for (;;) { \
            MP_THREAD_GIL_EXIT(); \
            ret = syscall; \
            MP_THREAD_GIL_ENTER(); \
            if (ret == -1) { \
                int err = errno; \
                if (err == EINTR) { \
                    mp_handle_pending(MP_HANDLE_PENDING_CALLBACKS_AND_EXCEPTIONS); \
                    continue; \
                } \
                raise; \
            } \
            break; \
        } \
}

// type definitions for the specific machine
typedef long mp_off_t;

// We need to provide a declaration/definition of alloca()
#define alloca(x) __builtin_alloca(x)

// Missing prototype
char *getcwd(char *buf, size_t size);
char *realpath(const char *path, char *resolved_path);
int setenv(const char *, const char *, int);
int unsetenv(const char *);

#define MICROPY_MIN_USE_STDOUT  (1)
#define MICROPY_HEAP_SIZE       (1000*1024) // heap size  kilobytes
#define UNIX_STACK_MULTIPLIER   (sizeof(void *) / 4)

#define MP_STATE_PORT MP_STATE_VM

#define MICROPY_VFS_ROM                (1)
#define MICROPY_VFS_ROM_IOCTL          (0)

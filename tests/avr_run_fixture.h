#ifndef AVR_RUN_FIXTURE_H
#define AVR_RUN_FIXTURE_H

/// @brief mkdtemp/usleep/kill/PATH_MAX need _POSIX_C_SOURCE (>= 200809L)
/// visible to glibc under -std=c11. glibc locks in feature-test visibility
/// at the first system header it processes in the translation unit, so
/// defining it here is too late if the including .c file pulls in tst.h
/// (-> <stdlib.h>) first, as is convention. Define it at the very top of
/// the .c file, before any #include, if you hit implicit-declaration
/// warnings/errors here.

/// @brief Runs a full pipeline external to the compiler binary: source ->
/// compile_fixture.h -> `avra` (assemble) -> `simavr` (simulate, with a gdb
/// stub) -> `avr-gdb` (run to a breakpoint and read the return register).
/// Requires `avra`, `simavr` and `avr-gdb` on PATH.
///
/// Unlike compile_fixture.h's compile_to_asm(), which only checks the
/// mnemonics gen_avr emits, this actually executes the program and checks
/// the real result - it can catch codegen bugs that produce plausible-looking
/// but wrong assembly.

#include "compile_fixture.h"

#include <fcntl.h>
#include <limits.h>
#include <signal.h>
#include <stdbool.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/wait.h>

#define AVR_RUN_MCU "atmega16"
#define AVR_RUN_FREQ_HZ "8000000"
#define AVR_RUN_GDB_PORT 1234

/// @brief Byte address gdb breaks at. gen_avr always emits a fixed __init
/// prologue before any user code: 6 one-word setup instructions, a 2-word
/// `call main`, and a 1-word `out PORTB, r<ret_reg>` - i.e. `main`'s return
/// value is in the return register at word address 9, regardless of what
/// `main` itself compiles to. AVR program memory is word-addressed in
/// hardware but gdb addresses it in bytes, hence *2.
#define AVR_RUN_EXIT_BYTE_ADDR 0x12

struct AvrRunResult {
    /// @brief false if any pipeline stage (compile/assemble/simulate/debug)
    /// failed; see stderr for diagnostics.
    bool ok;
    /// @brief value of the return register once `main` has returned. Only
    /// meaningful if `ok`.
    int retval;
};

/// @brief Runs `argv[0]`, capturing combined stdout+stderr into `out` (if
/// non-NULL) and blocking until it exits. Returns its exit code, or -1 if it
/// didn't exit normally.
static int avr_run_capture(char* const argv[], char* out, size_t out_cap)
{
    int pipefd[2];
    if (pipe(pipefd) != 0)
        return -1;

    pid_t pid = fork();
    if (pid == 0) {
        close(pipefd[0]);
        dup2(pipefd[1], STDOUT_FILENO);
        dup2(pipefd[1], STDERR_FILENO);
        close(pipefd[1]);
        execvp(argv[0], argv);
        _exit(127);
    }
    close(pipefd[1]);

    size_t len = 0;
    char discard[256];
    ssize_t n;
    while ((n = read(pipefd[0], discard, sizeof(discard))) > 0) {
        if (out != NULL && len + 1 < out_cap) {
            size_t copy = (size_t)n;
            if (copy > out_cap - 1 - len)
                copy = out_cap - 1 - len;
            memcpy(out + len, discard, copy);
            len += copy;
        }
    }
    if (out != NULL && out_cap > 0)
        out[len] = '\0';
    close(pipefd[0]);

    int status;
    waitpid(pid, &status, 0);
    return WIFEXITED(status) ? WEXITSTATUS(status) : -1;
}

/// @brief Runs `argv[0]` in the background (stdout+stderr redirected to
/// `log_path`) and returns its pid without waiting for it to exit.
static pid_t avr_run_spawn_background(char* const argv[], const char* log_path)
{
    pid_t pid = fork();
    if (pid == 0) {
        int fd = open(log_path, O_WRONLY | O_CREAT | O_TRUNC, 0644);
        if (fd >= 0) {
            dup2(fd, STDOUT_FILENO);
            dup2(fd, STDERR_FILENO);
            close(fd);
        }
        execvp(argv[0], argv);
        _exit(127);
    }
    return pid;
}

static bool avr_run_wait_for_port(int port, int timeout_ms)
{
    for (int waited = 0; waited < timeout_ms; waited += 20) {
        int s = socket(AF_INET, SOCK_STREAM, 0);
        if (s < 0)
            return false;

        struct sockaddr_in addr = {0};
        addr.sin_family = AF_INET;
        addr.sin_port = htons((unsigned short)port);
        addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);

        bool connected = connect(s, (struct sockaddr*)&addr, sizeof(addr)) == 0;
        close(s);
        if (connected)
            return true;
        nanosleep(&(struct timespec){ .tv_nsec = 20 * 1000 * 1000 }, NULL);
    }
    return false;
}

static void avr_run_dump_log(const char* path)
{
    FILE* f = fopen(path, "r");
    if (f == NULL)
        return;
    char buf[4096];
    size_t n;
    while ((n = fread(buf, 1, sizeof(buf), f)) > 0)
        fwrite(buf, 1, n, stderr);
    fclose(f);
}

static struct AvrRunResult run_avr(const char* source)
{
    struct AvrRunResult result = { .ok = false, .retval = 0 };
    pid_t sim_pid = -1;
    char dir[] = "/tmp/sprache_avr_XXXXXX";

    struct CompileResult cr = compile_source(source);
    if (has_errors(&cr.errors) || cr.ir_head == NULL) {
        fprintf(stderr, "run_avr: compilation failed for: %s\n", source);
        free_compile_result(&cr);
        return result;
    }

    char* asm_text = compile_to_asm(cr.ir_head);
    free_compile_result(&cr);
    if (asm_text == NULL) {
        fprintf(stderr, "run_avr: codegen produced no output\n");
        return result;
    }

    if (mkdtemp(dir) == NULL) {
        fprintf(stderr, "run_avr: mkdtemp failed\n");
        free(asm_text);
        return result;
    }

    char asm_path[PATH_MAX], hex_path[PATH_MAX], sim_log[PATH_MAX];
    snprintf(asm_path, sizeof(asm_path), "%s/prog.asm", dir);
    snprintf(hex_path, sizeof(hex_path), "%s/prog.hex", dir);
    snprintf(sim_log, sizeof(sim_log), "%s/simavr.log", dir);

    FILE* f = fopen(asm_path, "w");
    if (f == NULL) {
        fprintf(stderr, "run_avr: could not write %s\n", asm_path);
        free(asm_text);
        goto cleanup_dir;
    }
    fputs(asm_text, f);
    fclose(f);
    free(asm_text);

    // assemble
    {
        char avra_out[2048];
        char* avra_argv[] = { "avra", "-o", hex_path, asm_path, NULL };
        if (avr_run_capture(avra_argv, avra_out, sizeof(avra_out)) != 0) {
            fprintf(stderr, "run_avr: avra failed:\n%s\n", avra_out);
            goto cleanup_dir;
        }
    }

    // simulate, with a gdb stub listening on AVR_RUN_GDB_PORT
    {
        char* simavr_argv[] = {
            "simavr", "-g", "-m", AVR_RUN_MCU, "-f", AVR_RUN_FREQ_HZ, hex_path, NULL
        };
        sim_pid = avr_run_spawn_background(simavr_argv, sim_log);
        if (!avr_run_wait_for_port(AVR_RUN_GDB_PORT, 2000)) {
            fprintf(stderr, "run_avr: simavr never opened the gdb port:\n");
            avr_run_dump_log(sim_log);
            goto cleanup_dir;
        }
    }

    // debug: run to the fixed breakpoint and read the return register
    {
        char target_cmd[32], break_cmd[32], print_cmd[32];
        snprintf(target_cmd, sizeof(target_cmd), "target remote :%d", AVR_RUN_GDB_PORT);
        snprintf(break_cmd, sizeof(break_cmd), "break *0x%x", AVR_RUN_EXIT_BYTE_ADDR);
        snprintf(print_cmd, sizeof(print_cmd), "print/x $r%u", target.ret_reg);

        char* gdb_argv[] = {
            "avr-gdb", "-q", "-batch",
            "-ex", "set pagination off",
            "-ex", "set confirm off",
            "-ex", target_cmd,
            "-ex", break_cmd,
            "-ex", "continue",
            "-ex", print_cmd,
            "-ex", "kill",
            NULL
        };
        char gdb_out[2048];
        avr_run_capture(gdb_argv, gdb_out, sizeof(gdb_out));

        const char* marker = "$1 = 0x";
        const char* p = strstr(gdb_out, marker);
        if (p == NULL) {
            fprintf(stderr, "run_avr: could not read return register from gdb output:\n%s\n", gdb_out);
            goto cleanup_dir;
        }
        result.retval = (int)strtol(p + strlen(marker), NULL, 16);
        result.ok = true;
    }

cleanup_dir:
    if (sim_pid > 0) {
        kill(sim_pid, SIGTERM);
        waitpid(sim_pid, NULL, 0);
    }
    {
        char* rm_argv[] = { "rm", "-rf", dir, NULL };
        avr_run_capture(rm_argv, NULL, 0);
    }
    return result;
}

/// @brief Asserts that compiling and actually executing `source` on the
/// simulator leaves `expected` in the return register.
#define TST_ASSERT_AVR_RETURNS(expected, source) do { \
    struct AvrRunResult _tst_avr = run_avr(source); \
    TST_ASSERT(_tst_avr.ok); \
    if (_tst_avr.ok) \
        TST_ASSERT_EQ(expected, _tst_avr.retval); \
} while (0)

#endif

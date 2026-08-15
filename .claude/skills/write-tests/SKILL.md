---
name: write-tests
description: Use when adding test coverage to Sprache - a new compiler feature, a bugfix, or filling a gap in existing coverage. Explains the two test tiers (unit vs. integration), the custom tst.h assertion framework, the fixtures each tier builds on, and where new test files go so `make test` picks them up automatically.
---

# Writing tests for Sprache

Sprache has no external test framework - `tests/tst.h` is a small self-rolled one
(assert macros + a runner). Tests are plain `.c` files under `tests/`, auto-discovered
by the makefile (`find tests -name "*_tests.c"`) and compiled each into its own binary
linked against every lib source file (`makefile:55-57`). No registration step beyond
listing the test functions in that file's own `main()`.

Two tiers exist, picked by **what you're actually testing**, not by convention alone:

| | Unit (`tests/unit/<module>/`) | Integration (`tests/integration/`) |
|---|---|---|
| Exercises | one stage/data structure in isolation | the full pipeline: source -> ... -> AVR |
| Input | hand-built `IRInstr`/`InterfGraph` structs | Sprache source strings |
| Needs external tools | no | yes: `avra`, `simavr`, `avr-gdb` on `PATH` |
| Good for | algorithm edge cases (interference, spilling, offset math) | precedence, end-to-end correctness, "does the program actually compute the right value" |

If you're testing a single function/algorithm's logic (e.g. does `create_interf_graph`
find the right overlaps, does `regalloc` spill correctly), write a **unit** test. If
you're testing that a language feature produces the right runtime behavior, write an
**integration** test. Reference examples: `tests/unit/regalloc/regalloc_tests.c` and
`tests/unit/regalloc/interf_graph_tests.c` (unit), `tests/integration/arithmetic_tests.c`
(integration).

## `tst.h` assertion macros

- `TST_ASSERT_EQ(expected, actual)` - typed equality (`__typeof__(actual)`), prints
  both values on failure.
- `TST_ASSERT(cond)` - boolean check, prints the failing expression source.
- `TST_ASSERT_STR_CONTAINS(haystack, needle)` - substring check, prints the haystack
  on failure. Useful for asserting a specific mnemonic/operand appears in emitted asm
  without needing to run it (see "Checking emitted asm without simulating" below).
- `TST_RUN(fn)` - runs one test function, tracks pass/fail, prints per-test result.
- `TST_SUMMARY()` - prints the run/failed totals; call once at the end of `main()`.

Every test file follows this shape:

```c
#include "tst.h"
// ... fixture/module includes ...

static void some_behavior_produces_expected_result()
{
    // arrange, act, TST_ASSERT_*
}

int main()
{
    TST_RUN(some_behavior_produces_expected_result);
    // ... one TST_RUN per test function ...
    TST_SUMMARY();
    return 0;
}
```

Name test functions as a full sentence describing the behavior
(`regalloc_when_four_vregs_mutually_interfere_uses_all_pregs`,
`arithmetic_respects_operator_precedence`), not `test1`/`test_foo`. Put a short comment
above the test showing the IR or source being exercised - every existing test does this
and it's what makes the assertions below legible without re-deriving the scenario.

## Unit tests (`tests/unit/<module>/<module>_tests.c`)

Build the data structure under test directly, bypassing the parser/IR generator - e.g.
`regalloc_tests.c` constructs `IRInstr` lists by hand with `new_instr(...)` and manual
`IROperand` literals (`.type = OPRND_REG, .reg.regalloc_done = false, .reg.vreg_i = N`),
chains them via `->next`, then calls the function under test directly (`regalloc(a)`,
`create_interf_graph(head)`). This is the right level when you need precise control over
an edge case (e.g. "vreg redefined mid-list", "exactly 4 mutually-interfering vregs") that
would be awkward or indirect to provoke from source text.

Free whatever you allocated at the end of each test (`free_irlist(head)`,
`free_interf_graph(g)`) - these tests link against the real allocator/xalloc code, not a
mock, so leaks are real leaks.

Put the file in a subdirectory named after the module (`tests/unit/regalloc/`,
mirroring `src/backend/codegen/regalloc.c`) - a new module gets a new subdirectory, an
addition to an existing module's coverage goes in its existing `*_tests.c` (or a new
`*_tests.c` alongside it, as `interf_graph_tests.c` sits next to `regalloc_tests.c` to
separate graph-construction tests from allocation tests).

## Integration tests (`tests/integration/<topic>_tests.c`)

Two fixtures, both under `tests/` (included as `"integration/avr_run_fixture.h"` /
`"compile_fixture.h"` since the makefile adds `-I$(TEST_DIR)`):

- **`compile_fixture.h`** - runs source through tokenizer -> parser -> sema -> IR
  (`gen_ir_from_source`), mirroring `main.c`'s pipeline up to (not including) regalloc.
  `gen_avr_from_result(result)` then runs codegen and returns the emitted assembly as a
  heap string (caller frees it). Always free the compile result with
  `free_compile_result(&r)`.
- **`avr_run_fixture.h`** (includes `compile_fixture.h`) - takes it further: assembles
  with `avra`, boots `simavr` with a gdb stub, and uses `avr-gdb` to run to a fixed
  breakpoint and read the return register. Exposes
  `TST_ASSERT_AVR_RETURNS(expected, source)`, which is what every existing integration
  test uses:

  ```c
  #define _POSIX_C_SOURCE 200809L   // must be the very first line - see the comment
                                      // at the top of avr_run_fixture.h for why
  #include "tst.h"
  #include "integration/avr_run_fixture.h"

  static void addition_returns_expected_value()
  {
      TST_ASSERT_AVR_RETURNS(5, "fn main() { return 2 + 3; }");
  }
  ```

  `_POSIX_C_SOURCE 200809L` must be defined before *any* include, including `tst.h`,
  because glibc locks feature-test visibility at the first system header it sees - copy
  this line verbatim at the top of any new file that includes `avr_run_fixture.h`.

### Checking emitted asm without simulating

`TST_ASSERT_AVR_RETURNS` actually executes the program - it catches real codegen bugs
but is slower and needs `avra`/`simavr`/`avr-gdb` installed. If you only need to confirm
*which instruction* got emitted (e.g. a new operator picked the right AVR mnemonic, or a
spilled vreg produced an `std`/`ldd` pair), use `compile_fixture.h` directly and check
the text instead:

```c
#include "tst.h"
#include "compile_fixture.h"
#include "backend/codegen/regalloc.h"

static void new_op_emits_expected_mnemonic()
{
    struct CompileResult r = gen_ir_from_source("fn main() { return 5 <op> 3; }");
    for (struct IRFunc* f = r.ir_head; f != NULL; f = f->next)
        regalloc(f);
    char* asm_text = gen_avr_from_result(r);

    TST_ASSERT_STR_CONTAINS(asm_text, "<mnemonic> r");

    free(asm_text);
    free_compile_result(&r);
}
```

No existing integration test uses this pattern yet (they all run the simulator), but
`compile_fixture.h`'s doc comment is written for exactly this - reach for it when
`TST_ASSERT_AVR_RETURNS` is more than you need, or when the environment doesn't have
`avra`/`simavr`/`avr-gdb` available.

Pick the file by *language feature*, not by pipeline stage - `arithmetic_tests.c`,
`bitwise_tests.c`, `variable_tests.c`, `function_call_tests.c` each group tests by what
a Sprache user would recognize as one feature area. Add a new file for a new feature
area; add a test function to an existing one for more coverage of the same area.

## Running

```sh
make test
```

Builds every `tests/**/*_tests.c` into its own binary (`build/tests/...`) and runs them
all. A new file needs no makefile changes - it's picked up by the `find` glob
automatically. To run just one file's binary directly during iteration:

```sh
make test TEST_DIR=tests   # or build the single binary target directly:
gcc -Wall -Wextra -std=c11 -g -Iinclude -Itests tests/unit/regalloc/regalloc_tests.c $(find src -name "*.c" ! -name main.c) -o /tmp/t && /tmp/t
```

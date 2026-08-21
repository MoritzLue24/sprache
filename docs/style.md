# Sprache — Code Style

The binding style guide for all C code in `src/`, `include/` and `tests/`.

**There is no formatter.** No `.clang-format`, no `make format`, no
`make format-check`. Every rule in this document — including the whitespace ones
— is written by hand and checked by reading the diff.

Two parts:

| Part | Covers |
| --- | --- |
| **[Part 1](#part-1--formatting)** §1–§5 | Braces, blank lines, initialisers, wrapping — the mechanical layer |
| **[Part 2](#part-2--conventions)** §6–§15 | Naming, headers, X-macros, memory, error handling, tests |

---

# Part 1 — Formatting

## 1. Braces and indentation

| Aspect | Rule |
| --- | --- |
| Brace style | Stroustrup |
| Indent width | 4 spaces, never tabs |
| Column limit | 80, hard, comments included |
| Preprocessor `#` | always column 1, even inside a function or switch |
| Case labels | indented one level inside the `switch` |
| File ending | exactly one trailing newline |

After *every* curly brace follows either a newline or a space:
```c
int a[] = {1, 2, 3};  // not allowed
int a[] = { 1, 2, 3 }; // allowed
```

Stroustrup means: opening brace on the same line for `if` / `for` / `while` /
`switch` / `struct` / `enum`, on its own line for function bodies, and `else` /
`else if` start on a new line after the closing brace.

```c
void f(void)
{
    if (a) {
        ...
    }
    else {
        ...
    }
}
```

Switch — a one-line body may share the label's line; anything longer goes below
it:

```c
switch (t) {
    case TK_IDENT: return "ident";
    case TK_NUMBER:
        emit_number(tok);
        return "number";
}
```

Same goes for one-line if statements, but its not necessary:
```c
if (strcmp(a, b) == 0) return a;
```

(On why there is no `default:` here, see §9.)

### 1.1 Braces are mandatory

Every `if`, `else`, `for`, `while` and `do` body gets braces — no exceptions,
not even for a single statement.

```c
// WRONG
if (strcmp(a, b) == 0)
    return a;

// CORRECT
if (strcmp(a, b) == 0) {
    return a;
}
```

## 2. Blank lines

- Never more than one consecutive blank line.
- No blank line at the start of a block or at the start of the file.
- One blank line after the include guard.
- One blank line after the include block; no blank lines *between* includes.
- One blank line after every `struct` definition.
- One blank line after every function definition (not declaration).
- No blank line between consecutive function declarations — but only when
  neither carries a comment. As soon as theres a comment between the 
  declarations, separate the them.

```c
#ifndef TOKENS_H
#define TOKENS_H

#include "lexer/tokens.h"
#include <stddef.h>

void f(void);
void g(void);

/// @brief doc
void h(void);

#endif
```

## 3. File structure

All top-level-items have to follow the following order,
the file is structured as followed.

At first, pre-processor-directives, always at the top of the file:
1. (Implemented headers, for source-files)
2. Other project headers
3. System headers
4. Macro definitions

### 3.1 Header-specific
1. Enum definitions
2. Struct definitions
3. Common functions (grouped by context)

After each enum / struct definition, closed context functions related
to this object has to follow first. Then the order above continues.

### 3.2 Source-specific
1. Static variables
2. Static function forward-declarations
    (not optional, always forward-decl static functions)
3. Implemented functions, included from header
4. Static function implementation (same order as declarations)

## 4. Struct initialisers

If line length not exceeded: all at the same line.
If line length exceeds: every field gets its own line:
```c

return (struct Error){ .kind }
return (struct Token){
    .kind = TK_IDENT,
    .value = value,
    .loc = start,
};
```

## 5. Exceeding the column limit

The 80-column limit is hard. What to do when a construct does not fit depends on
the construct.

### 5.1 Function declarations and definitions

The function name always stays on the same line as the return type. Never break
after the return type — wrap the parameters instead.

Declaration: parameters block-indented, `)` and `;` on their own line.

```c
void myfunc(
    int a, int a1, int a2, int b3,
    int b, int b1, int b2, int b3,
    int c
);
```

Parameters are bin-packed, and each parameter line must itself stay within 80
columns; start a new line when it would not.

Definition: identical, but the body's opening brace goes on the same line as
the closing `)`.

```c
void diag_add(
    struct Arena* a, struct DiagList* dl, enum DiagCode code,
    struct SourceLoc loc, ...
) {
    ...
}
```

### 5.2 Function and macro calls

Same principle: block-indent the arguments, `)` on its own line.

```c
DARRAY_ADD(
    a, dl, message,
    arena, myfield,
    something
);
// in combination with §3
DARRAY_ADD(
    a, &tkl, ((struct Token){
        .kind = TK_INVALID,
        .loc = loc,
        .value = NULL,
    })
);
```

### 5.3 Expressions

Break *before* the operator, indent one level, do not align.

```c
int a = strcmp(argv[i], "-o") == 0
    || strcmp(argv[i], "--output") == 0;
```

For a condition, §4.4 gives the correct form.

### 5.4 Compound statements

Nothing after the `(`; the condition is block-indented and wrapped per §4.3;
the `)` shares its line with the `{`.

```c
if (
    strcmp(argv[i], "-h") == 0
    || strcmp(argv[i], "--help") == 0
) {
    args->show_help = true;
    return true;
}
```

This is what keeps a wrapped condition distinguishable from the body it
introduces: the `)` closes the condition on its own line, so nothing inside the
parentheses ever sits at the body's indentation.

For a `for` statement, break after each `;` where possible:

```c
for (
    const struct IRFunc* current = head_ir_func;
    current != NULL;
    current = current->next
) {
    ..
}
```

### 5.5 Macro definitions

The body can start on the identifier's line,
only if the column limit is not exceeded.

For "body-wrapped" macros, like the following example, no identation is needed

```c
// correct
#define MyMacro(a) do { \
    .. \
} while(0)
```

---

# Part 2 — Conventions beyond formatting

## 6. Naming

| Kind | Convention | Examples |
| --- | --- | --- |
| Struct / enum tag | `PascalCase` | `struct TokenList`, `enum DiagCode` |
| Enumerator | `PREFIX_UPPER_SNAKE` | `TK_IDENT`, `TC_PUNCT`, `DIAG_INVALID`, `SPRACHE_STAGE_ASM` |
| Function | `lower_snake_case` | `diag_has_errors`, `token_kind_str` |
| Variable, field, parameter | `lower_snake_case` | `out_file`, `start_ptr` |
| Macro | `UPPER_SNAKE` | `ARENA_CALLOC_LIST`, `DARRAY_ADD`, `TST_RUN` |
| Compile-time constant | `#define` + `UPPER_SNAKE` | `ARENA_BLOCK_SIZE`, `DIAG_INIT_CAPACITY` |
| File | `lower_snake_case.c/.h/.def` | `tokens_dump.c`, `diag_internal.h` |

### 6.1 Prefixes

- **Every enumerator carries a short prefix derived from its enum**:
  `TokenKind` → `TK_`, `TokenClass` → `TC_`, `DiagCode` → `DIAG_`,
  `SpracheStage` → `SPRACHE_STAGE_`.
- **Subject specific functions are prefixed with their subject**,
    and the subject comes first:
    `diag_dump`, `diag_dump_all`, `arena_calloc`, `token_kind_is_kw`.
    Prefer `<subject>_<verb>` over `<verb>_<subject>`.
- **The public API in `include/sprache/` uses the `sprache_` prefix** when the
    name would otherwise be generic: `sprache_compile`, `sprache_stage_from_str`.
    A name that is already namespaced by its own type (`diag_*`) does not need it.
- **Macros are prefixed by their module**: `ARENA_*`, `DARRAY_*`, `TST_*`.
- **Identifiers a macro introduces into the caller's scope get an underscore
    prefix** so they cannot collide: `_tst_exp`, `_tst_act`.

### 6.2 Short names

Short names are fine — and preferred — for tightly scoped locals with an
obvious meaning: `a` (arena), `i`, `j` (index), `c` (current char), `dl`
(diag list), `tkl` (token list), `d`, `tok`. Anything that outlives a few lines
or crosses a function boundary gets a full name.

`a` is the conventional name for a `struct Arena*` parameter.

### 6.3 Out-parameters

Pointer parameters that exist to return a value are prefixed `out_`, and their
meaning is documented with `@param`:

```c
/// @param out_len set to the length of the match (undefined if TK_INVALID)
static enum TokenKind match_punct(
    const char* source, const size_t* i, size_t* out_len
);
```

## 7. Types and declarations

- **No `typedef` for structs or enums.** Always spell out `struct Token`,
    `enum TokenKind`. This is deliberate — the tag makes the kind of type visible
    at every use site.
- **Pointer binds left**: `char* s`, `struct Arena* a`, `const struct Token* tok`.
- **`const` on every input you do not modify**, including `const char*` for
    borrowed strings and `const struct X*` for read-only struct parameters.
- **`size_t` for sizes, counts, lengths and indices**; `unsigned` for
    `SourceLoc` line/col; plain `int` only for C-mandated interfaces (`argc`,
    `main`'s return).
- **`bool` from `<stdbool.h>`**, and return `true` / `false` — never `1` / `0`.
- A list type is always the triple `items` / `count` / `capacity`, in that
    order, so the `DARRAY_*` macros apply:

  ```c
  struct TokenList {
      struct Token* items;
      size_t count;
      size_t capacity;
  };
  ```

## 8. Headers

### 8.1 Layout

- Public API lives in `include/sprache/` and is included as `"sprache/diag.h"`.
- Everything else lives next to its `.c` file in `src/` and is included by its
  path from `src/`: `"lexer/tokens.h"`, `"utils/arena.h"`.

### 8.2 Include guards

`#ifndef` guards, never `#pragma once`. The macro is the file's basename in
upper case plus `_H`, with no path component and no trailing comment on
`#endif`:

```c
#ifndef TOKENS_H
#define TOKENS_H

...

#endif
```

### 8.3 Self-containedness and forward declarations

Every header compiles on its own and includes what it uses. When only a pointer
to a type is needed, forward-declare the tag instead of including its header:

```c
struct Arena;

struct CompileResult sprache_compile(
    struct Arena* a, struct CompileOptions options
);
```

## 9. X-macro `.def` files

Token kinds, diagnostics and compiler stages are single-sourced in `.def` files.
This is the project's central extension mechanism — respect it.

**Writing a `.def` file**

- Extension `.def`, placed next to the header that declares the enum.
- The first line is a comment giving the record shape, and only that:
  `// TOKEN(kind, spelling, class)`.
- Any further explanation of a field goes in the same comment block at the top.
- One record per line; blank lines may group related records.

**Consuming one**

Always the full three-step dance, with the `#undef` immediately after the
`#include`, and all three at column 1:

```c
#define TOKEN(kind, spelling, class) case kind: return #kind;
#include "lexer/tokens.def"
#undef TOKEN
```

**Rules**

- A new token / diagnostic / stage is added **only** in the `.def` file. If
    adding one requires editing a `switch`, that `switch` is written wrong.
- The enum declares an explicit `*_INVALID` member as its first (zero) value,
    outside the `.def`.
- A `switch` over such an enum handles `*_INVALID` explicitly and has **no
    `default:` label**, so `-Wswitch` reports unhandled cases. The fallback goes
    after the switch:

  ```c
  switch (code) {
      case DIAG_INVALID:
          break;
  #define DIAG(name, format) \
      case name: \
          return #name;
  #include "sprache/diag.def"
  #undef DIAG
  }
  return "<invalid diag code>";
  ```

## 10. Comments

- **Doxygen `///` for API documentation**, using `@brief`, `@note`, `@param`,
    `@return`. `Doxyfile` is checked in; `make docs` renders it.
- **Document the declaration, not the definition.** For a public function that
    means the header; for a `static` function it means the forward declaration at
    the top of the `.c` file. Never duplicate the comment on the definition.
- Not every function needs a comment. Document what the signature does not say:
    invariants, assumptions about the caller, ownership, edge cases.

  ```c
  /// @brief Advances by exactly one character and returns it.
  /// @note Does not skip whitespace. Returns '\0' at end of input without
  /// advancing further.
  static char loc_step(const char* source, size_t* i, struct SourceLoc* loc);
  ```

- **`//` for implementation notes** inside function bodies. Trailing `//`
    comments are allowed; align them into a column within a single block, one
    space after the code at minimum.
- Comments say *why*, or state a non-obvious contract. Do not restate the code.
- English, always — including commit-adjacent notes in `.def` files.
- Keep comments true. A stale comment is worse than none; when behaviour
    changes, the comment changes in the same edit.

## 11. Memory

The arena owns everything. There is exactly one lifetime in the compiler:
the arena created in `main` and released by `free_arena` when the process is
done.

- **Allocate with `ARENA_CALLOC(a, T)` / `ARENA_CALLOC_LIST(a, n, T)`.**
- **Never call `malloc`, `calloc`, `realloc` or `free` in compiler code.** Raw
    allocation exists only inside `src/utils/arena.c`, and only through the
    `xmalloc` / `xcalloc` / `xrealloc` wrappers, which abort on failure so callers
    never check for `NULL`.
- **Do not write `free_*` / destructor functions** for arena-allocated types,
    and do not document them as needing to be freed. Nothing that comes out of the
    arena is individually released.
- **The arena is the first parameter after the context-parameter**, named `a`,
    of any function that allocates.
- **Growable lists use `DARRAY_INIT` / `DARRAY_ADD`** with a `#define`d initial
    capacity constant (`TOKENLIST_INIT_CAPACITY`, `DIAG_INIT_CAPACITY`).
- Returned strings and lists are arena-owned and borrowed by the caller; they
    stay valid until `free_arena`. Say so in the doc comment when it is not
    obvious.

## 13. Error handling

Three distinct mechanisms — pick by who made the mistake.

| Situation | Mechanism |
| --- | --- |
| Error in the user's *source program* | `diag_add(...)` into the `DiagList` |
| Error in how the compiler was *invoked*, or an I/O failure | message on `stderr`, `exit(1)` or `return false` |
| Broken internal invariant | `assert` |

**Diagnostics.** Message texts are `printf` formats declared in
`include/sprache/diag.def`; `diag_add` formats and arena-allocates them. Never
build a diagnostic string at the call site. `struct CompileResult` carries `ok`
plus the list; the driver decides how to print it.

**Driver and I/O errors.** One line on `stderr`, prefixed `Error: `, ending in
`\n`, naming the offending value in single quotes:

```c
fprintf(stderr, "Error: Unexpected argument '%s'\n", argv[i]);
return false;
```

A predicate-style function (`parse_args`) prints the message itself and returns
`false`; unrecoverable failures deep in a utility (`read_file`, `xmalloc`)
print and `exit(1)`.

**Assertions.** Use `assert` for preconditions a caller inside this codebase
must uphold, and state the same thing in the doc comment:

```c
assert(isdigit((unsigned char)source[*i]));
```

## 14. Tests

See the `write-tests` skill for the full workflow. Style rules:

- Unit tests live in `tests/unit/<module>/<module>_tests.c`. The filename
  **must** end in `_tests.c` — the makefile globs for it.
- Assertions come from `tests/tst.h`: `TST_ASSERT`, `TST_ASSERT_EQ`,
  `TST_ASSERT_STR_CONTAINS`.
- One `static void` function per behaviour, named as a lower_snake_case
  sentence describing that behaviour:
  `lex_punct_prefers_longest_match`,
  `lex_literal_stops_before_trailing_punct`.
- Each test sets up and tears down its own arena — `init_arena` at the top,
  `free_arena` at the bottom. No shared state between tests.
- Put the input under test in a comment above the call when it contains escapes
  or is otherwise hard to read.
- `main` is just `TST_RUN` per test, in declaration order, then `TST_SUMMARY()`.

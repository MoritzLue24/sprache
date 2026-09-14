# Semantic Analysis

What sema owns, what it hands on, and which rules are not visible from
reading the code. For the *shape* of the code, read the files; this
document covers the decisions behind them.

## Status

<!-- @claude-begin -->
| Part | State |
| --- | --- |
| `type/` — `enum TypeKind` and its queries | done |
| `builtin/` — `enum BuiltinKind` and its queries | done |
| `sema/symbols.h`, `symtable.c`, `symbol.c`, `symbol_dump.c` | done |
| `sema/sema.c` — the checker itself | done, no unit tests yet |
| Wiring in `compile/compile.c` | done, sema only runs if parsing succeeded |
| AST annotation fields + dump | done |

Everything under [Design contracts](#design-contracts-for-semac) is
implemented in `sema.c`.
<!-- @claude-end -->

---

## Where sema sits

```
source -> lex -> parse -> SEMA -> irgen -> regalloc -> avrgen -> asm
                   |        |       |
                   |        |       +-- reads the annotations, then the
                   |        |           AST is dead
                   |        +-- resolves names, resolves and checks types,
                   |            annotates the tree IN PLACE
                   +-- produces an AST that is syntactically valid but
                       says nothing about meaning
```

The parser deliberately resolves nothing. `parse_type()` accepts any
identifier and stores only its spelling in `NODE_TYPE.value` — deciding
whether `uint8` names a type is sema's job, and it stays that way when
user-defined type names arrive.

Sema does not rebuild the tree. It writes two fields per node and leaves
everything else untouched.

---

## Lifetimes

**The rule everything below follows from: the AST and its annotations die
once `irgen` is finished. Whatever the backend still needs must have been
copied into the IR before that.**

| Construct | Defined in | Lives from → until | Read by |
| --- | --- | --- | --- |
| `enum TypeKind` | `src/type/` | sema → avrgen | sema, irgen, IR, regalloc, avrgen |
| `enum BuiltinKind` | `src/builtin/` | sema → avrgen | sema, irgen, IR, avrgen |
| `struct Symbol`, `Scope`, `SymTable` | `src/sema/` | sema → irgen | sema, irgen — **nobody else** |
| `n->type`, `n->symbol` | annotation on `struct Node` | sema → irgen | irgen |

Symbols genuinely do not outlive sema: `irgen` reads each one once and
translates it into something IR-native (a variable becomes a vreg number,
a function becomes an `IRFunc`). After that they are unreachable.

### Worked example: `@read(pin)`

```
parse    NODE_BUILTIN { value: "read", args: [...] }
           knows the name, nothing else

sema     n->symbol = &Symbol { SYM_BUILTIN, builtin_kind: BUILTIN_READ,
                               params: {uint8}, type: uint8 }
         n->type   = uint8

irgen    IR_BUILTIN { builtin: BUILTIN_READ, dest: %2, args: [%0] }
           reads n->symbol->builtin_kind and COPIES the enum into the
           instruction; the symbol is not needed after this point

avrgen   switch (instr->builtin) { case BUILTIN_READ: ... }
           only ever sees the IR
```

The symbol is the **hand-off point**, not the carrier. That is why
`builtin_kind` belongs on `struct Symbol` while the enum itself does not
belong under `sema/`.

---

## AST annotations: who writes what

`struct Node` carries two fields sema owns. `node_init()` zeroes them, so
before sema has run they read `TYPE_INVALID` and `NULL` — which is what
`-s AST` shows, and what `-s SEMA` shows for anything sema failed to
resolve.

| Field | Written by | Meaning |
| --- | --- | --- |
| `kind`, `loc`, `op`, `value` | parser | never touched by sema |
| `type` | sema | the type of this node. On a `NODE_TYPE` node: the type it *denotes* |
| `symbol` | sema | the entity this name refers to |

`symbol` is set on the name-bearing kinds — `NODE_IDENT`, `NODE_CALL`,
`NODE_BUILTIN`, `NODE_PARAM`, `NODE_VAR_DECL`,
`NODE_VAR_DEF`, `NODE_FUNC_DEF` — and stays `NULL` everywhere else.

`ast_dump.c` prints both fields for every node unconditionally. It is the
one place in `parser/` that includes a header from `sema/`
(`sema/symbols.h`, for `symbol_dump`). That is deliberate: presentation
code knows what it presents, and the dependency stays inside the
`_dump.c` file — `parser/ast.h` itself only forward-declares
`struct Symbol`.

---

## Why the modules are split this way

The include direction decides it, not taste.

**`enum TypeKind` cannot live under `sema/`.** `struct Node` has an
`enum TypeKind type` field, and C has no forward declaration for enums
(unlike structs) — the compiler must know the underlying size. So
`parser/ast.h` has to `#include` the full definition. If that header sat
under `sema/`, every file touching the AST would drag in the semantic
analyser.

**`enum BuiltinKind` cannot either**, for the mirror-image reason: the
`IR_BUILTIN` instruction carries it and avrgen switches on it, so the
backend would end up including a frontend header.

**`struct Symbol` can and does stay under `sema/`**, because only sema
and irgen ever name it, and both reach it through a pointer.

Result:

```
type/type.h        no project dependencies at all
builtin/builtin.h  -> type/type.h
parser/ast.h       -> type/type.h        + forward-declares struct Symbol
sema/symbols.h     -> type/, builtin/, sprache/diag.h
```

`type.def` deliberately has **no token column**: a type is matched by its
spelling. Otherwise `type/type.h` would need `lexer/tokens.h`, and the
AVR backend would transitively include the tokenizer.

---

## The symbol table

### Symbols are individually allocated — do not "optimise" this

```c
struct SymbolList {
    struct Symbol** items;   // pointers, NOT values
    ...
};
```

`symtable_declare()` allocates each `struct Symbol` on its own with
`ARENA_CALLOC` and stores only the pointer in the list.

This is load-bearing. `DARRAY_ENSURE` grows a list by allocating a **new**
array and copying into it, so anything held by address into `items` would
dangle after the 11th entry of a scope (`SYMBOL_LIST_INIT_CAPACITY` is
10). Since `n->symbol` holds exactly such an address for the entire
remaining life of the AST, storing symbols by value would produce
dangling pointers that only appear in larger programs.

Growing the list reallocates the *pointer array*; the symbols themselves
never move.

### No shadowing at all

`symtable_declare()` rejects a name that `symtable_lookup()` finds
**anywhere in the scope chain**, not just in the innermost scope. So this
is stricter than C:

```sprache
fn f() -> uint8 {
    var a: uint8 = 1;
    { var a: uint8 = 2; }   // error: outer 'a' is visible here
    var f: uint8;           // error: collides with fn f
    return a;
}
```

Two consequences worth knowing:

- A separate "this name is a type" check is unnecessary —
  `var uint8: uint8;` already fails against the predeclared `SYM_TYPE`.
- The table cannot tell "redeclared in the same scope" from "shadows an
  outer name", because both return `NULL`. Distinct diagnostics for the
  two cases would need an additional `symtable_lookup_local()`.

### Scopes are not opened for you

`symtable_init()` only sets `current = NULL`. The global scope is the
caller's job — forget it and `symtable_declare()` trips its `assert`.

Intended nesting:

```
global scope        predeclared types + every top-level function
 └─ function scope  parameters AND the body's top-level locals, together
     └─ block scope one per nested block
```

Parameters share a scope with the body's outermost locals on purpose, so
`fn f(a: uint8) -> uint8 { var a: uint8; ... }` is a redeclaration, as in
C. That means `check_func_def()` must iterate the body's statements
itself rather than calling `check_block()`, which would open a second
scope.

---

## Types

`type.def` currently defines `none`, `uint8`, `int8`.

**`TYPE_NONE` is spellable.** `type_kind_from_str("none")` finds it, so
`var x: none;` and `fn f(a: none)` parse and resolve without complaint,
yielding a variable of size 0. `fn f() -> none` is presumably wanted;
the other two are not. Rejecting them needs an explicit check — nothing
prevents it automatically.
<!-- @claude-begin -->
That check is `resolve_type(..., is_ret_type)`. An operand of type
`none` is rejected by `check_operand()`.
<!-- @claude-end -->

**There is no `bool`.** Comparison operators (`==`, `<`, …) are lexed but
not in `binop.def` yet. When they arrive they need a result type, and
conditions need something to be checked against; without a `bool`, `if`
falls back to integer truthiness and `if (x = 5)` becomes a silent bug
instead of a type error.

**`type_kind_str()` returns the enumerator name, not the spelling.**

```c
type_kind_str(TYPE_UINT8)          // "TYPE_UINT8"
type_kind_from_str("uint8")        // TYPE_UINT8
```

The two are not inverses despite their names. Anything user-facing —
diagnostics, the `-s SEMA` dump — currently shows `TYPE_UINT8` where a
reader expects `uint8`. The `spelling` column exists and is so far only
used by `type_kind_from_str`.

---

## Builtins

**Builtins are not entries in the symbol table.** `@name` is resolved
with `builtin_kind_from_spelling()`, and the `struct Symbol` that lands
on the node is *synthesised* from the `builtin_kind_*` accessors rather
than looked up.

The reason is namespacing. `@` already opens a separate namespace, so
putting builtins into the ordinary scope chain would reserve their names:
`fn read()` would collide with `@read`, and a bare `read(...)` would
resolve to the builtin and then have to be rejected by kind. Synthesising
avoids all of it and needs no extra scope.

`SYM_BUILTIN` still exists as the kind of that synthesised symbol, so the
dump and irgen see builtins the same way they see anything else.

The argument slots in `builtin.def` are fixed at four and padded with
`TYPE_INVALID` because **a braced list cannot be passed through a macro
argument** — the preprocessor splits on its commas. `argc` bounds every
read, so the padding is never looked at.

---

## Design contracts for `sema.c`

<!-- @claude-begin -->
The rules `sema.c` is built on. Change them here first.
<!-- @claude-end -->

### Two passes

Pass 1 declares every top-level function **and resolves its complete
signature** — return type *and* every parameter type. Pass 2 checks the
bodies.

The split has to be exactly there: a call may be checked before the
callee's body is visited.

```sprache
fn main() -> uint8 { return add(1, 2); }
fn add(a: uint8, b: uint8) -> uint8 { return a + b; }
```

Without the look-ahead, `add`'s parameter types would still be
`TYPE_INVALID` when `main` is checked.

### `TYPE_INVALID` is the error channel

`parser.c` needs `p->panic` *and* `sync()` because it has a cursor to
reposition. Sema walks a finished tree, so only the suppression half is
needed — and the return value does it:

- **as an input** (`expected`): the context imposes no type
- **as an output**: this expression has no usable type, **and a
  diagnostic was already reported**

A caller that receives `TYPE_INVALID` propagates it in silence. That is
what keeps one broken construct to one diagnostic, with no state to
track.

The expected type travels **down** so literals and unary `-` can adopt
it, and is verified **on the way back up**, centrally in `check_expr()`.
No caller compares two types itself.

**The one exception:** a variable declared with an unknown type is still
entered into the table with `type = TYPE_INVALID`, so later uses report a
type problem rather than an additional "undeclared identifier". Reading
such an identifier therefore returns `TYPE_INVALID` *without* reporting —
the diagnostic was issued at the declaration.

### Order of checks decides the message

| Where | Order | Otherwise |
| --- | --- | --- |
| `check_var_decl_or_def` | check the initialiser **first**, declare the variable after | `var a: uint8 = a;` would silently read itself |
| `check_assign` | check the left operand **first**, then test for a modifiable lvalue | `x = 1` with undeclared `x` would say "not assignable" instead of "undeclared" |
| `check_ident` | set `n->symbol` **even on failure** | the dump would say `<unresolved>` for a name that was found but had the wrong kind |

### Literals have no type of their own

<!-- @claude-begin -->
A literal adopts the expected type. With no expectation — or an
expectation of `none`, which cannot hold one — it falls back to `uint8`;
the lexer only ever produces non-negative literals. Unary `-` asks its
operand for `int8` when nothing above asked for anything **and** the
operand has no type of its own, and it rejects unsigned operands, so
`var a: uint8 = -1;` is an error.

"No type of its own" (`expr_is_untyped()`) means built from literals
only: a literal, a unary over one, or a non-assignment binary over two.
In a binary expression with no context, the operand that **has** a type
is checked first and its type becomes the expectation for the other, so
`1 + a` behaves like `a + 1`, and so does `-1 + a`.

Known gap: `-128` does not fit `int8`, because `128` is checked against
`int8` before the negation is applied.

### Assignment targets

`check_assign()` checks the target without an expectation, then asks
`expr_is_modifiable_lvalue()`, which today accepts only an identifier
resolving to a `SYM_VAR`. The test is skipped when the target already
came back `TYPE_INVALID`, so `x = 1` with an undeclared or non-variable
`x` yields exactly one diagnostic. The assignment's type is the target's
type; `check_expr()` compares it against the context like any other.
<!-- @claude-end -->

### A function must return on every path

Two separate checks: each `return e` is type-checked against the
enclosing function's return type, and the body as a whole must satisfy
`stmt_always_returns()`. With no `if`/`while` in the language yet that
predicate is trivial, but it is written in the shape those constructs
will need.

---

## Open items

<!-- @claude-begin -->
- No unit tests for `type/`, `builtin/` or `sema/`.
- A `-> none` function is exempt from the return-path check, but
  `return` always takes an expression, so it can only return the result
  of another `none` call.
<!-- @claude-end -->
- Warnings (e.g. unreachable code after `return`) are impossible until
  `struct Diag` gains a severity field; `diag_has_errors()` counts every
  diagnostic as an error.

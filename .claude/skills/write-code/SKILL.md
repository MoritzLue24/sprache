---
name: write-code
description: Use when creating or modifying ANY file in the Sprache repo - C sources and headers, .def files, tests, makefile, docs. Covers the mandatory @claude-begin/@claude-end watermark that marks every region Claude wrote, the binding style guide in docs/style.md, and the verification pass that runs before handing work back.
---

# Writing and modifying files in Sprache

Three things happen on every file edit, in this order:

1. **Style** - `docs/style.md` is the single source of truth. Read it, do not
   assume it.
2. **Watermark** - every region you write gets wrapped in `@claude-begin` / `@claude-end`.
3. **Verify** - column limit, marker balance, build. Before you report back, not after.

None of the three is optional, and none is skipped because a change "is only one line".

## 1. Watermark - mandatory

Every region of a file that you author or change is wrapped in a marker pair:

```c
// @claude-begin
void sema_check(
    struct Arena* a, struct Node* n_program, struct DiagList* dl
);
// @claude-end
```

The tag is exactly `@claude-begin` and `@claude-end` - no date, no model name, no
description. It stays greppable and never goes stale.

### Comment syntax per file type

| Files | Marker |
| --- | --- |
| `.c`, `.h`, `.def` | `// @claude-begin` … `// @claude-end` |
| `makefile`, `Doxyfile`, shell | `# @claude-begin` … `# @claude-end` |
| `.md` | `<!-- @claude-begin -->` … `<!-- @claude-end -->` |

If a comment would be a **syntax error** in that file (strict `.json` without
JSONC support), write no marker and say so in your reply instead. Never invent a
marker syntax the tool chain will choke on.

### Placement

- The markers sit on their **own lines**, at the **same indentation** as the code
    they wrap.
- Both marker lines count against the column limit like any other line. At deep
    indentation they still fit - the tag is short.
- **New file:** one pair around the entire content. `// @claude-begin` is line 1,
    `// @claude-end` the last line. Include guards go inside the pair.
- **Existing file:** one pair per contiguous edited region.

### Granularity

- **One pair per contiguous region**, never one pair per line. Two edited regions
    separated by two or fewer untouched lines get a single pair covering both.
- **Prefer the whole declaration.** If you rewrote most of a function, wrap the
    function - signature, doc comment and body - rather than three lines in its
    middle. A marker pair inside a function body is for a genuinely local change.
- A doc comment you wrote belongs **inside** the pair, together with the thing it
    documents.

### Rules for existing markers

- Editing code that already sits **inside** a marker pair adds **no** new markers -
    the existing pair already covers it.
- If your change spans the boundary of an existing pair, **widen that pair** rather
    than nesting a second one. Marker pairs never nest.
- **Never remove or relocate a marker** unless you deleted everything inside it -
    then remove the now-empty pair too.
- If the user has since edited code inside a pair, the markers stay. They are the
    user's to remove; strip them only when asked ("remove the watermarks").

### Two traps

- **Backslash-continued macros.** A `//` comment inside a `\`-continued `#define`
    swallows the continuation and breaks the macro. Put the pair *around* the whole
    `#define`, never between its lines.

  ```c
  // @claude-begin
  #define ARENA_CALLOC(a, T) \
      ((T*)arena_calloc((a), 1, sizeof(T)))
  // @claude-end
  ```

- **The X-macro `#define` / `#include` / `#undef` sequence is one unit.** Wrap all
    three together, never just the `#define` line.

### What gets no watermark

Generated or throwaway output: `build/`, `docs/doxygen/`, compiler-emitted `.asm` /
`.s` under `examples/`, and files in the scratchpad directory. Also: commit
messages, PR bodies and your chat replies - the watermark marks source, not prose.

Configuration under `.claude/` is exempt too: a skill's YAML frontmatter must start
on line 1, so a marker above it would break the parse. Say in your reply which
`.claude/` files you touched instead.

## 2. Style - read `docs/style.md`

`docs/style.md` is binding and is the **only** place the rules live. Nothing from it
is repeated here, so it can change without this skill going stale - which also means
you cannot satisfy this step from memory.

- Read it in full before your first edit of a session.
- Re-read the section that governs the construct you are about to write whenever you
    touch an area you have not touched yet in this session. Its table of contents is
    at the top.
- **There is no formatter.** Every rule it states, whitespace included, is written by
    hand and checked by reading the diff.
- Where it is silent, copy the surrounding code. Before writing a new file, read its
    two closest neighbours in the same directory and match their shape.

If a rule in `docs/style.md` and a pattern in existing code disagree, the document
wins - and the mismatch is worth mentioning in your reply.

## 3. Verify before reporting back

Run all four. Do not report a change as done until they pass.

**Marker balance** - every `begin` has its `end`, no nesting. Skip files that
*document* the markers (this skill, `docs/style.md`) - their code samples contain
the literal tags and will read as unbalanced:

```sh
for f in $(git diff --name-only; git ls-files -o --exclude-standard); do
    [ -f "$f" ] || continue
    awk -v f="$f" '
        /@claude-begin/ { d++ }
        /@claude-end/   { d-- }
        d < 0 { print f":"FNR": @claude-end without a begin" }
        d > 1 { print f":"FNR": nested @claude-begin" }
        END   { if (d != 0) print f": unclosed @claude-begin" }
    ' "$f"
done
```

**Column limit** - on the files you touched, with `max` set to the limit
`docs/style.md` states:

```sh
awk -v max=<limit> 'length > max { print FILENAME":"FNR": "length" cols" }' <files>
```

**Build**, and **tests** when you touched anything under `src/` or `tests/`:

```sh
make && make test
```

`make asan` for anything touching memory or the arena. `make docs` after changing
Doxygen comments - it needs `doxygen` and `dot` on `PATH` and fails loudly if
either is missing.

**Read your own diff** (`git diff`). This is where the hand-formatting rules are
actually enforced - the compiler will not catch a misplaced blank line or a wrong
wrap form.

## Working preferences

- **Tests:** the `write-tests` skill covers the two tiers and `tst.h`. A new
    `*_tests.c` needs no makefile change - it is picked up by a `find` glob.
- **A new `.c` file** likewise needs no makefile change.
- **Do not touch** `build/`, `docs/doxygen/` - both are generated and `make clean`
    deletes them.
- **Do not commit or push** unless asked.

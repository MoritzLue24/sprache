# Tokenizer

## Structure

Divided into the lexical analysis logic, and the structure of the output:
- `lexer.h`
- `tokens.h`

### Tokens
```mermaid
classDiagram {
    class Token {
        <<struct>>
        enum TokenType type,
        char* value,
        struct Loc begin,
        struct Loc end
        struct Token* next
    }

    class Loc {
        <<struct>>
        const char* source,
        char c,
        size_t i,
        unsigned int ln,
        unsigned int col,
        bool end
    }

    class TokenType {
        <<enum>>
        TT_INVALID,
        TT_END,
        TT_IDENT,
        TT_FUNC,
        ...
        TT_VAR,
        TT_SEMICOLON,
        ...
        TT_LITERAL
    }

    Token --> Loc
    Token --> TokenType
}
```
Comments:
- `struct Token` owns `value`: The struct itself is responsible for allocating & freeing the memory for this string.
- `struct Loc` does not own `source`: The string is owned by the source file, and should not be freed by the struct.
- A list of tokens is implemented as a linked list, with `next` pointing to the next token in the list. The last token in the list has `next` set to `NULL`.

**For keyword & punctuation matching:**
```mermaid
classDiagram {
    class MatchTypePair {
        <<struct>>
        const char* match,
        enum TokenType type
    }
}
```
Comments:
- `match` is owned by the struct
- Arrays of `MatchTypePair` are used to match keywords and punctuation to their corresponding token types. The arrays are declared as global variables in `lexer.h` & defined in `lexer.c`.



## Dynamic Memory Management & Ownership


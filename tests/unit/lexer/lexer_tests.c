#include "tst.h"
#include "lexer/lexer.h"

static void lex_ident_and_keyword_boundary()
{
    struct Arena a;
    init_arena(&a);
    struct TokenList tkl = lex(&a, "myIdent _ident24_a fn");

    TST_ASSERT_EQ(TK_IDENT, tkl.items[0].kind);
    TST_ASSERT(strcmp(tkl.items[0].value, "myIdent") == 0);
    TST_ASSERT_EQ(TK_IDENT, tkl.items[1].kind);
    TST_ASSERT(strcmp(tkl.items[1].value, "_ident24_a") == 0);
    TST_ASSERT_EQ(TK_FUNC, tkl.items[2].kind);
    TST_ASSERT_EQ(TK_END, tkl.items[3].kind);

    free_arena(&a);
}

static void lex_punct_prefers_longest_match()
{
    struct Arena a;
    init_arena(&a);
    struct TokenList tkl = lex(&a, "== != <= >= ->");

    enum TokenKind expected[] = {
        TK_EQEQ, TK_NEQ, TK_LE, TK_GE, TK_ARROW, TK_END
    };

    for (size_t k = 0; k < sizeof(expected) / sizeof(*expected); k++) {
        TST_ASSERT_EQ(expected[k], tkl.items[k].kind);
    }

    free_arena(&a);
}

static void lex_punct_single_char_tokens()
{
    struct Arena a;
    init_arena(&a);
    struct TokenList tkl = lex(&a, "= ( ) < >");

    enum TokenKind expected[] = {
        TK_EQ, TK_LPAREN, TK_RPAREN, TK_LT, TK_GT, TK_END
    };

    for (size_t k = 0; k < sizeof(expected) / sizeof(*expected); k++) {
        TST_ASSERT_EQ(expected[k], tkl.items[k].kind);
    }

    free_arena(&a);
}

static void lex_literal_stops_before_trailing_punct()
{
    struct Arena a;
    init_arena(&a);
    // 123)
    struct TokenList tkl = lex(&a, "123)");

    TST_ASSERT_EQ(TK_LITERAL, tkl.items[0].kind);
    TST_ASSERT(strcmp(tkl.items[0].value, "123") == 0);
    TST_ASSERT_EQ(TK_RPAREN, tkl.items[1].kind);
    TST_ASSERT_EQ(TK_END, tkl.items[2].kind);

    free_arena(&a);
}

static void lex_skips_whitespace_without_emitting_tokens()
{
    struct Arena a;
    init_arena(&a);
    struct TokenList tkl = lex(&a, "  a   b\t\tc  ");

    TST_ASSERT_EQ(TK_IDENT, tkl.items[0].kind);
    TST_ASSERT_EQ(TK_IDENT, tkl.items[1].kind);
    TST_ASSERT_EQ(TK_IDENT, tkl.items[2].kind);
    TST_ASSERT_EQ(TK_END, tkl.items[3].kind);
    TST_ASSERT_EQ((size_t)4, tkl.count);

    free_arena(&a);
}

static void lex_tracks_line_and_col_across_newline()
{
    struct Arena a;
    init_arena(&a);
    // a
    // bb
    struct TokenList tkl = lex(&a, "a\nbb");

    TST_ASSERT_EQ(1u, tkl.items[0].loc.line);
    TST_ASSERT_EQ(1u, tkl.items[0].loc.col);
    TST_ASSERT_EQ(2u, tkl.items[1].loc.line);
    TST_ASSERT_EQ(1u, tkl.items[1].loc.col);

    free_arena(&a);
}

int main()
{
    TST_RUN(lex_ident_and_keyword_boundary);
    TST_RUN(lex_punct_prefers_longest_match);
    TST_RUN(lex_punct_single_char_tokens);
    TST_RUN(lex_literal_stops_before_trailing_punct);
    TST_RUN(lex_skips_whitespace_without_emitting_tokens);
    TST_RUN(lex_tracks_line_and_col_across_newline);
    TST_SUMMARY();
}

#include "../../include/scanner.h"

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../../include/common.h"

/* -------------------------------------------------------------------------- */
/*                        Configuration & fast macros                         */
/* -------------------------------------------------------------------------- */

#define ERR_LEN(msg) (sizeof(msg) - 1)

#define PEEK() (*scanner.current)
#define PEEK_NEXT() (*(scanner.current + 1))
#define IS_ALPHA(c) \
    (((c) >= 'a' && (c) <= 'z') || ((c) >= 'A' && (c) <= 'Z') || (c) == '_')
#define IS_DIGIT(c) ((c) >= '0' && (c) <= '9')
#define IS_HEX_DIGIT(c) \
    (IS_DIGIT(c) || ((c) >= 'a' && (c) <= 'f') || ((c) >= 'A' && (c) <= 'F'))

/* -------------------------------------------------------------------------- */
/*                             Global scanner state                           */
/* -------------------------------------------------------------------------- */

Scanner scanner;

/* -------------------------------------------------------------------------- */
/*                         Very hot inline functions                          */
/* -------------------------------------------------------------------------- */

static inline char advance() {
    char c = *scanner.current++;
    if (c == '\n') {
        scanner.line++;
        scanner.column = 1;
        scanner.lineStart = scanner.current;
    } else {
        scanner.column++;
    }
    return c;
}

static inline bool match_char(char expected) {
    if (PEEK() != expected) return false;
    scanner.current++;
    scanner.column++;
    return true;
}

static inline bool is_at_end() { return PEEK() == '\0'; }

static inline Token make_token(TokenType type) {
    Token token;
    token.type = type;
    token.start = scanner.start;
    token.length = (int)(scanner.current - scanner.start);
    token.line = scanner.line;
    token.column = scanner.column;
    return token;
}

static inline Token error_token(const char* msg, int len) {
    Token token;
    token.type = TOKEN_ERROR;
    token.start = msg;
    token.length = len;
    token.line = scanner.line;
    token.column = scanner.column;
    return token;
}

/* -------------------------------------------------------------------------- */
/*                       Fast whitespace & comment skipping                   */
/* -------------------------------------------------------------------------- */

static void skip_whitespace() {
    const char* p = scanner.current;
    int col = scanner.column;
    int line = scanner.line;
    const char* lineStart = scanner.lineStart;

    for (;;) {
        char c = *p;
        if (c == ' ' || c == '\r' || c == '\t') {
            p++;
            col++;
        } else if (c == '\n') {
            break; /* newline is significant */
        } else if (c == '/' && p[1] == '/') {
            p += 2;
            while (*p != '\n' && *p) p++;
        } else if (c == '/' && p[1] == '*') {
            p += 2;
            while (!(*p == '*' && p[1] == '/') && *p) {
                if (*p == '\n') {
                    line++;
                    col = 1;
                    lineStart = p + 1;
                } else
                    col++;
                p++;
            }
            if (*p) {
                p += 2;
                col += 2;
            }
        } else {
            break;
        }
    }

    scanner.current = p;
    scanner.column = col;
    scanner.line = line;
    scanner.lineStart = lineStart;
}

/* -------------------------------------------------------------------------- */
/*                         Perfect‐switch keyword lookup                      */
/* -------------------------------------------------------------------------- */

static TokenType identifier_type(const char* start, int length) {
    switch (start[0]) {
        case 'a':
            if (length == 2 && start[1] == 's') return TOKEN_AS;
            if (length == 3 && memcmp(start, "and", 3) == 0) return TOKEN_AND;
            break;
        case 'b':
            if (length == 5 && memcmp(start, "break", 5) == 0)
                return TOKEN_BREAK;
            if (length == 4 && memcmp(start, "bool", 4) == 0) return TOKEN_BOOL;
            break;
        case 'c':
            if (length == 8 && memcmp(start, "continue", 8) == 0)
                return TOKEN_CONTINUE;
            if (length == 5 && memcmp(start, "catch", 5) == 0)
                return TOKEN_CATCH;
            if (length == 5 && memcmp(start, "const", 5) == 0)
                return TOKEN_CONST;
            break;
        case 'e':
            if (length == 4 && memcmp(start, "else", 4) == 0) return TOKEN_ELSE;
            if (length == 4 && memcmp(start, "elif", 4) == 0) return TOKEN_ELIF;
            break;
        case 'f':
            if (length == 5 && memcmp(start, "false", 5) == 0)
                return TOKEN_FALSE;
            if (length == 3 && memcmp(start, "for", 3) == 0) return TOKEN_FOR;
            if (length == 2 && start[1] == 'n') return TOKEN_FN;
            break;
        case 'i':
            if (length == 2 && memcmp(start, "if", 2) == 0) return TOKEN_IF;
            if (length == 2 && memcmp(start, "in", 2) == 0) return TOKEN_IN;
            if (length == 3 && memcmp(start, "i32", 3) == 0) return TOKEN_INT;
            if (length == 3 && memcmp(start, "i64", 3) == 0) return TOKEN_I64;
            if (length == 4 && memcmp(start, "impl", 4) == 0) return TOKEN_IMPL;
            if (length == 6 && memcmp(start, "import", 6) == 0)
                return TOKEN_IMPORT;
            break;
        case 'l':
            if (length == 3 && memcmp(start, "let", 3) == 0) return TOKEN_LET;
            break;
        case 'm':
            if (length == 3 && memcmp(start, "mut", 3) == 0) return TOKEN_MUT;
            if (length == 5 && memcmp(start, "match", 5) == 0)
                return TOKEN_MATCH;
            break;
        case 'n':
            if (length == 3 && memcmp(start, "nil", 3) == 0) return TOKEN_NIL;
            if (length == 3 && memcmp(start, "not", 3) == 0) return TOKEN_NOT;
            break;
        case 'o':
            if (length == 2 && memcmp(start, "or", 2) == 0) return TOKEN_OR;
            break;
        case 'p':
            if (length == 5 && memcmp(start, "print", 5) == 0)
                return TOKEN_PRINT;
            if (length == 3 && memcmp(start, "pub", 3) == 0) return TOKEN_PUB;
            break;
        case 'r':
            if (length == 6 && memcmp(start, "return", 6) == 0)
                return TOKEN_RETURN;
            break;
        case 's':
            if (length == 6 && memcmp(start, "struct", 6) == 0)
                return TOKEN_STRUCT;
            if (length == 6 && memcmp(start, "static", 6) == 0)
                return TOKEN_STATIC;
            break;
        case 't':
            if (length == 4 && memcmp(start, "true", 4) == 0) return TOKEN_TRUE;
            if (length == 3 && memcmp(start, "try", 3) == 0) return TOKEN_TRY;
            break;
        case 'u':
            if (length == 3 && memcmp(start, "use", 3) == 0) return TOKEN_USE;
            if (length == 3 && memcmp(start, "u32", 3) == 0) return TOKEN_U32;
            if (length == 3 && memcmp(start, "u64", 3) == 0) return TOKEN_U64;
            break;
        case 'w':
            if (length == 5 && memcmp(start, "while", 5) == 0)
                return TOKEN_WHILE;
            break;
    }
    return TOKEN_IDENTIFIER;
}

/* -------------------------------------------------------------------------- */
/*                            Identifier & keyword scan                       */
/* -------------------------------------------------------------------------- */

static Token identifier() {
    while (IS_ALPHA(PEEK()) || IS_DIGIT(PEEK())) {
        advance();
    }
    int length = (int)(scanner.current - scanner.start);
    TokenType type = identifier_type(scanner.start, length);
    return make_token(type);
}

/* -------------------------------------------------------------------------- */
/*                               Sequence matching */
/* -------------------------------------------------------------------------- */

static inline bool match_sequence(const char* seq) {
    const char* p = scanner.current;
    while (*seq) {
        if (*p++ != *seq++) return false;
    }
    return true;
}

/* -------------------------------------------------------------------------- */
/*                          Number literal scanning                           */
/* -------------------------------------------------------------------------- */

static Token number() {
    /* 0xABC-style hex? */
    if (scanner.start[0] == '0' && (PEEK() == 'x' || PEEK() == 'X')) {
        advance(); /* consume x/X */
        if (!IS_HEX_DIGIT(PEEK()))
            return error_token("Invalid hexadecimal literal.",
                               ERR_LEN("Invalid hexadecimal literal."));
        while (IS_HEX_DIGIT(PEEK()) || PEEK() == '_') {
            if (PEEK() == '_') {
                advance();
                if (!IS_HEX_DIGIT(PEEK()))
                    return error_token(
                        "Invalid underscore placement in number.",
                        ERR_LEN("Invalid underscore placement in number."));
            } else {
                advance();
            }
        }
        if (PEEK() == 'u' || PEEK() == 'U') advance();
        return make_token(TOKEN_NUMBER);
    }

    /* Decimal integer + underscores */
    while (IS_DIGIT(PEEK()) || PEEK() == '_') {
        if (PEEK() == '_') {
            advance();
            if (!IS_DIGIT(PEEK()))
                return error_token(
                    "Invalid underscore placement in number.",
                    ERR_LEN("Invalid underscore placement in number."));
        } else {
            advance();
        }
    }

    /* Fractional part */
    if (PEEK() == '.' && IS_DIGIT(PEEK_NEXT())) {
        advance();
        while (IS_DIGIT(PEEK()) || PEEK() == '_') {
            if (PEEK() == '_') {
                advance();
                if (!IS_DIGIT(PEEK()))
                    return error_token(
                        "Invalid underscore placement in number.",
                        ERR_LEN("Invalid underscore placement in number."));
            } else {
                advance();
            }
        }
    }

    /* Exponent part */
    if (PEEK() == 'e' || PEEK() == 'E') {
        advance();
        if (PEEK() == '+' || PEEK() == '-') advance();
        if (!IS_DIGIT(PEEK()))
            return error_token(
                "Invalid scientific notation: Expected digit after 'e' or 'E'.",
                ERR_LEN("Invalid scientific notation: Expected digit after 'e' "
                        "or 'E'."));
        while (IS_DIGIT(PEEK()) || PEEK() == '_') {
            if (PEEK() == '_') {
                advance();
                if (!IS_DIGIT(PEEK()))
                    return error_token(
                        "Invalid underscore placement in number.",
                        ERR_LEN("Invalid underscore placement in number."));
            } else {
                advance();
            }
        }
    }

    /* Optional suffixes */
    if (match_sequence("i32")) {
        advance();
        advance();
        advance();
    } else if (match_sequence("i64")) {
        advance();
        advance();
        advance();
    } else if (match_sequence("u32")) {
        advance();
        advance();
        advance();
    } else if (match_sequence("u64")) {
        advance();
        advance();
        advance();
    } else if (match_sequence("f64")) {
        advance();
        advance();
        advance();
    } else if (PEEK() == 'u' || PEEK() == 'U') {
        advance();
    }

    return make_token(TOKEN_NUMBER);
}

/* -------------------------------------------------------------------------- */
/*                              String literal scanning                       */
/* -------------------------------------------------------------------------- */

static Token string() {
    while (PEEK() != '"' && !is_at_end()) {
        if (PEEK() == '\\') {
            advance();
            if (PEEK() == 'n' || PEEK() == 't' || PEEK() == '\\' ||
                PEEK() == '"') {
                advance();
            } else {
                return error_token("Invalid escape sequence.",
                                   ERR_LEN("Invalid escape sequence."));
            }
        } else {
            advance();
        }
    }

    if (is_at_end()) {
        /* unterminated string */
        return error_token("Unterminated string.",
                           ERR_LEN("Unterminated string."));
    }

    advance(); /* closing '"' */
    return make_token(TOKEN_STRING);
}

/* -------------------------------------------------------------------------- */
/*                               Public API                                   */
/* -------------------------------------------------------------------------- */

/**
 * Find the start of the line containing the given position.
 */
const char* findLineStart(const char* source, const char* position) {
    if (!source || !position || position < source) {
        return source;
    }
    
    const char* lineStart = position;
    
    // Move backwards until we find a newline or reach the beginning
    while (lineStart > source && *(lineStart - 1) != '\n') {
        lineStart--;
    }
    
    return lineStart;
}

/**
 * Calculate the display column accounting for tab width.
 */
int calculateColumn(const char* lineStart, const char* position, int tabWidth) {
    if (!lineStart || !position || position < lineStart) {
        return 1;
    }
    
    int column = 1;
    const char* p = lineStart;
    
    while (p < position) {
        if (*p == '\t') {
            // Advance to next tab stop
            column = ((column - 1) / tabWidth + 1) * tabWidth + 1;
        } else {
            column++;
        }
        p++;
    }
    
    return column;
}

/**
 * Initialize scanner for a new source buffer.
 */
void init_scanner(const char* source) {
    scanner.start = source;
    scanner.current = source;
    scanner.source = source;
    scanner.line = 1;
    scanner.column = 1;
    scanner.lineStart = source;
    scanner.inBlockComment = false;
}

/**
 * Retrieve the next token.
 */
Token scan_token() {
    skip_whitespace();
    scanner.start = scanner.current;

    if (is_at_end()) {
        return make_token(TOKEN_EOF);
    }

    char c = advance();

    /* Single‐char or 2‐char tokens */
    switch (c) {
        case '\n':
            return make_token(TOKEN_NEWLINE);
        case '(':
            return make_token(TOKEN_LEFT_PAREN);
        case ')':
            return make_token(TOKEN_RIGHT_PAREN);
        case '{':
            return make_token(TOKEN_LEFT_BRACE);
        case '}':
            return make_token(TOKEN_RIGHT_BRACE);
        case '[':
            return make_token(TOKEN_LEFT_BRACKET);
        case ']':
            return make_token(TOKEN_RIGHT_BRACKET);
        case ';':
            return make_token(TOKEN_SEMICOLON);
        case ',':
            return make_token(TOKEN_COMMA);
        case '.':
            if (match_char('.')) return make_token(TOKEN_DOT_DOT);
            return make_token(TOKEN_DOT);
        case '?':
            return make_token(TOKEN_QUESTION);
        case '-':
            if (match_char('>')) return make_token(TOKEN_ARROW);
            if (match_char('=')) return make_token(TOKEN_MINUS_EQUAL);
            return make_token(TOKEN_MINUS);
        case '+':
            if (match_char('=')) return make_token(TOKEN_PLUS_EQUAL);
            return make_token(TOKEN_PLUS);
        case '/':
            if (match_char('=')) return make_token(TOKEN_SLASH_EQUAL);
            return make_token(TOKEN_SLASH);
        case '%':
            if (match_char('=')) return make_token(TOKEN_MODULO_EQUAL);
            return make_token(TOKEN_MODULO);
        case '*':
            if (match_char('=')) return make_token(TOKEN_STAR_EQUAL);
            return make_token(TOKEN_STAR);
        case '!':
            if (match_char('=')) return make_token(TOKEN_BANG_EQUAL);
            return make_token(TOKEN_BIT_NOT);
        case '=':
            return make_token(match_char('=') ? TOKEN_EQUAL_EQUAL
                                              : TOKEN_EQUAL);
        case '<':
            if (match_char('<')) return make_token(TOKEN_SHIFT_LEFT);
            return make_token(match_char('=') ? TOKEN_LESS_EQUAL : TOKEN_LESS);
        case '>':
            if (PEEK() == '>' && PEEK_NEXT() != '{' && PEEK_NEXT() != '>') {
                advance();
                return make_token(TOKEN_SHIFT_RIGHT);
            }
            return make_token(match_char('=') ? TOKEN_GREATER_EQUAL
                                              : TOKEN_GREATER);
        case '&':
            return make_token(TOKEN_BIT_AND);
        case '|':
            return make_token(TOKEN_BIT_OR);
        case '^':
            return make_token(TOKEN_BIT_XOR);
        case ':':
            return make_token(TOKEN_COLON);
        case '"':
            return string();
    }

    /* Identifiers and numbers */
    if (IS_ALPHA(c)) return identifier();
    if (IS_DIGIT(c)) return number();

    return error_token("Unexpected character.",
                       ERR_LEN("Unexpected character."));
}

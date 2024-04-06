#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "lex.h"

/**
 * Get token from stdin stream, put the token string into `lexeme[]`
 * @returns token type
 */
static TokenSet getToken(void);
static TokenSet curToken = UNKNOWN;
static char lexeme[MAXLEN];

static int isvariablebody(char c) {
    return isalnum(c) || c == '_';
}

TokenSet getToken(void) {
    int i = 0;
    char c = '\0';

    // remove preceeding null charactor
    while ((c = fgetc(stdin)) == ' ' || c == '\t');
    // now `c` is a non-null char

    if (isdigit(c)) {
        // INT part
        lexeme[0] = c;
        c = fgetc(stdin);
        i = 1;
        while (isdigit(c) && i < MAXLEN) {
            lexeme[i] = c;
            ++i;
            c = fgetc(stdin);
        }
        // now `c` is not a digit (or i == MAXLEN)
        ungetc(c, stdin);
        // put boundary check first :)
        if (i == MAXLEN) {
            fprintf(stderr, "buffer error: single token exceeds lexical buffer\n");
            exit(0);
        }
        lexeme[i] = '\0';
        return INT;
    } else if (c == '+' || c == '-') {
        // ADDSUB | ADDSUB_ASSIGN | INCDEC
        lexeme[0] = c;
        // check if is single `+` `-` or not
        // first take out the following char from stream
        char preceeding = fgetc(stdin);
        if ((preceeding == '+' || preceeding == '-') && preceeding == c) {
            // INCDEC
            lexeme[1] = preceeding;
            lexeme[2] = '\0';
            return INCDEC;
        } else if (preceeding == '=') {
            // ADDSUB_ASSIGN
            lexeme[1] = '=';
            lexeme[2] = '\0';
            return ADDSUB_ASSIGN;
        } else {
            // ADDSUB
            // don't forget to push back stream
            ungetc(preceeding, stdin);
            lexeme[1] = '\0';
            return ADDSUB;
        }
    } else if (c == '*' || c == '/') {
        // MULDIV
        lexeme[0] = c;
        lexeme[1] = '\0';
        return MULDIV;
    } else if (c == '|') {
        // bitwise OR
        strcpy(lexeme, "|");
        return BIT_OR;
    } else if (c == '^') {
        // bitwise XOR
        strcpy(lexeme, "^");
        return BIT_XOR;
    } else if (c == '&') {
        // bitwise AND
        strcpy(lexeme, "&");
        return BIT_AND;
    } else if (c == '\n') {
        lexeme[0] = '\0';
        return END;
    } else if (c == '=') {
        strcpy(lexeme, "=");
        return ASSIGN;
    } else if (c == '(') {
        strcpy(lexeme, "(");
        return LPAREN;
    } else if (c == ')') {
        strcpy(lexeme, ")");
        return RPAREN;
    } else if (isvariablebody(c)) {
        // vairable part
        lexeme[0] = c;
        c = fgetc(stdin);
        i = 1;
        while (isvariablebody(c) && i < MAXLEN) {
            lexeme[i] = c;
            ++i;
            c = fgetc(stdin);
        }
        ungetc(c, stdin);
        // put boundary check first :)
        if (i == MAXLEN) {
            fprintf(stderr, "buffer error: single token exceeds lexical buffer\n");
            exit(0);
        }
        lexeme[i] = '\0';
        return ID;
    } else if (c == EOF) {
        return ENDFILE;
    } else {
        return UNKNOWN;
    }
}

void advance(void) {
    curToken = getToken();
}

int match(TokenSet token) {
    if (curToken == UNKNOWN)
        advance();
    return token == curToken;
}

char* getLexeme(void) {
    return lexeme;
}

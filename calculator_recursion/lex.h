#ifndef __LEX__
#define __LEX__

#define MAXLEN 256

/**
 * All possible token
 * @enum
 */
typedef enum token_set_t {
    UNKNOWN,
    END,     // \n
    ENDFILE, // EOF
    INT,     // integer
    ID,      // variable system
    ADDSUB,  // + - (binary | unary)
    MULDIV,  // * /
    ASSIGN,  // = += -= *= /= %= |= &= ^=
    LPAREN,  // (
    RPAREN   // )
} TokenSet;

/**
 * Test if a token matches the current token
 * @param token 
 * @returns `(1|0)` as `true|false`
 */
extern int match(TokenSet token);

/**
 * Get the next token stored in `lexeme`
 */
extern void advance(void);

/**
 * Get the lexeme of the current token
 * @returns `lexeme` string (representing a token detail)
 */
extern char* getLexeme(void);

#endif // __LEX__

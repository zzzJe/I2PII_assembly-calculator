#ifndef __PARSER__
#define __PARSER__

#include "lex.h"
#define TBLSIZE 64

/**
 * Set PRINTERR to 1 to print error message while calling error()
 * @todo set PRINTERR to 0 before you submit your code
 */
#define PRINTERR 1

/**
 * Macro to print error message and exit the program
 * This will also print where you called it in your program
 */
#define error(errorNum, detail) { \
    if (PRINTERR) \
        fprintf(stderr, "error() called at %s:%d\n", __FILE__, __LINE__); \
    err(errorNum, detail); \
}

/**
 * Error types
 * @enum
 */
typedef enum error_type_t {
    UNDEFINED, // variable not defined
    MISPAREN,  // parentheses not matched
    NOTNUMID,  // not number and id
    NOTFOUND,  // 
    RUNOUT,    // variable table out-of-bound
    NOTLVAL,   // 
    DIVZERO,   // this is evaluation error
    SYNTAXERR  // expression with other expression behind
} ErrorType;

/**
 * Structure of the symbol table
 * @struct
 */
typedef struct {
    int val;
    char name[MAXLEN];
} Symbol;

/**
 * Structure of a tree node
 * @struct
 */
typedef struct _Node {
    TokenSet data;
    int val;
    char lexeme[MAXLEN];
    struct _Node *left; 
    struct _Node *right;
} BTNode;

/**
 * The symbol table
 */
extern Symbol table[TBLSIZE];

/**
 * There would be `x/y/z` symbol initially, value is `0`
 */
extern void initTable(void);

/**
 * Get the value of variable stored in table
 * If no exists then create a new var and return 0
 * @param str varuable name
 * @returns value of variable
 * @warning Will blame if exceed table capacity
 */
extern int getval(char* str);

/**
 * Set the value of variable stored in table
 * If no exists then create a new var, set the value, and return `val`
 * @param str variable name
 * @param val to-bo-set value
 * @returns value of variable
 * @warning Will blame if exceed table capacity
 */
extern int setval(char* str, int val);

/**
 * Make a new node according to token type and lexeme
 * @param tok
 * @param lexe
 * @returns ast node
 */
extern BTNode* makeNode(TokenSet tok, const char* lexe);

/**
 * Free the syntax tree
 */
extern void freeTree(BTNode *root);

extern BTNode* factor(void);
extern BTNode* term(void);
extern BTNode* term_tail(BTNode *left);
extern BTNode* expr(void);
extern BTNode* expr_tail(BTNode *left);
extern void statement(void);

extern void statement(void);
extern BTNode* assign_expr(void);
extern BTNode* or_expr(void);
extern BTNode* or_expr_tail(BTNode* left);
extern BTNode* xor_expr(void);
extern BTNode* xor_expr_tail(BTNode* left);
extern BTNode* and_expr(void);
extern BTNode* and_expr_tail(BTNode* left);
extern BTNode* addsub_expr(void);
extern BTNode* addsub_expr_tail(BTNode* left);
extern BTNode* muldiv_expr(void);
extern BTNode* muldiv_expr_tail(BTNode* left);
extern BTNode* unary_expr(void);
extern BTNode* factor(void);

// Print error message and exit the program
extern void err(ErrorType errorNum, char* detail);

#endif // __PARSER__

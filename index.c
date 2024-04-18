#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>

// lex.h

#define MAXLEN 256

/**
 * All possible token
 * @enum
 */
typedef enum token_set_t {
    UNKNOWN,       // ヾ(•ω•`)o
    END,           // \n
    ENDFILE,       // EOF
    ASSIGN,        // =
    ADDSUB_ASSIGN, // += -=
    BIT_OR,        // |
    BIT_XOR,       // ^
    BIT_AND,       // &
    ADDSUB,        // + - (binary | unary)
    MULDIV,        // * /
    INCDEC,        // ++ --
    INT,           // integer
    ID,            // variable system
    LPAREN,        // (
    RPAREN         // )
} TokenSet;

/**
 * Test if a token matches the current token
 * @param token 
 * @returns `(1|0)` as `true|false`
 */
int match(TokenSet token);

/**
 * Get the next token stored in `lexeme`
 */
void advance(void);

/**
 * Get the lexeme of the current token
 * @returns `lexeme` string (representing a token detail)
 */
char* getLexeme(void);

// parser.h

#define TBLSIZE 64
#define NO_REG_LABEL -1

/**
 * Set PRINTERR to 1 to print error message while calling error()
 * @todo set PRINTERR to 0 before you submit your code
 */
#define PRINTERR 0

/**
 * Macro to print error message and exit the program
 * This will also print where you called it in your program
 */
#define error(errorNum, detail) {\
    fprintf(stdout, "EXIT 1\n");\
    if (PRINTERR) {\
        fprintf(stderr, "error() called at %s:%d\n", __FILE__, __LINE__);\
        err(errorNum, detail);\
    }\
    exit(0);\
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
    int reg;
    char lexeme[MAXLEN];
    struct _Node *left; 
    struct _Node *right;
} BTNode;

/**
 * The symbol table
 */
Symbol table[TBLSIZE];

/**
 * There would be `x/y/z` symbol initially, value is `0`
 */
void initTable(void);

/**
 * Get the value of variable stored in table
 * If no exists then panic
 * @param str varuable name
 * @returns value of variable
 * @warning Will blame if exceed table capacity
 */
int getval(char* str);

/**
 * Set the value of variable stored in table
 * If no exists then create a new var, set the value, and return `val`
 * @param str variable name
 * @param val to-bo-set value
 * @returns value of variable
 * @warning Will blame if exceed table capacity
 */
int setval(char* str, int val);

/**
 * Get the addr in table
 * If no exists then panic
 * @param str 
 * @returns address of variable 
 */
int get_addr(char* str);

/**
 * Make a new node according to token type and lexeme
 * @param tok
 * @param lexe
 * @returns ast node
 */
BTNode* makeNode(TokenSet tok, const char* lexe);

/**
 * Free the syntax tree
 */
void freeTree(BTNode *root);

void statement(void);
BTNode* assign_expr(void);
BTNode* or_expr(void);
BTNode* or_expr_tail(BTNode* left);
BTNode* xor_expr(void);
BTNode* xor_expr_tail(BTNode* left);
BTNode* and_expr(void);
BTNode* and_expr_tail(BTNode* left);
BTNode* addsub_expr(void);
BTNode* addsub_expr_tail(BTNode* left);
BTNode* muldiv_expr(void);
BTNode* muldiv_expr_tail(BTNode* left);
BTNode* unary_expr(void);
BTNode* factor(void);

// Print error message and exit the program
void err(ErrorType errorNum, char* detail);

// codeGen.h

// Evaluate the syntax tree
int evaluateTree(BTNode *root);

/**
 * Generate necessary asm
 * recurse to `TokenSet::ASSIGN` to generate asm, others need not to generate
 * 
 * @param root 
 */
void generate_assembly(BTNode* root);

// Print the syntax tree in prefix
void printPrefix(BTNode *root);

// lex.c

/**
 * Get token from stdin stream, put the token string into `lexeme[]`
 * @returns token type
 */
TokenSet getToken(void);
TokenSet curToken = UNKNOWN;
char lexeme[MAXLEN];

int isvariablebody(char c) {
    return isalnum(c) || c == '_';
}

TokenSet getToken(void) {
    // TODO:
    // variable cannot start with digit error should be handled seperately here
    
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
        // here c may be  `+` or `-`
        // check `preceeding == c` means `++` `--`
        // check `preceeding == =` means `+=` `-=`
        // that's all the possible extension
        if (preceeding == c) {
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
        // fprintf(stderr, "tokenization error: undefined token occurs\n");
        return UNKNOWN;
    }
}

void advance(void) {
    curToken = getToken();
}

int match(TokenSet token) {
    return token == curToken;
}

char* getLexeme(void) {
    return lexeme;
}

// parser.c

/**************************************************************************
 *                               -= IDEA =-                               *
 * The main idea is to use grammer relation to build abstruct syntax tree *
 * - `lexeme[]` is always to-be-processed                                 *
 * - use `match()` to check `lexeme[]`'s type                             *
 * - use `getLexeme()` to get underling token data                        *
 * - lastly use `advance()` to hot update `lexeme[]`                      *
 *   in order to make `lexeme[]` to-be-processed                          *
 * - grammer is the following                                             *
 *   00. statement                                                        *
 *     - ENDFILE                                                          *
 *     - END                                                              *
 *     - assign_expr END                                                  *
 *   01. assign_expr                                                      *
 *     - ID ASSIGN assign_expr                                            *
 *     - ID ADDSUB_ASSIGN assign_expr                                     *
 *     - or_expr                                                          *
 *   02. or_expr                                                          *
 *     - xor_expr or_expr_tail                                            *
 *   03. or_expr_tail                                                     *
 *     - BIT_OR xor_expr or_expr_tail                                     *
 *     - NiL                                                              *
 *   04. xor_expr                                                         *
 *     - and_expr xor_expr_tail                                           *
 *   05. xor_expr_tail                                                    *
 *     - BIT_XOR and_expr xor_expr_tail                                   *
 *     - NiL                                                              *
 *   06. and_expr                                                         *
 *     - addsub_expr and_expr_tail                                        *
 *   07. and_expr_tail                                                    *
 *     - BIT_AND addsub_expr add_expr_tail                                *
 *     - NiL                                                              *
 *   08. addsub_expr                                                      *
 *     - muldiv_expr addsub_expr_tail                                     *
 *   09. addsub_expr_tail                                                 *
 *     - ADDSUB muldiv_expr addsub_expr_tail                              *
 *     - NiL                                                              *
 *   10. muldiv_expr                                                      *
 *     - unary_expr muldiv_expr_tail                                      *
 *   11. muldiv_expr_tail                                                 *
 *     - MULDIV unary_expr muldiv_expr_tail                               *
 *     - NiL                                                              *
 *   12. unary_expr                                                       *
 *     - ADDSUB unary_expr                                                *
 *     - factor                                                           *
 *   13. factor                                                           *
 *     - INT                                                              *
 *     - ID                                                               *
 *     - INCDEC ID                                                        *
 *     - LPAREN assign_expr RPAREN                                        *
 **************************************************************************/

/**
 * symbol count
 */
int sbcount = 0;
/**
 * valiable table for lookups
 */
Symbol table[TBLSIZE];

void initTable(void) {
    strcpy(table[0].name, "x");
    table[0].val = 0;
    strcpy(table[1].name, "y");
    table[1].val = 0;
    strcpy(table[2].name, "z");
    table[2].val = 0;
    sbcount = 3;
}

/**
 * Determine whether variable is registered 
 * 
 * @param name The name of the variable
 * @return boolean
 */
int variable_in_table(char* name) {
    for (int i = 0; i < sbcount; i++)
        if (strcmp(name, table[i].name) == 0)
            return 1;
    return 0;
}

/**
 * Register variable in the table, if variable exists, then do nothing
 * 
 * @param name The name of the variable
 */
void register_in_table(char* name) {
    for (int i = 0; i < sbcount; i++)
        if (strcmp(name, table[i].name) == 0)
            return;
    
    if (sbcount >= TBLSIZE)
        error(RUNOUT, "Try to allocate memory on full-capacity stack");
    
    strcpy(table[sbcount].name, name);
    table[sbcount].val = 0;
    sbcount++;
}

int getval(char* str) {
    int i = 0;

    // default: will not exceeed
    for (i = 0; i < sbcount; i++)
        if (strcmp(str, table[i].name) == 0)
            return table[i].val;
    
    error(NOTFOUND, "Occurs in the evaluatation");
}

int setval(char* str, int val) {
    int i = 0;

    for (i = 0; i < sbcount; i++) {
        if (strcmp(str, table[i].name) == 0) {
            table[i].val = val;
            return val;
        }
    }

    if (sbcount >= TBLSIZE)
        error(RUNOUT, "Try to allocate memory on full-capacity stack");
    
    strcpy(table[sbcount].name, str);
    table[sbcount].val = val;
    sbcount++;
    return val;
}

int get_addr(char* str) {
    for (int i = 0; i < sbcount; i++)
        if (strcmp(str, table[i].name) == 0)
            return i * 4;
    
    error(NOTFOUND, "Occurs in the `get_addr` evaluatation");
}

BTNode* makeNode(TokenSet tok, const char* lexe) {
    BTNode* node = (BTNode*)malloc(sizeof(BTNode));
    strcpy(node->lexeme, lexe);
    node->data = tok;
    node->val = 0;
    node->reg = NO_REG_LABEL;
    node->left = NULL;
    node->right = NULL;
    return node;
}

void freeTree(BTNode* root) {
    if (root != NULL) {
        freeTree(root->left);
        freeTree(root->right);
        free(root);
    }
}

int is_ast_has_illegal_unregistered_variable(BTNode* root) {
    if (!root)
        return 0;
    if (root->data == INT)
        return 0;
    if (root->data == ID)
        return !variable_in_table(root->lexeme);
    if (root->data == ASSIGN) {
        // lookup root->right first
        if (is_ast_has_illegal_unregistered_variable(root->right))
            return 1;
        // register root->left then
        register_in_table(root->left->lexeme);
        return 0;
    } else {
        // + - * / | ^ &
        return is_ast_has_illegal_unregistered_variable(root->left)
            || is_ast_has_illegal_unregistered_variable(root->right);
    }
}

#define IS_LEAF(n) ((n) && !(n)->left && !(n)->right)
#define IS_INT_0(n) ((n)->data == INT && evaluateTree(n) == 0)
#define IS_INT_1(n) ((n)->data == INT && evaluateTree(n) == 1)
#define IS_INT_n1(n) ((n)->data == INT && evaluateTree(n) == -1)
/**
 * Optimize unesessary nodes in a ast
 * 
 * @param root 
 */
void optimize(BTNode** root) {
    // if a node is `NULL` or is a leaf, then no need to optimize
    if (!*root || IS_LEAF(*root))
        return;
    
    // if the root has tree-shaped child, do it first
    if (!IS_LEAF((*root)->left))
        optimize(&(*root)->left);
    if (!IS_LEAF((*root)->right))
        optimize(&(*root)->right);
    
    // :   [+]
    // :   / \
    // : [0] [tree]
    // to
    // : [tree]
    else if ((*root)->lexeme[0] == '+' && IS_INT_0((*root)->left)) {
        BTNode* right_subtree = (*root)->right;
        (*root)->right = NULL;
        freeTree(*root);
        *root = right_subtree;
    }
    // :      [+]
    // :      / \
    // : [tree] [0]
    // to
    // : [tree]
    else if ((*root)->lexeme[0] == '+' && IS_INT_0((*root)->right)) {
        BTNode* left_subtree = (*root)->left;
        (*root)->left = NULL;
        freeTree(*root);
        *root = left_subtree;
    }
    // :      [-]
    // :      / \
    // : [tree] [0]
    // to
    // : [tree]
    else if ((*root)->lexeme[0] == '-' && IS_INT_0((*root)->right)) {
        BTNode* left_subtree = (*root)->left;
        (*root)->left = NULL;
        freeTree(*root);
        *root = left_subtree;
    }
    // :   [*]      :      [*]
    // :   / \      :      / \
    // : [0] [tree] : [tree] [0]
    // to
    // : [0]
    else if ((*root)->lexeme[0] == '*' && (IS_INT_0((*root)->left) || IS_INT_0((*root)->right))) {
        BTNode* new_node = makeNode(INT, "0");
        freeTree(*root);
        *root = new_node;
    }
    // :   [*]
    // :   / \
    // : [1] [tree]
    // to
    // : [tree]
    else if ((*root)->lexeme[0] == '*' && IS_INT_1((*root)->left)) {
        BTNode* right_subtree = (*root)->right;
        (*root)->right = NULL;
        freeTree(*root);
        *root = right_subtree;
    }
}
#undef IS_LEAF
#undef IS_INT_0
#undef IS_INT_1
#undef IS_INT_n1

#define IS_LEAF(n) ((n) && !(n)->left && !(n)->right)
void optimize_constant(BTNode** root) {
    if (!*root || IS_LEAF(*root))
        return;

    if (!IS_LEAF((*root)->left))
        optimize_constant(&(*root)->left);
    if (!IS_LEAF((*root)->right))
        optimize_constant(&(*root)->right);
    
    // :      [operation]
    // :          / \
    // : [constant] [constant]
    // to
    // : [constant']
    if ((*root)->left->data == INT && (*root)->right->data == INT) {
        char buffer[MAXLEN];
        sprintf(buffer, "%d", evaluateTree(*root));
        BTNode* new_node = makeNode(INT, buffer);
        freeTree(*root);
        *root = new_node;   
    }
}
#undef IS_LEAF

void statement(void) {
    // 00. statement
    //   - ENDFILE
    //   - END
    //   - assign_expr END
    BTNode* retp = NULL;
    advance();

    if (match(ENDFILE)) {
        printf("MOV r0 [0]\n");
        printf("MOV r1 [4]\n");
        printf("MOV r2 [8]\n");
        printf("EXIT 0\n");
        exit(0);
    } else if (match(END)) {
        if (PRINTERR)
            printf(">> ");
    } else {
        retp = assign_expr();
        if (match(END)) {
            // This part is for optimazation
            // In exam, do not implement this part first, use evaluation time error instead
            if (is_ast_has_illegal_unregistered_variable(retp))
                error(NOTFOUND, "Occurs in parsing part");
            optimize_constant(&retp);
            evaluateTree(retp);
            generate_assembly(retp);
            freeTree(retp);
            if (PRINTERR)
                printf(">> ");
        } else if (match(UNKNOWN)) {
            // 2024/04/13 stupid TA problem fixed
            // 
            // mechanism:
            // `statement()` will not call `advance()` at the very begining
            // `statement()` rely heavily on `getToken()`'s calling `advance()`
            // to handle initial state, but here comes the problem, `TokenSet::UNKNOWN`
            // is not unique to the initial state, when it encounter unknown
            // token in stdin steam, it will become `TokenSet::UNKNOWN`
            // from the design of `getToken()` by TA we can infer that the nil check
            // is ONLY FOR INITIAL STATE while it is totally wrong
            // 
            // when we encounter undefined token and want to check if current
            // token is `TokenSet::UNKNOWN`, we will call `match()` to
            // check the fact, but `match()` do some UNEXPECTED SIDE EFFECT
            // it wash the UNKNOWN state once, and return the judgement,
            // which makes the overall program behavier hard to predict
            // 
            // solution:
            // 0. REMOVE UNNECESSARY SIDE EFFECT... TA...
            // 1. put the `advance()` in the beginning of `statement()`
            // 2. remove `advance()` in the bottom of `statement()`
            // 3. remove unnecessary init check by `getToken()`
            error(SYNTAXERR, "Undefined token occurs");
        } else {
            error(SYNTAXERR, "Unexpected token after complete expression");
        }
    }
}

BTNode* assign_expr(void) {
    // 01. assign_expr
    //   - ID ASSIGN assign_expr
    //   - ID ADDSUB_ASSIGN assign_expr
    //   - or_expr

    // 1.
    //   [or_expr]
    // 2.
    //    [=]
    //    / \
    // [id] [assign_expr]
    // 3. (this will incur double free)
    //    [=]
    //    / \
    // [id] [+-]
    //    \_/  \
    //         [assign_expr]
    // 3.
    //    [=]
    //    / \
    // [id] [+-]
    //      /  \
    //   [id]  [assign_expr]

    // 1. use `or_expr` stored in `left` first
    // 2. check the following token
    //    if is ADDSUB_ASSIGN / ASSIGN:
    //      if `left` is `ID`:
    //        go ahead
    //      else:
    //        error!()
    //    else:
    //      directly return `left`
    BTNode* retp = NULL;
    BTNode* left = NULL;

    left = or_expr();
    if (match(ASSIGN) || match(ADDSUB_ASSIGN)) {
        if (left->data != ID)
            error(NOTLVAL, "Assign must be on the lvalue");
        // now left is an identifier
        if (match(ASSIGN)) {
            retp = makeNode(ASSIGN, "=");
            advance();
            retp->left = left;
            retp->right = assign_expr();
        } else {
            // here directly translate
            // :   [+=]
            // :   /  \
            // : [x]  [3]
            // to
            // :   [=]
            // :   / \
            // : [x] [+]
            // :     / \
            // :   [x] [3]
            retp = makeNode(ASSIGN, "=");
            retp->left = left;
            retp->right = makeNode(ADDSUB, (char[2]){ getLexeme()[0], '\0' });
            advance();
            retp->right->left = makeNode(ID, left->lexeme);
            retp->right->right = assign_expr();
        }
    } else
        // the END right after `assign_expr()` is checked in `statement()`
        retp = left;
    
    return retp;
}

BTNode* or_expr(void) {
    // 02. or_expr
    //   - xor_expr or_expr_tail

    // 1.
    //   [xor_expr]
    // 2.
    //          [|]
    //          / \
    // [xor_expr] [xor_expr]
    // 3.
    //            [|] ...
    //            / \
    //          [|] [xor_expr]
    //          / \
    // [xor_expr] [xor_expr]
    BTNode* node = xor_expr();
    return or_expr_tail(node);
}

BTNode* or_expr_tail(BTNode* left) {
    // 03. or_expr_tail
    //  - BIT_OR xor_expr or_expr_tail
    //  - NiL
    if (match(BIT_OR)) {
        //      [|]
        BTNode* node = makeNode(BIT_OR, getLexeme());
        advance();
        //      [|]
        //      /
        // [node]
        node->left = left;
        // this is invalid, if use this grammar, then evaluation would be right2left
        // 
        //      [|] <- last evaluated
        //      / \
        // [node] [|]
        //        / \
        //   [node] [...]
        // 
        // ```c
        // node->right = or_expr();
        // return node;
        // ```

        //        [|]
        //        / \
        //      [|] <- first evaluated
        //      / \
        // [node] [xor_expr()]
        node->right = xor_expr();
        return or_expr_tail(node);
    } else {
        return left;
    }
}

BTNode* xor_expr(void) {
    // 04. xor_expr
    //   - and_expr xor_expr_tail
    BTNode* node = and_expr();
    return xor_expr_tail(node);
}

BTNode* xor_expr_tail(BTNode* left) {
    // 05. xor_expr_tail
    //   - BIT_XOR and_expr xor_expr_tail
    //   - NiL
    if (match(BIT_XOR)) {
        BTNode* node = makeNode(BIT_XOR, getLexeme());
        advance();
        node->left = left;
        node->right = and_expr();
        return xor_expr_tail(node);
    } else {
        return left;
    }
}

BTNode* and_expr(void) {
    // 06. and_expr
    //   - addsub_expr and_expr_tail
    BTNode* node = addsub_expr();
    return and_expr_tail(node);
}

BTNode* and_expr_tail(BTNode* left) {
    // 07. and_expr_tail
    //   - BIT_AND addsub_expr add_expr_tail
    //   - NiL
    if (match(BIT_AND)) {
        BTNode* node = makeNode(BIT_AND, getLexeme());
        advance();
        node->left = left;
        node->right = addsub_expr();
        return and_expr_tail(node);
    } else {
        return left;
    }
}

BTNode* addsub_expr(void) {
    // 08. addsub_expr
    //   - muldiv_expr addsub_expr_tail
    BTNode* node = muldiv_expr();
    return addsub_expr_tail(node);
}

BTNode* addsub_expr_tail(BTNode* left) {
    // 09. addsub_expr_tail
    //   - ADDSUB muldiv_expr addsub_expr_tail
    //   - NiL
    if (match(ADDSUB)) {
        BTNode* node = makeNode(ADDSUB, getLexeme());
        advance();
        node->left = left;
        node->right = muldiv_expr();
        return addsub_expr_tail(node);
    } else {
        return left;
    }
}

BTNode* muldiv_expr(void) {
    // 10. muldiv_expr
    //   - unary_expr muldiv_expr_tail
    BTNode* node = unary_expr();
    return muldiv_expr_tail(node);
}

BTNode* muldiv_expr_tail(BTNode* left) {
    // 11. muldiv_expr_tail
    //   - MULDIV unary_expr muldiv_expr_tail
    //   - NiL
    if (match(MULDIV)) {
        BTNode* node = makeNode(MULDIV, getLexeme());
        advance();
        node->left = left;
        node->right = unary_expr();
        return muldiv_expr_tail(node);
    } else {
        return left;
    }
}

BTNode* unary_expr(void) {
    // 12. unary_expr
    //   - ADDSUB unary_expr
    //   - factor
    
    // directly construct `0 ADDSUB unary_expr()`
    // for optimization, if `+ - - + + -` should be optimized into `-`, `- -` should be nothing
    // SHOULD THIS BE IN `optimize(BTNode*)` ?
    // there are 2 possible solution
    // 1. use a loop to keep track of consecutive `ADDSUB` tokens
    //   - use a variable `is_unary_negative := 0`
    //     ┌───────┬───┐
    //     │ a ^ * │ a'│
    //     ├───────┼───┤
    //     │ 0 ^ 0 │ 0 │
    //     │ 1 ^ 0 │ 1 │
    //     │ 0 ^ 1 │ 1 │
    //     │ 1 ^ 1 │ 0 │
    //     └───────┴───┘
    //   - while match(ADDSUB):
    //       is_unary_negative ^= getLexeme()[0] == '-'
    //       advance()
    // 2. use recursion // too complicated
    //    - if getLexeme()[0] == '+':
    //        return unary_expr()
    //      else:
    //        next_unary := unary_expr()
    //        if next_unary is ADDSUB and '-': // but here need to check if it is unary or binary operation
    //          retp := next_unary
    //          free(next_unary->left)
    //          free(next_unary)
    //          return retp
    //        else:
    //          make `0 - {next_unary}` grammar

    // IF `optimize()` IS FINISHED, THEN USE THE FOLLOWING MAY BE MORE APPROPRIATE
    // if (match(ADDSUB)) {
    //     BTNode* root = makeNode(ADDSUB, (char[2]){ getLexeme()[0], '\0' });
    //     advance();
    //     root->left = makeNode(INT, "0");
    //     root->right = unary_expr();
    //     return root;
    // } else {
    //     return factor();
    // }

    int is_unary_negation = 0;
    for (; match(ADDSUB); advance())
        is_unary_negation ^= getLexeme()[0] == '-';
    
    // here match(ADDSUB) is false
    BTNode* node = factor();
    if (is_unary_negation) {
        BTNode* root = makeNode(ADDSUB, "-");
        root->left = makeNode(INT, "0");
        root->right = node;
        return root;
    } else
        return node;
}

BTNode* factor(void) {
    // 13. factor
    //   - INT
    //   - ID
    //   - INCDEC ID
    //   - LPAREN assign_expr RPAREN
    BTNode* retp = NULL;

    if (match(INT)) {
        retp = makeNode(INT, getLexeme());
        advance();
        // TODO:
        // variable name start with number should be in the tokenizer part
        // b/c we cannot assert that `ID` after `INT` is seperated by space or not
        // and that is two kind of error, but both are error
        // for examine-oriented project, it is best not to handle it with extra time
        // but for best practice, `variable cannot start with digit` error should be
        // handled seperately in `lex.c`
    } else if (match(ID)) {
        retp = makeNode(ID, getLexeme());
        advance();
    } else if (match(LPAREN)) {
        advance();
        retp = assign_expr();
        if (match(RPAREN))
            advance();
        else
            error(MISPAREN, "");
    } else if (match(INCDEC)) {
        char addsub_operation = getLexeme()[0];
        advance();
        // consider case `++(x)`
        BTNode* next = factor();
        if (next->data == ID) {
            retp = makeNode(ASSIGN, "=");
            retp->left = next;
            retp->right = makeNode(ADDSUB, (char[2]){ addsub_operation, '\0' });
            retp->right->left = makeNode(ID, next->lexeme);
            retp->right->right = makeNode(INT, "1");
        } else {
            error(SYNTAXERR, addsub_operation == '+'
                ? "Cannot apply increment operator on non-identifier"
                : "Cannot apply decrement operator on non-identifier"
            );
        }
    } else {
        error(NOTNUMID, "");
    }
    return retp;
}

void err(ErrorType errorNum, char* detail) {
    if (PRINTERR) {
        fprintf(stderr, "error: ");
        switch (errorNum) {
            case MISPAREN:
                fprintf(stderr, "mismatched parenthesis. %s\n", detail);
                break;
            case NOTNUMID:
                fprintf(stderr, "number or identifier expected. %s\n", detail);
                break;
            case NOTFOUND:
                fprintf(stderr, "variable not defined. %s\n", detail);
                break;
            case RUNOUT:
                fprintf(stderr, "out of memory. %s\n", detail);
                break;
            case NOTLVAL:
                fprintf(stderr, "lvalue required as an operand. %s\n", detail);
                break;
            case DIVZERO:
                fprintf(stderr, "divide by constant zero. %s\n", detail);
                break;
            case SYNTAXERR:
                fprintf(stderr, "syntax error. %s\n", detail);
                break;
            default:
                fprintf(stderr, "undefined error. %s\n", detail);
                break;
        }
    }
    exit(0);
}

// codeGen.c

int reg_label = 6;
int mem_label = 63;

/**
 * Write a register registration on stdin and return the allocated rege label
 * 
 * @param node 
 */
void asm_ralloc(BTNode* node);
/**
 * Write a assign asm statement on stdin by a valid tree
 * the tree should have `=` rooted
 * 
 * @param assign_root 
 */
void asm_assign(BTNode* assign_root);
/**
 * Write a arithmetic asm statement on stdin by valid arthmetic tree
 * 
 * @param arith_root 
 */
void asm_arithmetic(BTNode* arith_root);
/**
 * Write the overall asm generating logic
 * if is a node:
 *   call `asm_ralloc()`
 * elif is arithmetic:
 *   call `asm_arithmetic()`
 * else:
 *   call `asm_assign()`
 * return the last reg label in the tree, if is `NO_REG_LABEL` then the asm does not generated
 * 
 * @param root 
 */
void asm_generate(BTNode* root);

void asm_ralloc(BTNode* node) {
    switch (node->data) {
    case ID:
        fprintf(stdout, "MOV r%d [%d]\n", reg_label, get_addr(node->lexeme));
        break;
    case INT:
        fprintf(stdout, "MOV r%d %s\n", reg_label, node->lexeme);
        break;
    default:
        return;
    }
    node->reg = reg_label++;
}

void asm_assign(BTNode* assign_root) {
    asm_generate(assign_root->right);
    fprintf(stdout, "MOV [%d] r%d\n", get_addr(assign_root->left->lexeme), assign_root->right->reg);
    assign_root->reg = assign_root->right->reg;
}

int max(int a, int b) {
    return a > b ? a : b;
}

int get_depth(BTNode* root) {
    return root
        ? 1 + max(get_depth(root->left), get_depth(root->right))
        : 0;
}

#define MIN(a, b) (a < b ? a : b)
#define MAX(a, b) (a > b ? a : b)
void asm_arithmetic(BTNode* arith_root) {
    int release_register = reg_label == 7;
    if (release_register) {
        fprintf(stdout, "MOV [%d] r6\n", mem_label * 4);
        reg_label--;
        mem_label--;
    }

    if (get_depth(arith_root->left) >= get_depth(arith_root->right)) {
        asm_generate(arith_root->left);
        asm_generate(arith_root->right);
    } else {
        asm_generate(arith_root->right);
        asm_generate(arith_root->left);
    }

    int small_reg = MIN(arith_root->left->reg, arith_root->right->reg);
    int large_reg = MAX(arith_root->left->reg, arith_root->right->reg);

    int former_reg = release_register ? large_reg : small_reg;
    int latter_reg = release_register ? small_reg : large_reg;

    switch (arith_root->lexeme[0]) {
    case '+':
        fprintf(stdout, "ADD r%d r%d\n", former_reg, latter_reg);
        break;
    case '-':
        fprintf(stdout, "SUB r%d r%d\n", arith_root->left->reg, arith_root->right->reg);
        if (arith_root->left->reg == latter_reg)
            fprintf(stdout, "MOV r%d r%d\n", former_reg, latter_reg);
        break;
    case '*':
        fprintf(stdout, "MUL r%d r%d\n", former_reg, latter_reg);
        break;
    case '/':
        fprintf(stdout, "DIV r%d r%d\n", arith_root->left->reg, arith_root->right->reg);
        if (arith_root->left->reg == latter_reg)
            fprintf(stdout, "MOV r%d r%d\n", former_reg, latter_reg);
        break;
    case '|':
        fprintf(stdout, "OR r%d r%d\n", former_reg, latter_reg);
        break;
    case '^':
        fprintf(stdout, "XOR r%d r%d\n", former_reg, latter_reg);
        break;
    case '&':
        fprintf(stdout, "AND r%d r%d\n", former_reg, latter_reg);
        break;
    }

    arith_root->reg = former_reg;

    if (release_register)
        fprintf(stdout, "MOV r6 [%d]\n", ++mem_label * 4);
    else
        reg_label--;
}
#undef MIN
#undef MAX

void asm_generate(BTNode* root) {
    if (!root)
        return;
    switch (root->data) {
    case ID:
    case INT:
        asm_ralloc(root);
        break;
    case ASSIGN:
        asm_assign(root);
        break;
    default:
        asm_arithmetic(root);
        break;
    }
}

void generate_assembly(BTNode* root) {
    if (!root)
        return;
    if (root->data == ASSIGN) {
        asm_generate(root);
        reg_label--;
    } else {
        generate_assembly(root->left);
        generate_assembly(root->right);
    }
}

int evaluateTree(BTNode* root) {
    int retval = 0, lv = 0, rv = 0;

    if (root != NULL) {
        switch (root->data) {
            case ID:
                retval = getval(root->lexeme);
                break;
            case INT:
                retval = atoi(root->lexeme);
                break;
            case ASSIGN:
                rv = evaluateTree(root->right);
                retval = setval(root->left->lexeme, rv);
                break;
            case ADDSUB:
            case MULDIV:
                lv = evaluateTree(root->left);
                rv = evaluateTree(root->right);
                if (strcmp(root->lexeme, "+") == 0) {
                    retval = lv + rv;
                } else if (strcmp(root->lexeme, "-") == 0) {
                    retval = lv - rv;
                } else if (strcmp(root->lexeme, "*") == 0) {
                    retval = lv * rv;
                } else if (strcmp(root->lexeme, "/") == 0) {
                    // here if right->data is not INT, then we should not panic!()
                    // if we need exact value of the tree, we should garentee that this tree is pure-number tree
                    if (rv == 0 && root->right->data == INT)
                        error(DIVZERO, "");
                    retval = rv == 0
                        ? lv
                        : lv / rv;
                }
                break;
            case BIT_AND:
            case BIT_OR:
            case BIT_XOR:
                lv = evaluateTree(root->left);
                rv = evaluateTree(root->right);
                if (strcmp(root->lexeme, "&") == 0) {
                    retval = lv & rv;
                } else if (strcmp(root->lexeme, "|") == 0) {
                    retval = lv | rv;
                } else if (strcmp(root->lexeme, "^") == 0) {
                    retval = lv ^ rv;
                }
                break;
            default:
                retval = 0;
        }
    }
    return retval;
}

void printPrefix(BTNode *root) {
    if (root != NULL) {
        printf("%s ", root->lexeme);
        printPrefix(root->left);
        printPrefix(root->right);
    }
}

int main() {
    initTable();
    if (PRINTERR)
        printf(">> ");
    while (1) {
        statement();
    }
    return 0;
}

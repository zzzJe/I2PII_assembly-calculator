#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "parser.h"
#include "codeGen.h"

/**************************************************************************
 *                               -= IDEA =-                               *
 * The main idea is to use grammer relation to build abstruct syntax tree *
 * - `lexeme[]` is always to-be-processed                                 *
 * - use `match()` to check `lexeme[]`'s type                             *
 * - use `getLexeme()` to get underling token data                        *
 * - lastly use `advance()` to hot update `lexeme[]`                      *
 *   in order to make `lexeme[]` to-be-processed                          *
 * - grammer is the following                                             *
 *   - statement                                                          *
 *     - ENDFILE                                                          *
 *     - END                                                              *
 *     - expr END                                                         *
 *   - expr                                                               *
 *     - term expr_tail                                                   *
 *   - expr_tail                                                          *
 *     - ADDSUB term expr_tail                                            *
 *     - NiL                                                              *
 *   - term                                                               *
 *     - factor term_tail                                                 *
 *   - term_tail                                                          *
 *     - MULDIV factor term_tail                                          *
 *     - NiL                                                              *
 *   - factor                                                             *
 *     - INT                                                              *
 *     - ADDSUB INT                                                       *
 *     - ADDSUB ID                                                        *
 *     - ADDSUB LPAREN expr RPAREN                                        *
 *     - ID                                                               *
 *     - ID ASSIGN expr                                                   *
 *     - LPAREN expr RPAREN                                               *
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

int getval(char* str) {
    int i = 0;

    // default: will not exceeed
    for (i = 0; i < sbcount; i++)
        if (strcmp(str, table[i].name) == 0)
            return table[i].val;

    // put the boundary check after the iteration is fine :)
    if (sbcount >= TBLSIZE)
        error(RUNOUT);
    
    strcpy(table[sbcount].name, str);
    table[sbcount].val = 0;
    sbcount++;
    return 0;
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
        error(RUNOUT);
    
    strcpy(table[sbcount].name, str);
    table[sbcount].val = val;
    sbcount++;
    return val;
}

BTNode* makeNode(TokenSet tok, const char* lexe) {
    BTNode* node = (BTNode*)malloc(sizeof(BTNode));
    strcpy(node->lexeme, lexe);
    node->data = tok;
    node->val = 0;
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

// factor := INT |
//           ADDSUB INT |
//           ADDSUB ID  | 
//           ADDSUB LPAREN expr RPAREN
//           ID | 
//           ID ASSIGN expr |
//           LPAREN expr RPAREN |
BTNode* factor(void) {
    BTNode* retp = NULL, *left = NULL;

    if (match(INT)) {
        // INT
        retp = makeNode(INT, getLexeme());
        advance();
    } else if (match(ID)) {
        left = makeNode(ID, getLexeme());
        advance();
        if (!match(ASSIGN)) {
            // ID
            retp = left;
        } else {
            // ID ASSIGN expr
            retp = makeNode(ASSIGN, getLexeme());
            advance();
            retp->left = left;
            retp->right = expr();
        }
    } else if (match(ADDSUB)) {
        retp = makeNode(ADDSUB, getLexeme());
        retp->left = makeNode(INT, "0");
        advance();
        if (match(INT)) {
            // ADDSUB INT
            retp->right = makeNode(INT, getLexeme());
            advance();
        } else if (match(ID)) {
            // ADDSUB ID
            retp->right = makeNode(ID, getLexeme());
            advance();
        } else if (match(LPAREN)) {
            // consume '('
            advance();
            // ADDSUB LPAREN expr RPAREN
            retp->right = expr();
            if (match(RPAREN))
                // consume ')'
                advance();
            else
                // souldpanic!()
                error(MISPAREN);
        } else {
            // unary operator with no expression on the right
            error(NOTNUMID);
        }
    } else if (match(LPAREN)) {
        // consume '('
        advance();
        // LPAREN expr RPAREN
        retp = expr();
        if (match(RPAREN))
            // consume ')'
            advance();
        else
            // souldpanic!()
            error(MISPAREN);
    } else {
        // factor must be the above case
        error(NOTNUMID);
    }
    return retp;
}

// term := factor term_tail
BTNode* term(void) {
    BTNode* node = factor();
    return term_tail(node);
}

// term_tail := MULDIV factor term_tail | NiL
BTNode* term_tail(BTNode* left) {
    BTNode* node = NULL;

    if (match(MULDIV)) {
        node = makeNode(MULDIV, getLexeme());
        advance();
        node->left = left;
        node->right = factor();
        return term_tail(node);
    } else {
        return left;
    }
}

// expr := term expr_tail
BTNode* expr(void) {
    BTNode* node = term();
    return expr_tail(node);
}

// expr_tail := ADDSUB term expr_tail | NiL
BTNode* expr_tail(BTNode* left) {
    BTNode* node = NULL;

    if (match(ADDSUB)) {
        node = makeNode(ADDSUB, getLexeme());
        advance();
        node->left = left;
        node->right = term();
        return expr_tail(node);
    } else {
        return left;
    }
}

// statement := ENDFILE | END | expr END
void statement(void) {
    BTNode* retp = NULL;

    if (match(ENDFILE)) {
        exit(0);
    } else if (match(END)) {
        printf(">> ");
        advance();
    } else {
        retp = expr();
        if (match(END)) {
            printf("%d\n", evaluateTree(retp));
            printf("Prefix traversal: ");
            printPrefix(retp);
            printf("\n");
            freeTree(retp);
            printf(">> ");
            advance();
        } else {
            error(SYNTAXERR);
        }
    }
}

void err(ErrorType errorNum) {
    if (PRINTERR) {
        fprintf(stderr, "error: ");
        switch (errorNum) {
            case MISPAREN:
                fprintf(stderr, "mismatched parenthesis\n");
                break;
            case NOTNUMID:
                fprintf(stderr, "number or identifier expected\n");
                break;
            case NOTFOUND:
                fprintf(stderr, "variable not defined\n");
                break;
            case RUNOUT:
                fprintf(stderr, "out of memory\n");
                break;
            case NOTLVAL:
                fprintf(stderr, "lvalue required as an operand\n");
                break;
            case DIVZERO:
                fprintf(stderr, "divide by constant zero\n");
                break;
            case SYNTAXERR:
                fprintf(stderr, "syntax error\n");
                break;
            default:
                fprintf(stderr, "undefined error\n");
                break;
        }
    }
    exit(0);
}

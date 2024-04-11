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

int getval(char* str) {
    int i = 0;

    // default: will not exceeed
    for (i = 0; i < sbcount; i++)
        if (strcmp(str, table[i].name) == 0)
            return table[i].val;

    // put the boundary check after the iteration is fine :)
    if (sbcount >= TBLSIZE)
        error(RUNOUT, "");
    
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
        error(RUNOUT, "Try to allocate memory on full-capacity stack");
    
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

void statement(void) {
    // 00. statement
    //   - ENDFILE
    //   - END
    //   - assign_expr END
    BTNode* retp = NULL;

    if (match(ENDFILE)) {
        exit(0);
    } else if (match(END)) {
        printf(">> ");
        advance();
    } else {
        retp = assign_expr();
        if (match(END)) {
            // printf("%d\n", evaluateTree(retp));
            printf("Prefix traversal: ");
            printPrefix(retp);
            printf("\n");
            freeTree(retp);
            printf(">> ");
            advance();
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
            error(SYNTAXERR, "Assign must be on the lvalue");
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

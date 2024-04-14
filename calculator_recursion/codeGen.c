#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "codeGen.h"
#define MIN(a, b) (a < b ? a : b)
#define MAX(a, b) (a > b ? a : b)
#define IS_LEAF(n) (n && !n->left && !n->right)

static int reg_label = 0;

/**
 * Write a register registration on stdin and return the allocated rege label
 * 
 * @param node 
 */
static void asm_ralloc(BTNode* node);
/**
 * Write a assign asm statement on stdin by a valid tree
 * the tree should have `=` rooted
 * 
 * @param assign_root 
 */
static void asm_assign(BTNode* assign_root);
/**
 * Write a arithmetic asm statement on stdin by valid arthmetic tree
 * 
 * @param arith_root 
 */
static void asm_arithmetic(BTNode* arith_root);
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
static void asm_generate(BTNode* root);

static void asm_ralloc(BTNode* node) {
    switch (node->data) {
    case ID:
        fprintf(stdout, "MOV r%d [%d]\n", reg_label, get_addr(node->lexeme));
        break;
    case INT:
        fprintf(stdout, "MOV r%d %d\n", reg_label, atoi(node->lexeme));
        break;
    default:
        return;
    }
    node->reg = reg_label++;
}

static void asm_assign(BTNode* assign_root) {
    if (assign_root->right->reg == NO_REG_LABEL)
        asm_generate(assign_root->right);
    fprintf(stdout, "MOV [%d] r%d\n", get_addr(assign_root->left->lexeme), assign_root->right->reg);
    assign_root->reg = assign_root->right->reg;
}

static void asm_arithmetic(BTNode* arith_root) {
    // do tree first
    if (arith_root->left->reg == NO_REG_LABEL && !IS_LEAF(arith_root->left))
        asm_generate(arith_root->left);
    if (arith_root->right->reg == NO_REG_LABEL && !IS_LEAF(arith_root->right))
        asm_generate(arith_root->right);
    // then do single node
    if (arith_root->left->reg == NO_REG_LABEL)
        asm_generate(arith_root->left);
    if (arith_root->right->reg == NO_REG_LABEL)
        asm_generate(arith_root->right);

    int small_reg = MIN(arith_root->left->reg, arith_root->right->reg);
    int large_reg = MAX(arith_root->left->reg, arith_root->right->reg);

    switch (arith_root->lexeme[0]) {
    case '+':
        fprintf(stdout, "ADD r%d r%d\n", small_reg, large_reg);
        break;
    case '-':
        fprintf(stdout, "SUB r%d r%d\n", arith_root->left->reg, arith_root->right->reg);
        if (arith_root->left->reg == large_reg)
            fprintf(stdout, "MOV r%d r%d\n", small_reg, large_reg);
        break;
    case '*':
        fprintf(stdout, "MUL r%d r%d\n", small_reg, large_reg);
        break;
    case '/':
        fprintf(stdout, "DIV r%d r%d\n", arith_root->left->reg, arith_root->right->reg);
        if (arith_root->left->reg == large_reg)
            fprintf(stdout, "MOV r%d r%d\n", small_reg, large_reg);
        break;
    case '|':
        fprintf(stdout, "OR r%d r%d\n", small_reg, large_reg);
        break;
    case '^':
        fprintf(stdout, "XOR r%d r%d\n", small_reg, large_reg);
        break;
    case '&':
        fprintf(stdout, "AND r%d r%d\n", small_reg, large_reg);
        break;
    }

    arith_root->reg = small_reg;
    reg_label--;
}

static void asm_generate(BTNode* root) {
    if (!root)
        return;
    if (root->data == ID || root->data == INT)
        asm_ralloc(root);
    else if (root->data == ASSIGN)
        asm_assign(root);
    else
        asm_arithmetic(root);
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
                    if (rv == 0)
                        error(DIVZERO, "");
                    retval = lv / rv;
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

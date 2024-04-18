#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "codeGen.h"

static int reg_label = 0;
static int mem_label = 63;

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
        fprintf(stdout, "MOV r%d %s\n", reg_label, node->lexeme);
        break;
    default:
        return;
    }
    node->reg = reg_label++;
}

static void asm_assign(BTNode* assign_root) {
    asm_generate(assign_root->right);
    fprintf(stdout, "MOV [%d] r%d\n", get_addr(assign_root->left->lexeme), assign_root->right->reg);
    assign_root->reg = assign_root->right->reg;
}

static int max(int a, int b) {
    return a > b ? a : b;
}

static int get_depth(BTNode* root) {
    return root
        ? 1 + max(get_depth(root->left), get_depth(root->right))
        : 0;
}

#define MIN(a, b) (a < b ? a : b)
#define MAX(a, b) (a > b ? a : b)
static void asm_arithmetic(BTNode* arith_root) {
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

static void asm_generate(BTNode* root) {
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

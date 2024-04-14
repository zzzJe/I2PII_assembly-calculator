#ifndef __CODEGEN__
#define __CODEGEN__

#include "parser.h"

// Evaluate the syntax tree
extern int evaluateTree(BTNode *root);

/**
 * Generate necessary asm
 * recurse to `TokenSet::ASSIGN` to generate asm, others need not to generate
 * 
 * @param root 
 */
extern void generate_assembly(BTNode* root);

// Print the syntax tree in prefix
extern void printPrefix(BTNode *root);

#endif // __CODEGEN__

# I2PII mini project 1 Assembly Generator

## TODO

- effectively simplify the expression
  - for `1 + x + 1`, it should be simplified as `2 + x`
  - for `(1 + x) + (1 + x)`, it should be simplified as `2 * (x + 1)`
  - for `(1 + x) + (3 + x)`, it should be simplified as `2 * x + 4`
  - for `(1 + x) + (3 + y)`, it should be simplified as `y + x + 4`
  - for `x * 3 + y - (x + y + z) * 2`, it should be simplified as `x - y - 2 * z`
  - these simplification is complicated to implement by just inspect the ast, should spend some time think about it
- FOR `SUB rx, ry, rz` VER. in [codeGen](./calculator_recursion/codeGen.c), `asm_arithmetic`, `-` and `/` are position-sensitive, meaning that associativity is not true here
  - we introduce depth-first recursion to avoid insufficient registers, which may cuz program to swap memory with registers (which is slow!!!)
  - however, my solution is not optimal when there are more than 2 args in `SUB`
    ```c
    /// 3 args ver
    switch() {
    ...
    case '-':
      fprintf(stdout, "SUB r%d r%d r%d\n", arith_root->left->reg, arith_root->left->reg, arith_root->right->reg);
      if (arith_root->left->reg == latter_reg)
          fprintf(stdout, "MOV r%d r%d\n", former_reg, latter_reg);
      break;
    ...
    }
    ```
    the last line garentee the correctness of the commitment on ***user smaller register first***
  - there is a appliable patch, we can judge the situation (`arith_root->left->reg == latter_reg` part), and only determine the destination register
  - so the optimized version would be
    ```c
    /// 3 args ver
    switch() {
    ...
    case '-': {
      fprintf(
        stdout, "SUB r%d r%d r%d\n",
        arith_root->left->reg == latter_reg
          ? arith_root->right->reg
          : arith_root->left->reg,
        arith_root->left->reg,
        arith_root->right->reg
      );
      break;
    }
    ...
    ```
- simplification when encountering `ASSIGN` tree
  - for `++x / ++x`, it should be parsed as `++x` two times while directly evaluated as `1` as its final ast
  - this simplification is hard to implement too, but for practical project, this should be a serious optimization issue
- for optimization
  - when encountering `(subtree) (*) (0)`
    - DON'T ignore this tree directly, cuz `subtree` may have **mutation tree** inside
    - DO call [`generate_assembly`](./calculator_recursion/codeGen.c) on `subtree`
      - this function recursively detecting **mutation tree** to handle it, and ignore pure **computation tree**
      - and this is what we want!
      - `generate_assembly` will do only necessary sub `subtree` nodes
  - when optimizing expression for **computation tree**
    - the tree would be like (no **mutation** here)
      ```
       [+]            |           [+]
       / \            |           / \
      1  [+]          |         [+] [+]
         / \          |        /  | |  \
        x  [+]        |     [+] [+] [+] [+]
           / \        |     | | | | | | | |
          1  [+]      |     x 1 y 1 z 1 x y
             / \      |
            y  [+]    |
               / \    |
              1  [+]  |
                 / \  |
                z   1 |
      ```
      or something else, which is impossible to solve it by simple pattern matching and node swapping
      - assume that there is no `/`
        - design a function to evaluate and count each categories
        - ||const|x|y|z|a|...|
          |:-:|:-:|:-:|:-:|:-:|:-:|:-:|
          |count|3|1|0|1|6|...|
        - when encountering `*` with **constant tree**, whos value is `n`, muliply all entry value with `n`
        - when encountering `ID` or `INT`, add/sub corresponding entry value
        - dump the table into a new tree
      - what if `/` is here?
        - if a tree contains `/` subtree
          1. recursively do sub trees of `/`, if sub trees contain no `/`
          2. when it's done, keep the `/` tree untouched (something like add a special entry in table) and do the counting optimization on the rest of the parts
        - the algorithm will be:
          - for trees without `/`:
            - do counting optimization
            - dump
          - for trees with `/` **rooted**:
            - recursive call this algorithm on subtrees (subtrees may be `/` rooted or not)
            - put the whole tree (with `/` root) into table entry
          - for trees with `/` but not `/` **rooted**:
            - treat `/` rooted tree as a special table entry
            - do rest of the counting algorithm
            - dump (for special entry, paste the tree directly)

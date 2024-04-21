# I2PII mini project 1 Assembly Generator

## TODO

- effectively simplify the expression
  - for `1 + x + 1`, it should be simplified as `2 + x`
  - for `(1 + x) + (1 + x)`, it should be simplified as `2 * (x + 1)`
  - for `(1 + x) + (3 + x)`, it should be simplified as `2 * x + 4`
  - for `(1 + x) + (3 + y)`, it should be simplified as `y + x + 4`
  - for `x * 3 + y - (x + y + z) * 2`, it should be simplified as `x - y - 2 * z`
  - these simplification is complicated to implement by just inspect the ast, should spend some time think about it
- simplification when encountering `ASSIGN` tree
  - for `++x / ++x`, it should be parsed as `++x` two times while directly evaluated as `1` as its final ast
  - this simplification is hard to implement too, but for practical project, this should be a serious optimization issue

# Data lab
- `int howManyBits(int x)`: return the minimum number of bits required to represent x in two's complement.
    - key: remove redundant sign bits at head, find the first appearance position of 1, **binary search + conditional shift**

# cmu 15-213
Programming labs and resources at cmu 15-213, finished in my self-study.
## About this course
- *tags*: **computer architecture**, **computer system**, **C**, **assembly language**
- *study time*: 2023 Summer
- *course homepage*: [15-213 Introduction to Computer Systems](https://www.cs.cmu.edu/~213/)
- *Textbook*: CS:APP3e

## Useful links
- Course Videos
  - [15-213 cmu, 2015](https://www.bilibili.com/video/BV1iW411d7hd)
  - [中文讲解](https://www.bilibili.com/video/BV1cD4y1D7uR/)
- [Lab Assignments](http://csapp.cs.cmu.edu/3e/labs.html)
- GDB: 
   -  [Beej's Quick Guide to GDB](https://beej.us/guide/bggdb/)
   -  [Cheat Sheet](http://csapp.cs.cmu.edu/3e/docs/gdbnotes-x86-64.pdf)
- [Norman Matloff's Unix and Linux Tutorial Center](https://heather.cs.ucdavis.edu/~matloff/unix.html)

## Projects list
Notes for some tricky problems in the labs:

### Data lab
> How to operate integers and floating points in the bit-level? 
1. `int howManyBits(int x)`: return the minimum number of bits required to represent x in two's complement.
    - key: remove redundant sign bits at head, find the first appearance position of 1, **binary search + conditional shift**

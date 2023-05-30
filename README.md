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

### Bomb lab
> Understand programs in the machine-level.

6 Phases:
1. be familiar with gdb commands
2. if-else, while loop
3. switch, jump-table
4. recursive function, shift

#### phase 5
1. check the length of input string
2. for loop, select chars from another string.
```c
char *input = "your string";
char *str = 0x4024b0;
char *res = %rsp + 0x10;
for (int i = 0; i < 6; i++) {
    int temp = input[i] & 0xf;
    res[i] = str[temp];
}
```
3. string match test

#### Phase 6
1. input string to 6 numbers, which must be in [1, 6] and unique.
2. number = 7 - number
3. rearrange the **linked list** by the order of numbers:
```c
int numbers[6];           // numbers = %rsp
struct node* list[6];     // list = %rsp + 0x20

for (int i = 0; i < 6; i++) {
    %edx = head;
    if (numbers[i] > 1)  {
        int temp = 1;
        while (temp < numbers[i]) {
            %edx = %edx->next;
            temp++;
        }
    }
    list[i] = %edx;
}

struct node *ptr = list[0];
for (int i = 1; i < 6; i++) {
    ptr->next = list[i];
    ptr = list[i];
}
list[5]->next = NULL;
```
4. check the ordering of linked-list:
```c
struct node *ptr = list[0];
for (int i = 5; i > 0; i--) {
    int val = ptr->next->val;
    if (ptr->val < val)
        explode();
    else
        ptr = ptr->next;
}
```

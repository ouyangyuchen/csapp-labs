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

## Labs
*Some notes for each lab.*
### Data lab
> How to operate integers and floating points in the bit-level? 
<details> <summary>howManyBits</summary>

- `int howManyBits(int x)`: return the minimum number of bits required to represent x in two's complement.
    - key: remove redundant sign bits at head, find the first appearance position of 1, **binary search + conditional shift**

</details>

### Bomb lab
> Understand programs in the machine-level.

`./bomb/strs` *is my answer*

phase 1-4
1. be familiar with gdb commands
2. if-else, while loop
3. switch, jump-table
4. recursive function, shift

<details> <summary>phase 5</summary>

1. check the length of input string.

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

</details>

<details><summary> phase 6 </summary>

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

</details>

### Attack lab
> Be careful about buffer overflow with rough memory writing.

#### Part 1: Code Injection Attacks
input string = commands in bytes code + filled bytes + address of the commands

<details><summary> commands </summary>

|level   |commands   |
|---|---|
|2   |move cookie to `%rdi`, push `&&touch2`, return|
|3| recover memory on stack (avoid being overwritten in `hexmatch()`), move &str to `%rdi`, push `&&touch3`, return

</details>

#### Part 2: Return-Oriented Programming
The stack has uncertain address and is non-executable. Therefore, we are unable to overwrite the address of command string.
However, the address of text/code is fixed. We can look into the machine code representation of functions, and extract valid parts of commands before return.

<details><summary> level 2 </summary>

1. notice that in the machine code of `getval_280()`, we can retrieve the value from stack to `%rax`.
```
00000000004019ca <getval_280>:
  4019ca:	b8 29 58 90 c3       	mov    $0xc3905829,%eax
  4019cf:	c3                   	ret    
58: popq %rax
90: nop
c3: ret
```
2. in the `setval_426()` function, we can transfer data to `%rdi`, which is the parameter of touch2.
```
00000000004019c3 <setval_426>:
  4019c3:	c7 07 48 89 c7 90    	movl   $0x90c78948,(%rdi)
  4019c9:	c3                   	ret    
48 89 c7: movq %rax, %rdi
90: nop
c3: ret
```
3. build the string data from the stack architecture:
```
[Before gets]
return address of test()
buffer
<-- %rsp

[After gets]
touch2
addr_setval_426
cookie value
addr_getval_280 <-- original return address
...(any value to fill the buffer)
<-- %rsp
```
where `addr_setval_426` and `addr_getval_280` are the corresbonding addresses of gadgets.

</details>

<details><summary> level 3 </summary>

**key problem**: We cannot update %rsp by the given gadgets. In order to place the data upside the stack pointer, we need to calculate how many **pop** operations are required.

```
mov %rsp, %rax              addval_190
mov %rax, $rdi              setval_426
pop %rax                    getval_280
mov %eax, %edx              addval_487
mov %edx, %ecx              getval_159
mov %ecx, %esi              addval_187
leaq (%rdi,%rsi,1), %rax    add_xy
mov %rax, %rdi              setval_426
```

</details>
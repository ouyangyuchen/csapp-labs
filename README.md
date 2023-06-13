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

<details><summary>phase 1-4</summary>

1. be familiar with gdb commands
2. if-else, while loop
3. switch, jump-table
4. recursive function, shift

  </details>
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

<details><summary>Part 1: Code Injection Attacks</summary>

input string = commands in bytes code + filled bytes + address of the commands

|level   |commands   |
|---|---|
|2   |move cookie to `%rdi`, push `&&touch2`, return|
|3| recover memory on stack (avoid being overwritten in `hexmatch()`), move &str to `%rdi`, push `&&touch3`, return

</details>

<details> <summary>Part 2: Return-Oriented Programming</summary>

The stack has uncertain address and is non-executable. Therefore, we are unable to overwrite the address of command string.
However, the address of text/code is fixed. We can look into the machine code representation of functions, and extract valid parts of commands before return.
- level 2
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
- level 3

*key problem*: We cannot update %rsp by the given gadgets. In order to place the data upside the stack pointer, we need to calculate how many pop operations are required to calculate the address of hex string.

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
#pop operations = 8 (#instructions) + 1 (pop %rax) = 9.

</details>

### Cache lab
> Optimize performance with undertandings of memory hierarchy.
<details><summary>Part A: Writing a Cache Simulator </summary>

1. parse the command line arguments using `getopt` function
For example, we have
```shell
> ./csim -h -v -s 4 -E 1 -b 2 -t example.txt
```

```c
int opt, s, E, b;
while ((opt = getopt(argc, argv, "hvs:E:b:t:")) != -1) {
  switch (opt) {
  case 's':
    s = atoi(optarg);
    break;
  case 'E':
    E = atoi(optarg);
    break;
  case 'b':
    b = atoi(optarg);
    break;
  case 't':
    file = optarg;
    break;
  default: 
    printf("Unknown option: %c", optopt);
    exit(1);
  }
}
```

2. `fscanf` function is used to parse the words in the text.
3. Extract the set index and tag bits from address (using mask).
4. Each set can be implemented by a linked-list, where every node is a line.

*Linked-list is efficient with `add_head` and `remove_node` operations:*
- LRU (least-recently used) policy corresponds to the order in the linked-list. The most recently used block can be always placed in the head.
- Remove the tail node when the number of blocks is larger than E.

</details>

<details><summary>Part B: Optimizing Matrix Transpose</summary>

cache: $s = 5, E = 1, b = 5$
32 bytes per line = 8 ints per line
capacity $C = 1024$ bytes = 256 ints

- 32 x 32 (Every 8 rows share the same set index)

*basic idea*: **8x8** sub-blocks + read 8 ints in a row of A, write 8 ints in a column of B
*enhancement*: There will be extra conflict misses when read and write entries on the diagonal line, so we can use 8 int variables to save the row to registers or temporarily skip the assignments in the diag.
*misses*: 288 < 300

- 64 x 64 (Every 4 rows share the same set index)

*basic idea*: similar to 32x32 case, divide the matrix into multiple **4x4** sub-blocks.
Because of 8 ints in a set of cache, it is not efficient enough to use just block-wise transposes.
*enhancement*: Transpose with **8x8 block with 4 4x4 sub-blocks**: 

```
---- ----
 2    1
---- ----
 3    4
---- ----
```

1. Transpose the 2nd block first, leave 1st block transposed in the cache
2. Transpose the 1st block while writing the previous entries of 1st block to 3rd block.
    1. Read a column from A's 3rd block to 4 registers
    2. Read a row from B's 1st block to the other 4 registers
    3. Write the column from A to the corresponding row in B's 1st block
    4. Write the row from B's 1st block to its 3rd block
3. Transpose the 4th block

*keys*: **Make full use of the cache from the previous step.**
*misses*: 1180 < 1300

- M = 61, N = 67 (Few conflict misses for no alignments)

*basic idea*: directly apply 8x8 blocks transpose
*misses*: 1994 < 2000

</details>

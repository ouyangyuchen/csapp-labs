# csapp labs
Programming labs and resources at cmu 15-213, finished in my self-study.
## About this course
- *tags*: **computer architecture**, **computer system**, **C**, **assembly language**
- *study semester*: 2015 Fall
- *course homepage*: [15-213 Introduction to Computer Systems](https://www.cs.cmu.edu/~213/)
- *Textbook*: Computer Systems:A Programmer's Perspective (3rd edition)

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
## Data lab
> How to operate integers and floating points in the bit-level? 

**howManyBits**

- `int howManyBits(int x)`: return the minimum number of bits required to represent x in two's complement.
    - key: remove redundant sign bits at head, find the first appearance position of 1, **binary search + conditional shift**

## Bomb lab
> Understand programs in the machine-level.

`./bomb/strs` *is my answer*

### phase 1-4

1. be familiar with gdb commands
2. if-else, while loop
3. switch, jump-table
4. recursive function, shift

### phase 5

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

### phase 6

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

## Attack lab
> Be careful about buffer overflow with rough memory writing.

### Part 1: Code Injection Attacks

input string = commands in bytes code + filled bytes + address of the commands

|level   |commands   |
|---|---|
|2   |move cookie to `%rdi`, push `&&touch2`, return|
|3| recover memory on stack (avoid being overwritten in `hexmatch()`), move &str to `%rdi`, push `&&touch3`, return

### Part 2: Return-Oriented Programming

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


## Cache lab
> Optimize performance with undertandings of memory hierarchy.

### Part A: Writing a Cache Simulator

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

### Part B: Optimizing Matrix Transpose

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

## Shell lab
> Understand exception control flow by building a tiny shell.

A *shell* is an interactive command-line interpreter that runs programs on behalf of the user.
A shell repeatedly prints a prompt, waits for a command line on stdin, and then carries out some action, as directed by the contents of the command line.
The first word in the command line is either the name of a built-in command or the pathname of an executable file. The remaining words are command-line arguments.

Here are the ideas on how to implement the functions and build a process management system:
- `eval()` function:
  1. parse the command line: break apart the whole line into pieces of arguments + decide fg or bg ?
  2. call `builtin_cmd()`: do all things and return if true, otherwise passes control to the process management part.
  3. process management part: now the input is an executable pathname, call `fork()` and `execve()` to create a child process.
    - foreground: `eval()` returns after the child is terminated, then call `wait_fg()` to wait for it.
    - background: no need to wait, continue the parent process.
- `builtin_cmd()` function: **just like basic procedures, it is called inside the shell process**.
  if the command is:
  - "quit": just terminate the parent process (unreaped children will be handled by `init` process)
  - "jobs": call `listjobs()` safely (temporarily block all signals to avoid corruption)
  - "fg" or "bg": call `do_fgbg()`
  - other: error case, throw an error by emitting a message to stdout.
- `do_fgbg()` function: continue the specified process.
  1. parse the second argument, determine the actual process/job it refers to.
  2. three cases available: all send a `SIGCONT` signal first
    - from `ST` to `FG`: state change in the jobs list + waiting
    - from `BG` to `FG`: state change in the jobs list + waiting (the running process will neglect the `SIGCONT` signal by default)
    - from `ST` to `BG`: only need to change the state
- `sigchld_handler`: **the only way to reap the terminated or stopped child process**
  The child process sends a `SIGCHLD` signal to parent if it has stopped or terminated. Here are 3 cases:
  1. terminate normally (`WIFEXITED(status) == 1`)
  2. terminated by receiving a `SIGINT` signal, either from other processes or `sigint_handler`
  3. suspended by receiving a `SIGTSTP` signal, either from other processes or `sigtstp_handler`

  **The handler need to change to the proper state or delete the job from list.** (ensure all `addjob()` are called before `deletejob()`, by blocking `SIGCHLD` signal just before `fork()`)
  The option is `WNOHANG | WUNTRACED` to catch the pid of processes without waiting.
- `wait_fg()`: The key is **`wait_fg` return after the fg process is reaped**
  The foreground process is reaped in the `sigchld_handler`, so do we determine whether it has been reaped?
  Claim: As long as the handler behaves correctly, the reaping activity is immediately followed by `deletejob()`. So when the job is no longer in jobs list, we can assume it has been reaped and then return.

  ```c
  struct job_t* target_job = getjobpid(jobs, fg_pid);   // block all signals temporarily
  while (target_job && target_job->state == FG)
      sleep(1);
  ```
  But we cannot determine the order of `sigchld_handler` and `wait_fg`, so the target job may be NULL (if handler comes first) or until `job.state != FG` (we get the job pointer first, but need to wait for the state change).
- `sigint_handler` and `sigtstp_handler`: catch the `SIGINT` or `SIGTSTP` signal from keyboard (\<Ctrl-c\> or \<Ctrl-z\>), and **pass the corresponding signal to the fg process group**.

## Malloc Lab
> Implement a dynamic memory allocator with high utility and throughput.

In this approach, malloc package uses **segregated double-linked list** to record the current free blocks,
which adopts the **LIFO finding** rule and **coalesce** principle.

- Init:
Extend heap by calling sbrk().
Initialize a number of head nodes in the segregated list, all free-lists share the same tail node (tail.prev doesn't matter)
Initialize prologue and epilogue blocks and mark as allocated (reduce corner cases)
  > NOTE: The heap extends on demand (malloc, realloc). Therefore, the program doesn't allocate space at here.

- Malloc: 
Find a free block at specific size-list, if not found, go to the next list with larger size,
If after traversing lists and still fail to find a block to hold the required 'size' data, extend heap for 'size' bytes.
When the block is found, delete this free block from seglist and place the block (mark as allocated + split)

- Free: 
Unmark the allocated block + coalesce + add to the corresponding list

- Realloc:
  1. new block size < old block size: shrink the current allocated block by calling place()
  2. new block size >= old block size + the next block on heap is free + the total size satisfy the requirement: merge the two blocks and place (next expansion strategy)
  3. new block size >= old block size + the next block is epilogue: extend heap for the exceeded bytes, change size of header and footer
  4. otherwise, simply calling free and malloc for new size

**block structure**
 ```
                                                                bp
free block     | header: size_t 4 | prev: ptr 4 | next: ptr 4 | payload 8*N-8 | footer size_t 4|

                                    bp
alloc block    | header: size_t 4 | payload 8*N | footer size_t 4|
```
where `8*N` is the alignment of space that user wants to allocate.

**segregated list**
```
[index] block size (bytes)
[0]     2^4 = {16 - 24}
[1]     2^5 = {32 - 56}
[2]     2^6 = {64 - 120}
[3]     2^7 = {128 - 248}
...
[7]     2^11 = {2048 - 4088}
[8]     >= 2^12
```

## Proxy Lab
> Build a tiny concurrent proxy with caches.

The proxy receives a positive integer from command line, use it as listening port to perform on all requests.

### Part I: Implementing a sequential web proxy
It's easy to build a sequential proxy based on the Tiny Server from textbook.

*basic idea*: The main thread parses the connection request, fetches response from the Web server and resends it back to client.

In part I, the core problem is to parse the request and headers successfuly:
1. Read the first line as HTTP request `$METHOD $URI HTTP/$VERSION`. Validate the method and version.
2. Parse the URI `http://hostname:port/path` to hostname, port number and path. If no explicit port exists, use 80 as port number.
3. Read the remaining lines as headers until `'\r\n'`.
4. Connect to the server based on hostname and port.
5. Rebuild the HTTP request `GET $PATH HTTP/1.0` and other important headers. Send them to the server socket.

### Part II: Dealing with multiple concurrent requests
The proxy uses **shared buffer** (`sbuf.h`, `sbuf.c`) and **prethreading** from textbook.

*main thread*: accept all connections from the listening port and add the fd to buffer.

*other threads*: get a fd from the buffer and perform all operations in part I.

### Part III: Caching web objects
The cache (see `cache.h`, `cache.c`) uses a doubly-linked list to manage multiple lines:
```c
typedef struct line {
    char *uri; // tag to identify the content
    char *data; // entire response from the server last time
    size_t size; // the size of data
    struct line *prev, *next; // pointers to the other line
}
```

Some clients fetch responses that have been cached (case 1), so that they only call **const methods** which are thread-safe for no writing to the cache structure.

On the contrary, the others will update the internal structure of cache lines by calling **non-const methods**, which means the lines they request haven't been cached (case 2).

> How to synchronize?

According to the relationship between requests and reading roles, we use the **first readers-writers mechanism** (multiple readers but only one writer can access the cache) to synchronize and protect the shared cache from multiple threads.
- case 1: pure reader
  - lock before finding the line from buffer successfully, unlock after **copying the data to private local buffer**. 
- case 2: partially reader + writer 
  - lock before finding the line it wants, and unlock after failing (as reader). 
  - lock before adding the data to cache and unlock after finishing adding (as real writer).

> How to determine the eviction policy?

If we use the strict LRU policy. Every read and write operation will update the order of lines:
- read: rearrange the line to the head of list.
- write: create a new line and insert into the head.

So there is no const method for multiple readers to be served simultaneously.

Instead, we use an approximate LRU policy that **don't update the order in reading operation and leave the line untouched.**
Therefore, the `find_line` method is safe for interleaves by multiple threads.

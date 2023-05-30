# Bomb lab

`./bomb/strs` *is the answers.*

## phase 1-4
1. be familiar with gdb commands
2. if-else, while loop
3. switch, jump-table
4. recursive function, shift

## phase 5
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

## phase 6
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
#include "cachelab.h"
#include <stdio.h>
#include <getopt.h>
#include <stdlib.h>
#include <unistd.h>

typedef struct linel{
    unsigned long tag;
    struct linel *next;
} line;

typedef struct {
    line *head;
    size_t size;
} set;

typedef struct {
    set *sets;
} cache;

// find the line with the same tag as input
// return the pointer to this line if it exists, else NULL
line *find(set *s, unsigned long tag) {
    line *ptr = s->head;
    while (ptr) {
        if (ptr->tag == tag) return ptr;
        ptr = ptr->next;
    }
    return NULL;
}

// remove the line without free space.
void rm(set *s, line *find_node) {
    if (find_node == NULL) return;
    s->size--;
    line *ptr = s->head;
    if (ptr == find_node) {
        s->head = ptr->next;
    }
    else {
        while (ptr->next != find_node)
            ptr = ptr->next;
        ptr->next = find_node->next;
    }
}

// add the line to the set
// if the size is larger than capacity E, remove the LRC line and return 1.
int add_to_head(set *s, line *to_add, int E) {
    to_add->next = s->head;
    s->head = to_add;
    s->size++;
    if (s->size > E) {
        // remove the last node in the linked list
        line *ptr = s->head;
        while (ptr->next->next)
            ptr = ptr->next;
        free(ptr->next);
        ptr->next = NULL;
        s->size--;
        return 1;
    }
    return 0;
}

int main(int argc, char **argv)
{
    // parse input options
    int s = 0, E = 0, b = 0;
    char *file = NULL;
    int opt;
    while ((opt = getopt(argc, argv, "s:E:b:t:")) != -1) {
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
        default: /* '?' */
            printf("Unknown option: %c", optopt);
            exit(1);
        }
    }

    // create the cache
    int S = 0x1 << s;
    cache cache_t;
    cache_t.sets = malloc(sizeof(set) * S);         // 2^s sets in total
    for (int i = 0; i < S; i++) {
        cache_t.sets[i].head = NULL;
        cache_t.sets[i].size = 0;
    }
    
    FILE *fp = fopen(file, "r");
    int c = 0, num;            // c: type of memory access, num: bytes accessed
    unsigned long addr;         // address 64-bit
    static unsigned long tag_mask, set_mask;
    // extract tag | set_index | offset in address
    //  bits: 64-s-b    s          b
    set_mask = (0x1 << (s + b)) - (0x1 << b);
    tag_mask = ~((0x1 << (s + b)) - 1);
    // read lines from file
    int misses = 0, hits = 0, evicts = 0;
    while ((fscanf(fp, " %c %lx,%d\n", (char *)&c, &addr, &num)) != EOF) {
        // neglect instruction caching
        if (c == 'I') continue;

        unsigned tag = (addr & tag_mask) >> (s + b);
        unsigned set_index = (addr & set_mask) >> b;
        // add the line to cache
        set *here = &cache_t.sets[set_index];
        line *match_line = find(here, tag);
        int match = match_line != NULL;
        rm(here, match_line);
        line *to_be_add = match ? match_line : malloc(sizeof(line));
        to_be_add->tag = tag;

        misses += !match;
        hits += match;
        evicts += add_to_head(here, to_be_add, E);

        // if modify mode, only add 1 to hits.
        hits += c == 'M';
        char *message = (match) ? "hit" : "miss";
        printf("%c %lx,%d %s\n", c, addr, num, message);
    }
    fclose(fp);

    printSummary(hits, misses, evicts);
    return 0;
}

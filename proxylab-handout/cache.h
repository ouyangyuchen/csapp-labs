#ifndef __CACHE_H__
#define __CACHE_H__

#include "csapp.h"
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Recommended max cache and object sizes */
#define MAX_CACHE_SIZE 1049000
#define MAX_OBJECT_SIZE 102400
#define MAX_LINES 8

typedef struct line {
  char *file_data;
  struct line *prev;
  struct line *next;
  char uri_tag[MAXLINE];
} line_t;

typedef struct {
  line_t *head;
  line_t *tail;
  size_t num;
} cache_t;


void cache_init();

char *fetch_cache(const char *uri);

int add_to_cache(const char *uri, const char *data, size_t size);

#endif // !__CACHE_H__

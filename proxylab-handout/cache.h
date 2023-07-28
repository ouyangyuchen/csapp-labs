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
  struct line *prev;
  struct line *next;
  char *file_data;
  size_t size;
  char uri_tag[MAXLINE];
} line_t;

typedef struct {
  line_t *head;
  line_t *tail;
  size_t num;
} cache_t;


void cache_init();

size_t fetch_cache(const char *uri, char **datap);

int add_to_cache(const char *uri, const char *data, size_t size);

#endif // !__CACHE_H__

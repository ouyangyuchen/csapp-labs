#include "cache.h"
#include "csapp.h"
#include <stddef.h>
#include <string.h>

static cache_t cache;

static line_t *find_line(const char *uri);
static void remove_line(line_t *target_line, int is_free);
static int insert_line(line_t *curr_line);

/* Initialize the cache with 0 line. */
void cache_init() {
  cache.head = Malloc(sizeof(line_t));
  cache.head->file_data = NULL;
  sprintf(cache.head->uri_tag, "");

  cache.tail = Malloc(sizeof(line_t));
  cache.tail->file_data = NULL;
  sprintf(cache.tail->uri_tag, "");

  cache.head->prev = NULL;
  cache.head->next = cache.tail;
  cache.tail->prev = cache.head;
  cache.tail->next = NULL;

  cache.num = 0;
}

/* TODO
 * Fetch data from cache
 * if uri matched a line -> return the file
 * else -> fetch data from web server
 *   if the file-length is too large -> not insert
 *   else -> insert to the head of cache.
 *     if num > MAXLINES -> remove the last line
 * return the file/data pointer*/

/* 
 * Fetch response from cache. 
 * Return the data if the request exists, else return NULL.
 */
char *fetch_cache(const char *uri) {
  line_t *target = find_line(uri);
  if (target) {
    remove_line(target, 0);
    insert_line(target);
    return target->file_data;
  } 
  else {
    return NULL;
  }
}

/* Add the uri and corresponding response data to cache. */
int add_to_cache(const char *uri, const char *data, size_t size) {
  if (size > MAX_OBJECT_SIZE)
    return -1;

  line_t *newline = Malloc(sizeof(line_t));
  newline->file_data = Malloc(MAX_OBJECT_SIZE);
  strncpy(newline->uri_tag, uri, MAXLINE);
  strncpy(newline->file_data, data, size);

  insert_line(newline);
  return 0;
}

/*
 * Insert a line to the head of cache list.
 */
static int insert_line(line_t *curr_line) {
  if (curr_line == NULL)
    return -1;

  /* Add to the linked list */
  curr_line->next = cache.head->next;
  curr_line->next->prev = curr_line;
  cache.head->next = curr_line;
  curr_line->prev = cache.head;

  cache.num++;
  if (cache.num > MAX_LINES)
    remove_line(find_line(NULL), 1);
  return 0;
}

/*
 * Remove the line from cache.
 * if is_free = 1, free the space it allocates.
 * if is_free = 0, directly return.
 */
static void remove_line(line_t *target_line, int is_free) {
  if (target_line == NULL)
    return;

  /* target -> the line need to be removed */
  target_line->prev->next = target_line->next;
  target_line->next->prev = target_line->prev;
  cache.num--;

  if (is_free) {
    Free(target_line->file_data);
    Free(target_line);
  }
}

/*
 * Find the line with given uri, return NULL if not found.
 * if uri = NULL, return the last line in the cache.
 */
static line_t *find_line(const char *uri) {
  if (cache.num == 0)
    return NULL;

  line_t *target;
  if (uri) {
    /* find the line with the same uri */
    for (target = cache.head->next; target != cache.tail;
         target = target->next) {
      if (strcmp(target->uri_tag, uri) == 0)
        return target;
    }
    return NULL;
  } 
  else {
    /* find the last line */
    return cache.tail->prev;
  }
}

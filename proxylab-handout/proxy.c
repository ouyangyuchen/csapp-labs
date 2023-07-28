#include "csapp.h"
#include "sbuf.h"
#include "cache.h"
#include <pthread.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <unistd.h>

#define MAX_HEADERS 10 /* max number of headers in a HTTP request */

#define MAX_THREADS 4 /* max number of threads in the poll */

#define SBUFSIZE 16 /* number of fd contained in the shared buffer */

void *thread(void *vargp);
void doit(int fd);
void clienterror(int fd, const char *cause, const char *errnum,
                 const char *shortmsg, const char *longmsg);
int read_requesthdrs(rio_t *rp, char **headers);
void free_hdrs(char **headers, int header_num);
int valid_req(int fd, const char *method, const char *uri, const char *version);
void parse_uri(const char *uri, char *host, char *port, char *path);
void send_req(int serverfd, const char *host, const char *path,
              const char **header, int header_num);
size_t fetch_server(int serverfd, char *buf);
void resend(int clientfd, char *buf, size_t size);

/* You won't lose style points for including this long line in your code */
static const char *user_agent_hdr =
    "User-Agent: Mozilla/5.0 (X11; Linux x86_64; rv:10.0.3) Gecko/20120305 "
    "Firefox/10.0.3\r\n";
static const char *conn_hdr = "Connection: close\r\n";
static const char *proxy_conn_hdr = "Proxy-Connection: close\r\n";

sbuf_t sbuf; /* shared connection fd buffer */

int main(int argc, char *argv[]) {
  int listenfd, connfd;
  struct sockaddr_storage clientaddr;
  socklen_t clientlen;

  /* Read from command line to get the port number */
  if (argc != 2) {
    fprintf(stderr, "usage: %s <port>\n", argv[0]);
    exit(1);
  }
  listenfd = Open_listenfd(argv[1]);

  sbuf_init(&sbuf, SBUFSIZE);
  pthread_t tid;
  for (int i = 0; i < MAX_THREADS; i++) {
    Pthread_create(&tid, NULL, thread, NULL); // create thread pool
  }

  cache_init();

  while (1) {
    clientlen = sizeof(clientaddr);
    connfd = Accept(listenfd, (SA *)&clientaddr, &clientlen);
    sbuf_insert(&sbuf, connfd); // insert the client to sbuf
  }
  return 0;
}

void *thread(void *vargp) {
  Pthread_detach(Pthread_self()); // detach itself
  while (1) {
    int connfd = sbuf_remove(&sbuf); // retrieve a client from sbuf
    doit(connfd);
    Close(connfd);
  }
}

/* 
 * Perform a HTTP transaction according to the request from client.
 * Send the request modified to the server and fetch data back to client.
 */
void doit(int clientfd) {
  rio_t rio;
  char buf[MAXLINE], method[MAXLINE], uri[MAXLINE], version[MAXLINE];
  char *headers[MAX_HEADERS];
  for (int i = 0; i < MAX_HEADERS; i++) {
    headers[i] = malloc(MAXLINE);
    if (headers[i] == NULL)
      app_error("Not enough heap memory for storing headers\n");
  }

  Rio_readinitb(&rio, clientfd);
  Rio_readlineb(&rio, buf, MAXLINE);          // read http request
  int hdrs = read_requesthdrs(&rio, headers); // read http headers line by line

  sscanf(buf, "%s %s %s", method, uri, version);
  if (valid_req(clientfd, method, uri, version)) {
    free_hdrs(headers, hdrs);
    return;
  }

  char response_buf[MAX_CACHE_SIZE];
  char *tempbuf;
  size_t read_size = fetch_cache(uri, &tempbuf);
  if (tempbuf) {
    /* read response directly from cache */
    strncpy(response_buf, tempbuf, read_size);
  }
  else {
    /* Parse uri to host, port number, path */
    char host[MAXLINE], port[10], path[MAXLINE];
    parse_uri(uri, host, port, path);
    printf("uri: %s\n", uri);
    printf("Host: %s\n", host);
    printf("Port: %s\n", port);
    printf("Path: %s\n\n", path);

    /* Connect the Web server host:port, send the HTTP/1.0 request and headers */
    int connfd = Open_clientfd(host, port);
    send_req(connfd, host, path, headers, hdrs);

    /* Retrieve data from server */
    read_size = fetch_server(connfd, response_buf);
    Close(connfd);

    /* If data size < MAX_OBJECT_SIZE, add object to the cache */
    if (read_size <= MAX_OBJECT_SIZE)
      add_to_cache(uri, response_buf, read_size);
  }
  /* Send the data in memory to client */
  resend(clientfd, response_buf, read_size);

  free_hdrs(headers, hdrs);
}

/*
 * Send GET HTTP/1.0 request and headers to the server.
 */
void send_req(int serverfd, const char *host, const char *path,
              const char **headers, int header_num) {
  /* send the request */
  char buf[MAXLINE];
  sprintf(buf, "GET %s HTTP/1.0\r\n", path);
  Rio_writen(serverfd, buf, strlen(buf));

  /* send Host header */
  int host_hd_index = -1;
  for (int i = 0; i < header_num; i++) {
    if (strstr(headers[i], "Host: ")) {
      host_hd_index = i;
      sprintf(buf, "%s", headers[i]); // use host header from web browser
      break;
    }
  }
  if (host_hd_index < 0)
    sprintf(buf, "Host: %s\r\n", host);
  Rio_writen(serverfd, buf, strlen(buf));

  /* send user-agent header, connection header, proxy-connection header */
  sprintf(buf, "%s%s%s", user_agent_hdr, conn_hdr, proxy_conn_hdr);
  Rio_writen(serverfd, buf, strlen(buf));

  /* send the remaining headers */
  sprintf(buf, "");
  for (int i = 0; i < header_num; i++) {
    if (i == host_hd_index)
      continue;
    strcat(buf, headers[i]);
  }
  if (strlen(buf) > 0)
    Rio_writen(serverfd, buf, strlen(buf));
  Rio_writen(serverfd, "\r\n", 2);
}

size_t fetch_server(int serverfd, char *buf) {
  rio_t rio;
  Rio_readinitb(&rio, serverfd);

  size_t num = Rio_readnb(&rio, buf, MAX_CACHE_SIZE);
  if (num == 0)
    printf("Truncate the response.\n");
  return num;
}

void resend(int clientfd, char *buf, size_t size) {
  Rio_writen(clientfd, buf, size);
}

/*
 * read the HTTP headers after request line to the buffer.
 * return the number of headers have been read.
 */
int read_requesthdrs(rio_t *rp, char **headers) {
  char buf[MAXLINE];
  int lines = 1;

  Rio_readlineb(rp, buf, MAXLINE);
  while (strcmp(buf, "\r\n")) {
    strcpy(*headers, buf);
    headers++;
    lines++;

    Rio_readlineb(rp, buf, MAXLINE);
  }

  return lines - 1;
}

/*
 * Separate http://ip:port/uri (static file path/dynamic params) from http
 * request. host = ip, port = port, path = /uri Caller need to allocate enough
 * space for 3 strings.
 */
void parse_uri(const char *uri, char *host, char *port, char *path) {
  char *ptr, *temp;
  char *ptr1, *ptr2, *ptr3;
  ptr1 = strstr(uri, "//");
  ptr1 += 2;
  ptr2 = strstr(ptr1, ":");
  ptr3 = strstr(ptr1, "/");

  int hostlen = (ptr2 == NULL) ? ptr3 - ptr1 : ptr2 - ptr1;
  strncpy(host, ptr1, hostlen);

  if (ptr2 == NULL)
    strcpy(port, "80");
  else
    strncpy(port, ptr2 + 1, ptr3 - ptr2 - 1);

  strncpy(path, ptr3, MAXLINE);
}

/*
 * send the error message back to the connected socket
 */
void clienterror(int fd, const char *cause, const char *errnum,
                 const char *shortmsg, const char *longmsg) {
  char buf[MAXLINE], body[MAXLINE];

  sprintf(body, "<html><title>Proxy Error</title>");
  sprintf(body, "%s<body bgcolor=" "ffffff" ">\r\n", body);
  sprintf(body, "%s%s: %s\r\n", body, errnum, shortmsg);
  sprintf(body, "%s<p>%s: %s\r\n", body, longmsg, cause);
  sprintf(body, "%s<hr><em>The Tiny Proxy</em>\r\n", body);
  int body_size = strlen(body);

  sprintf(buf, "HTTP/1.0 %s %s\r\n", errnum, shortmsg);
  Rio_writen(fd, buf, strlen(buf));
  sprintf(buf, "Content-type: text/html\r\n");
  Rio_writen(fd, buf, strlen(buf));
  sprintf(buf, "Content-length: %d\r\n\r\n", body_size);
  Rio_writen(fd, buf, strlen(buf));
  Rio_writen(fd, body, body_size);
}

/*
 * Parse the HTTP request, return 0 if valid, 1 if non-valid.
 */
int valid_req(int fd, const char *method, const char *uri,
              const char *version) {
  if (strcasecmp(method, "GET")) {
    clienterror(fd, method, "501", "Not implemented",
                "The proxy doesn't support this method");
    return 1;
  }
  if (!strcasestr(version, "HTTP/1.")) {
    return 1;
  }
  return 0;
}

/*
 * free the buffer to store headers.
 */
void free_hdrs(char **headers, int header_num) {
  for (int i = 0; i < header_num; i++) {
    free(headers[i]);
  }
}

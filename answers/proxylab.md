# 笔记
基于书上代码，比malloclab简单不少

# 代码
```cpp
#include "csapp.h"
#include "sbuf.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define NTHREADS 4
#define SBUFSIZE 16
/* Recommended max cache and object sizes */
#define MAX_CACHE_SIZE 1049000
#define MAX_OBJECT_SIZE 102400
#define MAX_STRING_SIZE 305
#define CACHE_SIZE 103
/* You won't lose style points for including this long line in your code */
static const char *user_agent_hdr = "User-Agent: Mozilla/5.0 (X11; Linux x86_64; rv:10.0.3) Gecko/20120305 Firefox/10.0.3\r\n";
sbuf_t sbuf;
sem_t mutex1, mutex2;
struct Node {
	char s[MAX_STRING_SIZE];
	char *content;
	int last_time;
} cache[CACHE_SIZE];
int cache_num;
void init() {
	for (int i = 0; i < CACHE_SIZE; i++) {
		cache[i].last_time = -1;
		memset(cache[i].s, 0, sizeof(cache[i].s));
		cache[i].content = NULL;
	}
}
int find(int fd, char *host) {
	// return 0;
	for (int i = 0; i < CACHE_SIZE; i++) {
		if (strcmp(host, cache[i].s) == 0) {
			// P(&mutex1);
			Rio_writen(fd, cache[i].content, strlen(cache[i].content));
			// V(&mutex1);
			return 1;
		}
	}
	return 0;
}

void *thread(void *vargp) {
	Pthread_detach(pthread_self());
	while (1) {
		int connfd = sbuf_remove(&sbuf); /* Remove connfd from buffer */ // line:conc:pre:removeconnfd
		doit(connfd);
		Close(connfd);
	}
}

int main(int argc, char **argv) {
	init();
	char hostname[MAXLINE], port[MAXLINE];
	socklen_t clientlen;
	struct sockaddr_storage clientaddr;
	int i, listenfd, connfd;
	// socklen_t clientlen;
	pthread_t tid;

	/* Check command line args */
	if (argc != 2) {
		fprintf(stderr, "usage: %s <port>\n", argv[0]);
		exit(1);
	}

	sbuf_init(&sbuf, SBUFSIZE);								   // line:conc:pre:initsbuf
	for (i = 0; i < NTHREADS; i++) /* Create worker threads */ // line:conc:pre:begincreate
		Pthread_create(&tid, NULL, thread, NULL);			   // line:conc:pre:endcreate

	// while (1) {
	// 	clientlen = sizeof(struct sockaddr_storage);
	// 	connfd = Accept(listenfd, (SA *)&clientaddr, &clientlen);
	// 	sbuf_insert(&sbuf, connfd); /* Insert connfd in buffer */
	// }
	listenfd = Open_listenfd(argv[1]);
	while (1) {
		clientlen = sizeof(clientaddr);
		connfd = Accept(listenfd, (SA *)&clientaddr, &clientlen); // line:netp:tiny:accept
		Getnameinfo((SA *)&clientaddr, clientlen, hostname, MAXLINE, port, MAXLINE, 0);
		printf("Accepted connection from (%s, %s)\n", hostname, port);
		sbuf_insert(&sbuf, connfd);
		// doit(connfd);  // line:netp:tiny:doit
		// Close(connfd); // line:netp:tiny:close
	}
}

void doit(int fd) {
	char clientRequest[MAXLINE];
	char buf[MAXLINE], method[MAXLINE], url[MAXLINE], version[MAXLINE];
	char filename[MAXLINE], host[MAXLINE];
	rio_t rio;

	/* Read request line and headers */
	Rio_readinitb(&rio, fd);
	Rio_readlineb(&rio, buf, MAXLINE);
	// printf("fufufufufuuf  %s", buf);
	// line:netp:doit:parserequest
	// printf("asdfadf  %s", method);

	// line:netp:doit:endrequesterr

	/* Parse URI from GET request */
	// printf("clclcllc0:  %s\n\n", clientRequest);
	parse_uri(buf, method, url, version, host, filename);			// line:netp:doit:staticcheck
	read_requesthdrs(&rio, host, filename, version, clientRequest); // line:netp:doit:readrequesthdrs

	if (strcasecmp(method, "GET") != 0) { // line:netp:doit:beginrequesterr
		clienterror(fd, method, "501", "Not Implemented", "Tiny does not implement this method");
		return;
	} // if (stat(filename, &sbuf) < 0) {			// line:netp:doit:beginnotfound
	// 	clienterror(fd, filename, "404", "Not found", "Tiny couldn't find this file");
	// 	return;
	// } // line:netp:doit:endnotfound
	printf("REQUEST HEADER:\n%s", clientRequest);
	puts("=======================Request Done===============================");

	char *colon = strchr(host, ':');
	char host_name[MAXLINE], port[MAXLINE];
	if (colon != NULL)
		strncpy(host_name, host, colon - host);
	else
		strcpy(host_name, host);
	if (colon != NULL) strncpy(port, colon + 1, strlen(host) - (colon - host + 1));
	printf("hostname=%s\nport=%s\n", host_name, port);
	char response[MAXLINE];
	int clientfd = open_clientfd(host_name, port);
	puts("=======================Connect Client Done===========================");
	rio_t client;
	// printf("clclcllc1:  %s\n\n", clientRequest);
	rio_readinitb(&client, clientfd);
	rio_writen(client.rio_fd, clientRequest, strlen(clientRequest));
	int n;
	if (find(fd, url))
		return;
	else {
		char *s;
		int flag = 0;
		s = malloc(MAX_OBJECT_SIZE * (sizeof(char)));
		s[0] = 0;
		while ((n = Rio_readnb(&client, response, MAXLINE)) != 0) {
			Rio_writen(fd, response, n);
			if (flag == 0 && strlen(s) + n < MAX_OBJECT_SIZE * (sizeof(char)))
				strcat(s, response);
			else
				flag = 1;
		}
		if (flag) goto l1;
		int t_min = 1e8, pos = 0;
		for (int i = 0; i < CACHE_SIZE; i++) {
			if (cache[i].last_time < t_min) {
				t_min = cache[i].last_time;
				pos = i;
			}
		}
		cache_num++;
		cache[pos].last_time = cache_num;

		// P(&mutex);
		// V(&mutex);
		memcpy(cache[pos].s, url, strlen(url));
		if (cache[pos].content != NULL) free(cache[pos].content);
		cache[pos].content = malloc(strlen(s) * sizeof(char) + 8);
		memcpy(cache[pos].content, s, strlen(s));
		// V(&mutex);
	l1:
		free(s);
	}
}

void read_requesthdrs(rio_t *rp, char *host, char *filename, char *version, char *clientRequest) {
	char buf[MAXLINE];
	sprintf(clientRequest, "GET %s %s\r\n", filename, version);
	Rio_readlineb(rp, buf, MAXLINE);
	printf("=-=-=-=-=- %s", buf);
	int flag_host = 0, flag_user_agent = 0, flag_connection = 0, flag_proxy_connection = 0;
	while (strcmp(buf, "\r\n")) { // line:netp:readhdrs:checkterm
		Rio_readlineb(rp, buf, MAXLINE);
		if (strcmp(buf, "\r\n") == 0) break;
		strcat(clientRequest, buf);
		if (strstr(buf, "Host") != NULL) { flag_host = 1; }
		if (strstr(buf, "User-Agent") != NULL) { flag_user_agent = 1; }
		if (strstr(buf, "Connection") != NULL) { flag_connection = 1; }
		if (strstr(buf, "Proxy-Connection") != NULL) { flag_proxy_connection = 1; }
		printf("%s", buf);
	}
	if (!flag_host) {
		char s[MAXLINE];
		sprintf(s, "Host: %s\r\n", host);
		strcat(clientRequest, s);
		// strcat(clientRequest, host);
	}
	if (!flag_user_agent) strcat(clientRequest, user_agent_hdr);
	if (!flag_connection) strcat(clientRequest, "Connection: close\r\n");
	if (!flag_proxy_connection) strcat(clientRequest, "Proxy-Connection: close\r\n");
	strcat(clientRequest, "\r\n");
	return;
}

void parse_uri(char *buf, char *method, char *url, char *version, char *host, char *filename) {
	sscanf(buf, "%s %s %s", method, url, version);
	char *domain_begin = strchr(url, '/') + 2;
	char *domain_last = strchr(domain_begin, '/') - 1;
	char *position_last = url + strlen(url) - 1;
	char *pos = strstr(version, "1.1");
	if (pos != NULL) { pos[2] = '0'; }
	strncpy(host, domain_begin, domain_last - domain_begin + 1);
	strcpy(filename, domain_last + 1);
	// printf("-=-=- host: %s\nfilename: %s\n", host, filename);
}

void clienterror(int fd, char *cause, char *errnum, char *shortmsg, char *longmsg) {
	char buf[MAXLINE];

	/* Print the HTTP response headers */
	sprintf(buf, "HTTP/1.0 %s %s\r\n", errnum, shortmsg);
	Rio_writen(fd, buf, strlen(buf));
	sprintf(buf, "Content-type: text/html\r\n\r\n");
	Rio_writen(fd, buf, strlen(buf));

	/* Print the HTTP response body */
	sprintf(buf, "<html><title>Tiny Error</title>");
	Rio_writen(fd, buf, strlen(buf));
	sprintf(buf, "<body bgcolor="
				 "ffffff"
				 ">\r\n");
	Rio_writen(fd, buf, strlen(buf));
	sprintf(buf, "%s: %s\r\n", errnum, shortmsg);
	Rio_writen(fd, buf, strlen(buf));
	sprintf(buf, "<p>%s: %s\r\n", longmsg, cause);
	Rio_writen(fd, buf, strlen(buf));
	sprintf(buf, "<hr><em>The Tiny Web server</em>\r\n");
	Rio_writen(fd, buf, strlen(buf));
}

```

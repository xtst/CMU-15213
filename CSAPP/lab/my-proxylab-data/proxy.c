#include "csapp.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
/* Recommended max cache and object sizes */
#define MAX_CACHE_SIZE 1049000
#define MAX_OBJECT_SIZE 102400
#define MAX_STRING_SIZE 12345
/* You won't lose style points for including this long line in your code */
static const char *user_agent_hdr = "User-Agent: Mozilla/5.0 (X11; Linux x86_64; rv:10.0.3) Gecko/20120305 Firefox/10.0.3\r\n";

int main(int argc, char **argv) {
	int listenfd;
	char hostname[MAXLINE], port[MAXLINE];
	socklen_t clientlen;
	struct sockaddr_storage clientaddr;

	/* Check command line args */
	if (argc != 2) {
		fprintf(stderr, "usage: %s <port>\n", argv[0]);
		exit(1);
	}

	listenfd = Open_listenfd(argv[1]);
	while (1) {
		clientlen = sizeof(clientaddr);
		int connfd = Accept(listenfd, (SA *)&clientaddr, &clientlen); // line:netp:tiny:accept
		Getnameinfo((SA *)&clientaddr, clientlen, hostname, MAXLINE, port, MAXLINE, 0);
		printf("Accepted connection from (%s, %s)\n", hostname, port);
		doit(connfd);  // line:netp:tiny:doit
		Close(connfd); // line:netp:tiny:close
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
	while ((n = Rio_readnb(&client, response, MAXLINE)) != 0) { Rio_writen(fd, response, n); }
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

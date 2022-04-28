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
char clientRequest[MAXLINE];
void doit(int fd);
void read_requesthdrs(rio_t *rp, char *host);
int parse_uri(char *buf, char *host, char *position);
void serve_static(int fd, char *filename, int filesize);
void get_filetype(char *filename, char *filetype);
void serve_dynamic(int fd, char *filename, char *cgiargs);
void clienterror(int fd, char *cause, char *errnum, char *shortmsg, char *longmsg);

int main(int argc, char **argv) {
	int listenfd, connfd;
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
		connfd = Accept(listenfd, (SA *)&clientaddr, &clientlen); // line:netp:tiny:accept
		Getnameinfo((SA *)&clientaddr, clientlen, hostname, MAXLINE, port, MAXLINE, 0);
		printf("Accepted connection from (%s, %s)\n", hostname, port);
		doit(connfd);  // line:netp:tiny:doit
		Close(connfd); // line:netp:tiny:close
	}
}
/* $end tinymain */

/*
 * doit - handle one HTTP request/response transaction
 */
/* $begin doit */
void doit(int fd) {
	int is_static;
	struct stat sbuf;
	char buf[MAXLINE], method[MAXLINE], uri[MAXLINE], version[MAXLINE];
	char filename[MAXLINE], position[MAXLINE], host[MAXLINE];
	rio_t rio;

	/* Read request line and headers */
	Rio_readinitb(&rio, fd);
	if (!Rio_readlineb(&rio, buf, MAXLINE)) // line:netp:doit:readrequest
		return;
	printf("%s", buf);
	sscanf(buf, "%s %s %s", method, uri, version); // line:netp:doit:parserequest
	if (strcasecmp(method, "GET")) {			   // line:netp:doit:beginrequesterr
		clienterror(fd, method, "501", "Not Implemented", "Tiny does not implement this method");
		return;
	}
	parse_uri(buf, host, position); // line:netp:doit:endrequesterr
	read_requesthdrs(&rio, host);	// line:netp:doit:readrequesthdrs

	/* Parse URI from GET request */
	is_static = parse_uri(uri, filename, cgiargs); // line:netp:doit:staticcheck
	if (stat(filename, &sbuf) < 0) {			   // line:netp:doit:beginnotfound
		clienterror(fd, filename, "404", "Not found", "Tiny couldn't find this file");
		return;
	} // line:netp:doit:endnotfound

	if (is_static) {												 /* Serve static content */
		if (!(S_ISREG(sbuf.st_mode)) || !(S_IRUSR & sbuf.st_mode)) { // line:netp:doit:readable
			clienterror(fd, filename, "403", "Forbidden", "Tiny couldn't read the file");
			return;
		}
		serve_static(fd, filename, sbuf.st_size);					 // line:netp:doit:servestatic
	} else {														 /* Serve dynamic content */
		if (!(S_ISREG(sbuf.st_mode)) || !(S_IXUSR & sbuf.st_mode)) { // line:netp:doit:executable
			clienterror(fd, filename, "403", "Forbidden", "Tiny couldn't run the CGI program");
			return;
		}
		serve_dynamic(fd, filename, cgiargs); // line:netp:doit:servedynamic
	}
}
/* $end doit */

/*
 * read_requesthdrs - read HTTP request headers
 */
/* $begin read_requesthdrs */
void read_requesthdrs(rio_t *rp, char *host) {
	char buf[MAXLINE];

	Rio_readlineb(rp, buf, MAXLINE);
	printf("%s", buf);
	int flag_host = 0, flag_user_agent = 0, flag_connection = 0, flag_proxy_connection = 0;
	while (strcmp(buf, "\r\n")) { // line:netp:readhdrs:checkterm
		Rio_readlineb(rp, buf, MAXLINE);
		strcat(clientRequest, buf);
		if (strstr(buf, "Host") != NULL) { flag_host = 1; }
		if (strstr(buf, "User-Agent") != NULL) { flag_user_agent = 1; }
		if (strstr(buf, "Connection") != NULL) { flag_connection = 1; }
		if (strstr(buf, "Proxy-Connection") != NULL) { flag_proxy_connection = 1; }
		printf("%s", buf);
	}
	if (!flag_host) { strcat(clientRequest, ""); }
	if (!flag_user_agent) strcat(clientRequest, user_agent_hdr);
	if (!flag_connection) strcat(clientRequest, "Connection: close");
	if (!flag_proxy_connection) strcat(clientRequest, "Proxy-Connection: close");
	return;
}
/* $end read_requesthdrs */

/*
 * parse_uri - parse URI into filename and CGI args
 *             return 0 if dynamic content, 1 if static
 */
/* $begin parse_uri */
int parse_uri(char *buf, char *host, char *position) {
	char *domain_begin = strchr(buf, '/') + 2;
	char *domain_last = strchr(domain_begin, '/') - 1;
	char *position_last = strchr(domain_last + 1, ' ') - 1;
	char *pos = strstr(buf, "1.1");
	if (pos != NULL) { pos + 2 = '0'; }
	strncpy(host, domain_begin, domain_last - domain_begin + 1);
	strncpy(position, domain_last + 1, position_last - domain_last);
	printf("host: %s\nposition: %s\n", host, position);
}
/* $end parse_uri */

/*
 * serve_static - copy a file back to the client
 */
/* $begin serve_static */
void serve_static(int fd, char *filename, int filesize) {
	int srcfd;
	char *srcp, filetype[MAXLINE], buf[MAXBUF];

	/* Send response headers to client */
	get_filetype(filename, filetype);	 // line:netp:servestatic:getfiletype
	sprintf(buf, "HTTP/1.0 200 OK\r\n"); // line:netp:servestatic:beginserve
	Rio_writen(fd, buf, strlen(buf));
	sprintf(buf, "Server: Tiny Web Server\r\n");
	Rio_writen(fd, buf, strlen(buf));
	sprintf(buf, "Content-length: %d\r\n", filesize);
	Rio_writen(fd, buf, strlen(buf));
	sprintf(buf, "Content-type: %s\r\n\r\n", filetype);
	Rio_writen(fd, buf, strlen(buf)); // line:netp:servestatic:endserve

	/* Send response body to client */
	srcfd = Open(filename, O_RDONLY, 0);						// line:netp:servestatic:open
	srcp = Mmap(0, filesize, PROT_READ, MAP_PRIVATE, srcfd, 0); // line:netp:servestatic:mmap
	Close(srcfd);												// line:netp:servestatic:close
	Rio_writen(fd, srcp, filesize);								// line:netp:servestatic:write
	Munmap(srcp, filesize);										// line:netp:servestatic:munmap
}

/*
 * get_filetype - derive file type from file name
 */
void get_filetype(char *filename, char *filetype) {
	if (strstr(filename, ".html"))
		strcpy(filetype, "text/html");
	else if (strstr(filename, ".gif"))
		strcpy(filetype, "image/gif");
	else if (strstr(filename, ".png"))
		strcpy(filetype, "image/png");
	else if (strstr(filename, ".jpg"))
		strcpy(filetype, "image/jpeg");
	else
		strcpy(filetype, "text/plain");
}
/* $end serve_static */

/*
 * serve_dynamic - run a CGI program on behalf of the client
 */
/* $begin serve_dynamic */
void serve_dynamic(int fd, char *filename, char *cgiargs) {
	char buf[MAXLINE], *emptylist[] = {NULL};

	/* Return first part of HTTP response */
	sprintf(buf, "HTTP/1.0 200 OK\r\n");
	Rio_writen(fd, buf, strlen(buf));
	sprintf(buf, "Server: Tiny Web Server\r\n");
	Rio_writen(fd, buf, strlen(buf));

	if (Fork() == 0) { /* Child */ // line:netp:servedynamic:fork
		/* Real server would set all CGI vars here */
		setenv("QUERY_STRING", cgiargs, 1);							// line:netp:servedynamic:setenv
		Dup2(fd, STDOUT_FILENO); /* Redirect stdout to client */	// line:netp:servedynamic:dup2
		Execve(filename, emptylist, environ); /* Run CGI program */ // line:netp:servedynamic:execve
	}
	Wait(NULL); /* Parent waits for and reaps child */ // line:netp:servedynamic:wait
}
/* $end serve_dynamic */

/*
 * clienterror - returns an error message to the client
 */
/* $begin clienterror */
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

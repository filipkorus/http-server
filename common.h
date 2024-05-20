#ifndef COMMON_H
#define COMMON_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <errno.h>
#include <pthread.h>

#define PORT 8080
#define BUFFER_SIZE 1024
#define DOCUMENT_ROOT "www"

#define MAX_HEADERS 100
#define MAX_HEADER_LENGTH 1024

char *getHeadersBuffer(char buffer[BUFFER_SIZE]);
char *getBodyBuffer(char buffer[BUFFER_SIZE]);

typedef struct {
    char key[MAX_HEADER_LENGTH];
    char value[MAX_HEADER_LENGTH];
} HttpHeader;

typedef struct {
    HttpHeader headers[MAX_HEADERS];
    int count;
} HttpHeaders;

void initHeaders(HttpHeaders *headers);
int addHeader(HttpHeaders *headers, const char *key, const char *value);
const char* getHeader(HttpHeaders *headers, const char *key);
void parseHeaders(HttpHeaders *headers, const char *buffer);
void printHeaders(HttpHeaders *headers);

void sendStatusCode200(int client_socket, const char *data);
void sendStatusCode400(int client_socket);
void sendStatusCode404(int client_socket);
void sendStatusCode405(int client_socket);
void sendStatusCode500(int client_socket);

void *handleHttpRequest(void *arg);

void sendFile(int client_socket, const char *file_path);

#endif // COMMON_H

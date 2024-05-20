#ifndef POST_H
#define POST_H

typedef struct {
    char *name;
    char *value;
} BodyField;

BodyField* parseHttpBodyFields(const char *str, int *count);

void handlePOST(int client_socket, const char *url, const char *body, const HttpHeaders *headers);

#endif // POST_H

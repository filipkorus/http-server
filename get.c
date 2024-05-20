#include "common.h"
#include "get.h"

void handleGET(int client_socket, const char *url) {
    char adjusted_url[BUFFER_SIZE];
    strncpy(adjusted_url, url, sizeof(adjusted_url) - 1);
    adjusted_url[sizeof(adjusted_url) - 1] = '\0'; // ensure null-termination

    // if URL ends with slash then add "index.html" at the end of it to serve this file
    if (adjusted_url[strlen(adjusted_url) - 1] == '/') {
        strncat(adjusted_url, "index.html", sizeof(adjusted_url) - strlen(adjusted_url) - 1);
    }

    char file_path[BUFFER_SIZE] = DOCUMENT_ROOT;
    strncat(file_path, adjusted_url, sizeof(file_path) - strlen(file_path) - 1);

    sendFile(client_socket, file_path);
}

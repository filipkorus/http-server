#include <dirent.h>
#include "common.h"

char *getHeadersBuffer(char buffer[BUFFER_SIZE]) {
    char *headers_buffer = strstr(buffer, "\r\n");
    char *end_of_headers = strstr(buffer, "\r\n\r\n");

    *end_of_headers = '\0';

    return headers_buffer;
}

char *getBodyBuffer(char buffer[BUFFER_SIZE]) {
    char *end_of_headers = strstr(buffer, "\r\n\r\n");

    return end_of_headers + 4; // Skip past the "\r\n\r\n"
}

void initHeaders(HttpHeaders *headers) {
    headers->count = 0;
}

int addHeader(HttpHeaders *headers, const char *key, const char *value) {
    if (headers->count >= MAX_HEADERS) {
        return -1;  // No more space for headers
    }
    strncpy(headers->headers[headers->count].key, key, MAX_HEADER_LENGTH - 1);
    strncpy(headers->headers[headers->count].value, value, MAX_HEADER_LENGTH - 1);
    headers->count++;
    return 0;
}

const char* getHeader(HttpHeaders *headers, const char *key) {
    for (int i = 0; i < headers->count; i++) {
        if (strcmp(headers->headers[i].key, key) == 0) {
            return headers->headers[i].value;
        }
    }
    return NULL;  // Header not found
}

void parseHeaders(HttpHeaders *headers, const char *buffer) {
    const char *line_start = buffer;
    const char *line_end;
    while (line_end = strstr(line_start, "\r\n")) {
        // Calculate line length
        size_t line_length = line_end - line_start;

        // Allocate memory for the line and copy it
        char *line = (char*)malloc(line_length + 1);
        if (line == NULL) {
            perror("malloc");
            exit(EXIT_FAILURE);
        }
        strncpy(line, line_start, line_length);
        line[line_length] = '\0';

        // Split the line into key and value
        char *colon = strstr(line, ": ");
        if (colon != NULL) {
            *colon = '\0';  // Null-terminate the key
            const char *key = line;
            const char *value = colon + 2;  // Skip ": "
            addHeader(headers, key, value);
        }

        // Free the temporary line buffer
        free(line);

        // Move to the next line
        line_start = line_end + 2;
    }
}

void printHeaders(HttpHeaders *headers) {
    for (int i = 0; i < headers->count; i++) {
        printf("%s: %s\n", headers->headers[i].key, headers->headers[i].value);
    }
}

void sendStatusCode200(int client_socket, const char *data) {
    const char *header = "HTTP/1.1 200 OK\r\n"
                           "Content-Type: text/html\r\n\r\n";

    char body[BUFFER_SIZE];

    if (data) {
        sprintf(
            body,
            "<html><body><h2>POST data received</h2>"
            "<ul>%s</ul>"
            "</body></html>",
            data
        );
    } else {
        sprintf(body, "<html><body><h2>POST data received</h2></body></html>");
    }

    send(client_socket, header, strlen(header), 0);
    send(client_socket, body, strlen(body), 0);
}

void sendStatusCode400(int client_socket) {
    const char *header = "HTTP/1.1 400 Bad Request\r\n"
                         "Content-Type: text/html\r\n\r\n";
    const char *body = "<html><body><h1>400 Bad Request</h1></body></html>";

    send(client_socket, header, strlen(header), 0);
    send(client_socket, body, strlen(body), 0);
}

void sendStatusCode404(int client_socket) {
    const char *header = "HTTP/1.1 404 Not Found\r\n"
                         "Content-Type: text/html\r\n\r\n";
    const char *body = "<html><body><h1>404 Not Found</h1></body></html>";

    send(client_socket, header, strlen(header), 0);
    send(client_socket, body, strlen(body), 0);
}

void sendStatusCode405(int client_socket) {
    const char *header = "HTTP/1.1 405 Method Not Allowed\r\n"
                         "Content-Type: text/html\r\n\r\n";
    const char *body = "<html><body><h1>405 Method Not Allowed</h1></body></html>";

    send(client_socket, header, strlen(header), 0);
    send(client_socket, body, strlen(body), 0);
}

void sendStatusCode500(int client_socket) {
    const char *header = "HTTP/1.1 500 Internal Server Error\r\n"
                         "Content-Type: text/html\r\n\r\n";
    const char *body = "<html><body><h1>500 Internal Server Error</h1></body></html>";

    send(client_socket, header, strlen(header), 0);
    send(client_socket, body, strlen(body), 0);
}

void *handleHttpRequest(void *arg) {
    int client_socket = *(int *)arg;
    free(arg);

    char buffer[BUFFER_SIZE] = {0};
    read(client_socket, buffer, sizeof(buffer) - 1);

    char method[BUFFER_SIZE], url[BUFFER_SIZE], protocol[BUFFER_SIZE];
    sscanf(buffer, "%s %s %s", method, url, protocol);

    char *body = getBodyBuffer(buffer);
    char *headers_buff = getHeadersBuffer(buffer);

    printf("%s %s %s\n", method, url, body);

    // headers structure
    HttpHeaders headers;
    initHeaders(&headers);
    parseHeaders(&headers, headers_buff);

    if (strcmp(method, "GET") == 0) {
        handleGET(client_socket, url);
    } else if (strcmp(method, "POST") == 0) {
        body ?
            handlePOST(
                client_socket,
                url,
                body,
                &headers
            ) :
            sendStatusCode400(client_socket);
    } else {
        sendStatusCode405(client_socket);
    }

    close(client_socket);
    return NULL;
}

void sendFile(int client_socket, const char *file_path) {
    char *dir_path = strdup(file_path); // Duplicate file_path to preserve it
    char *last_slash = strrchr(dir_path, '/');
    if (last_slash && strcmp(last_slash, "/index.html") == 0) {
        *last_slash = '\0'; // Remove index.html from the end
    }

    int file_fd = open(file_path, O_RDONLY);
    if (file_fd == -1) {
        // File doesn't exist, try to open it as a directory
        DIR *dir = opendir(dir_path);
        if (dir) {
            // It's a directory, list all files
            struct dirent *entry;
            char html_content[BUFFER_SIZE];
            snprintf(html_content, sizeof(html_content),
                     "HTTP/1.1 200 OK\r\n"
                     "Content-Type: text/html\r\n\r\n"
                     "<html><body><h1>Index of %s/</h1><ul>", dir_path + sizeof(DOCUMENT_ROOT) - 1);

            while ((entry = readdir(dir)) != NULL) {
                if (entry->d_type == DT_REG) { // Regular file
                    strcat(html_content, "<li><a href=\"");
                    strcat(html_content, entry->d_name);
                    strcat(html_content, "\">");
                    strcat(html_content, entry->d_name);
                    strcat(html_content, "</a></li>");
                }
            }
            strcat(html_content, "</ul></body></html>");

            send(client_socket, html_content, strlen(html_content), 0);
            closedir(dir);
        } else {
            // Failed to open file or directory
            // printf("\n%s does not exist\n", file_path);
            sendStatusCode404(client_socket);
        }
        free(dir_path); // Free the allocated memory for dir_path
        return;
    }

    // File exists, determine Content-Type based on file extension
    struct stat file_stat;
    fstat(file_fd, &file_stat);

    const char *content_type;
    const char *ext = strrchr(file_path, '.');
    if (ext && strcmp(ext, ".html") == 0) {
        content_type = "text/html";
    } else if (ext && strcmp(ext, ".css") == 0) {
        content_type = "text/css";
    } else if (ext && strcmp(ext, ".js") == 0) {
        content_type = "application/javascript";
    } else if (ext && strcmp(ext, ".jpg") == 0) {
        content_type = "image/jpeg";
    } else if (ext && strcmp(ext, ".png") == 0) {
        content_type = "image/png";
    } else if (ext && strcmp(ext, ".gif") == 0) {
        content_type = "image/gif";
    } else {
        content_type = "application/octet-stream"; // Default to binary data
    }

    char header[BUFFER_SIZE];
    snprintf(header, sizeof(header), "HTTP/1.1 200 OK\r\n"
                                     "Content-Length: %ld\r\n"
                                     "Content-Type: %s\r\n\r\n", file_stat.st_size, content_type);
    send(client_socket, header, strlen(header), 0);

    char buffer[BUFFER_SIZE];
    int bytes_read;
    while ((bytes_read = read(file_fd, buffer, sizeof(buffer))) > 0) {
        send(client_socket, buffer, bytes_read, 0);
    }
    close(file_fd);
    free(dir_path); // Free the allocated memory for dir_path
}

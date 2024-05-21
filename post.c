#include "common.h"
#include "post.h"

// Function to parse HTTP body fields
BodyField* parseHttpBodyFields(const char *str, int *count) {
    // Count the number of '&' characters to determine the number of fields
    int num_fields = 0;
    const char *ptr = str;
    while (*ptr) {
        if (*ptr == '&') {
            num_fields++;
        }
        ptr++;
    }
    num_fields++;  // Increment by 1 for the last field

    // Allocate memory for the HTTP body fields
    BodyField *fields = (BodyField*)malloc(num_fields * sizeof(BodyField));
    if (!fields) {
        perror("Memory allocation failed");
        exit(EXIT_FAILURE);
    }

    // Tokenize the string and extract HTTP body fields
    char *token, *saveptr;
    char *mutable_str = strdup(str);  // Make a mutable copy of the input string
    token = strtok_r(mutable_str, "&", &saveptr);
    *count = 0;  // Initialize count

    while (token != NULL) {
        // Split each token by '=' to extract name and value
        char *name = strtok(token, "=");
        char *value = strtok(NULL, "=");

        // Allocate memory for name and value and store in the fields array
        fields[*count].name = strdup(name);
        fields[*count].value = strdup(value);

        (*count)++;  // Increment count
        token = strtok_r(NULL, "&", &saveptr);
    }

    // Free the mutable string
    free(mutable_str);

    return fields;
}

// Function to free memory allocated for HTTP body fields
void freeHttpBodyFields(BodyField *fields, int count) {
    for (int i = 0; i < count; i++) {
        free(fields[i].name);
        free(fields[i].value);
    }
    free(fields);
}

void handlePOST(int client_socket, const char *url, const char *body, const HttpHeaders *headers) {
    // printHeaders(headers);

    const char *contentType = getHeader(headers, "Content-Type");

    if (strcmp(contentType, "application/json") == 0) {
        sendStatusCode200(client_socket, body); // send json body back
    } else if (strcmp(contentType, "application/x-www-form-urlencoded") == 0) {
        int count;
        BodyField *fields = parseHttpBodyFields(body, &count);

        char text[BUFFER_SIZE];

        for (int i = 0; i < count; i++) {
            char entry[BUFFER_SIZE] = {0};
            sprintf(entry, "<li>%s: %s</li>", fields[i].name, fields[i].value);
            strcat(text, entry);
        }

        sendStatusCode200(client_socket, text);

        // Free allocated memory
        freeHttpBodyFields(fields, count);
    } else {
        sendStatusCode400(client_socket);
    }
}

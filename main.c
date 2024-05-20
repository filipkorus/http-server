#include <stdio.h>
#include "common.h"
#include "get.h"
#include "post.h"

int main() {
    int server_fd, client_socket; // variables for server file descriptor and client socket
    struct sockaddr_in address; // structure for server address
    int addrlen = sizeof(address);

    // create a socket file descriptor for the server
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    int opt = 1;
    // set socket options to allow reuse of address and port
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt))) {
        perror("setsockopt failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    // set up server address structure
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    // bind the socket to the specified address and port
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("bind failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    // start listening for incoming connections with a queue size of 5
    if (listen(server_fd, 5) < 0) {
        perror("listen failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    printf("Server is listening on port %d\n", PORT);

    while (1) {
        // accept incoming client connection
        client_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen);
        
        if (client_socket < 0) {
            perror("accept failed");
            continue;
        }

        // allocate memory for client socket file descriptor
        int *pclient = malloc(sizeof(int));
        if (pclient == NULL) {
            perror("malloc failed");
            close(client_socket);
            continue;
        }
        *pclient = client_socket; // store the client socket file descriptor

        // create a new thread to handle the request
        pthread_t thread_id;
        if (pthread_create(&thread_id, NULL, handleHttpRequest, pclient) != 0) {
            perror("pthread_create failed");
            free(pclient);
            close(client_socket);
        } else {
            pthread_detach(thread_id);
        }
    }

    close(server_fd);
    return 0;
}

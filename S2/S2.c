#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>

#define PORT 5001
#define BUFFER_SIZE 1024
#define BASE_DIR "/home/patel77b/Project/S2"

// Function to create directory if it doesn't exist
void create_directory(const char *path) {
    struct stat st = {0};
    if (stat(path, &st) == -1) {
        mkdir(path, 0700);
    }
}

// Function to store a file received from S1
void store_file(int client_sock, char *filename, char *dest_path) {
    char full_path[BUFFER_SIZE];
    snprintf(full_path, sizeof(full_path), "%s/%s", BASE_DIR, dest_path);
    create_directory(full_path); // Ensure directory exists

    strcat(full_path, "/");
    strcat(full_path, filename);

    printf("S2: Storing file %s at %s\n", filename, full_path);

    int fd = open(full_path, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (fd < 0) {
        perror("S2: File open failed");
        send(client_sock, "ERROR", 5, 0);
        return;
    }

    char buffer[BUFFER_SIZE];
    int bytes;
    while ((bytes = recv(client_sock, buffer, sizeof(buffer), 0)) > 0) {
        write(fd, buffer, bytes);
    }
    close(fd);

    printf("S2: File %s stored successfully\n", filename);
    send(client_sock, "ACK", 3, 0);
}

int main() {
    int server_sock, client_sock;
    struct sockaddr_in server_addr, client_addr;
    socklen_t addr_size = sizeof(client_addr);

    // Create socket
    server_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (server_sock < 0) {
        perror("S2: Socket creation failed");
        exit(1);
    }

    // Configure server address
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    server_addr.sin_addr.s_addr = INADDR_ANY;

    // Bind
    if (bind(server_sock, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("S2: Bind failed");
        close(server_sock);
        exit(1);
    }

    // Listen
    if (listen(server_sock, 5) < 0) {
        perror("S2: Listen failed");
        close(server_sock);
        exit(1);
    }

    printf("S2 server listening on port %d...\n", PORT);

    while (1) {
        client_sock = accept(server_sock, (struct sockaddr*)&client_addr, &addr_size);
        if (client_sock < 0) {
            perror("S2: Accept failed");
            continue;
        }

        printf("S2: New connection accepted\n");

        char buffer[BUFFER_SIZE];
        int bytes_received = recv(client_sock, buffer, sizeof(buffer), 0);
        if (bytes_received <= 0) {
            printf("S2: No data received or connection closed\n");
            close(client_sock);
            continue;
        }
        buffer[bytes_received] = '\0';

        // Parse command (e.g., "uploadf:sample.pdf:S2/folder1")
        char *command = strtok(buffer, ":");
        char *arg1 = strtok(NULL, ":"); // filename
        char *arg2 = strtok(NULL, ":"); // destination_path

        printf("S2: Received command: %s, filename: %s, path: %s\n", command, arg1, arg2);

        if (strcmp(command, "uploadf") == 0) {
            store_file(client_sock, arg1, arg2);
        } else {
            printf("S2: Unknown command: %s\n", command);
            send(client_sock, "ERROR", 5, 0);
        }

        close(client_sock);
    }

    close(server_sock);
    return 0;
}

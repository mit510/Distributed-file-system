#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <arpa/inet.h> // Added for inet_addr

#define PORT 5000
#define S2_PORT 5001
#define S3_PORT 5002
#define S4_PORT 5003
#define BUFFER_SIZE 1024
#define BASE_DIR "/home/patel77b/Project/S1" // Replace with your actual path

// Function to create directory recursively if it doesn't exist
void create_directory_recursive(const char *path) {
    char dir_path[BUFFER_SIZE] = {0};
    strncpy(dir_path, path, BUFFER_SIZE - 1);

    char *token = strtok(dir_path, "/");
    char current_path[BUFFER_SIZE] = "";
    struct stat st = {0};

    while (token != NULL) {
        strcat(current_path, "/");
        strcat(current_path, token);
        if (stat(current_path, &st) == -1) {
            mkdir(current_path, 0700);
        }
        token = strtok(NULL, "/");
    }
}

// Function to connect to another server (e.g., S2, S3, S4)
int connect_to_server(const char *ip, int port) {
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        perror("S1: Socket creation failed");
        return -1;
    }

    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    server_addr.sin_addr.s_addr = inet_addr(ip);

    if (connect(sock, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("S1: Connection to server failed");
        close(sock);
        return -1;
    }
    printf("S1: Connected to server on port %d\n", port);
    return sock;
}

// Function to send a file to another server
void send_file(int sock, char *filename, char *dest_path) {
    char buffer[BUFFER_SIZE];
    snprintf(buffer, sizeof(buffer), "uploadf:%s:%s", filename, dest_path);
    printf("S1: Sending command to server: %s\n", buffer);
    send(sock, buffer, strlen(buffer), 0);

    // Receive file content from client and forward it
    char file_buffer[BUFFER_SIZE];
    int bytes;
    while ((bytes = recv(sock, file_buffer, sizeof(file_buffer), 0)) > 0) {
        if (strcmp(file_buffer, "EOF") == 0) {
            break; // End of file marker
        }
        send(sock, file_buffer, bytes, 0);
    }
    close(sock);

    // Wait for ACK
    recv(sock, buffer, sizeof(buffer), 0);
    printf("S1: Received ACK from server: %s\n", buffer);
}

// Function to store a file locally
void store_file_locally(int client_sock, char *filename, char *dest_path) {
    char full_path[BUFFER_SIZE];
    snprintf(full_path, sizeof(full_path), "%s/%s", BASE_DIR, dest_path);
    create_directory_recursive(dest_path); // Create directories recursively

    strcat(full_path, "/");
    strcat(full_path, filename);

    printf("S1: Storing file %s at %s\n", filename, full_path);

    int fd = open(full_path, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (fd < 0) {
        perror("S1: File open failed");
        send(client_sock, "ERROR", 5, 0);
        return;
    }

    char buffer[BUFFER_SIZE];
    int bytes;
    while ((bytes = recv(client_sock, buffer, sizeof(buffer), 0)) > 0) {
        if (strcmp(buffer, "EOF") == 0) {
            break; // End of file marker
        }
        write(fd, buffer, bytes);
    }
    close(fd);

    printf("S1: File %s stored successfully\n", filename);
    send(client_sock, "ACK", 3, 0);
}

void prcclient(int client_sock) {
    char buffer[BUFFER_SIZE];
    while (1) {
        int bytes_received = recv(client_sock, buffer, sizeof(buffer), 0);
        if (bytes_received <= 0) {
            printf("S1: Client disconnected\n");
            close(client_sock);
            return;
        }
        buffer[bytes_received] = '\0';

        printf("S1: Received raw command: %s\n", buffer);

        // Parse the command using strtok with a colon delimiter
        char *command = strtok(buffer, ":");
        char *arg1 = strtok(NULL, ":"); // filename
        char *arg2 = strtok(NULL, ":"); // destination_path

        if (!command || !arg1 || !arg2) {
            printf("S1: Invalid command format\n");
            send(client_sock, "ERROR", 5, 0);
            continue;
        }

        printf("S1: Parsed command: %s, filename: %s, path: %s\n", command, arg1, arg2);

        if (strcmp(command, "uploadf") == 0) {
            char *ext = strrchr(arg1, '.');
            if (!ext) {
                printf("S1: No extension in filename %s\n", arg1);
                send(client_sock, "ERROR", 5, 0);
                continue;
            }

            if (strcmp(ext, ".c") == 0) {
                store_file_locally(client_sock, arg1, arg2);
            } else if (strcmp(ext, ".pdf") == 0) {
                int s2_sock = connect_to_server("127.0.0.1", S2_PORT);
                if (s2_sock >= 0) {
                    printf("S1: Forwarding .pdf to S2\n");
                    send_file(s2_sock, arg1, arg2);
                    close(s2_sock);
                    send(client_sock, "ACK", 3, 0);
                } else {
                    send(client_sock, "ERROR", 5, 0);
                }
            } else if (strcmp(ext, ".txt") == 0) {
                int s3_sock = connect_to_server("127.0.0.1", S3_PORT);
                if (s3_sock >= 0) {
                    printf("S1: Forwarding .txt to S3\n");
                    send_file(s3_sock, arg1, arg2);
                    close(s3_sock);
                    send(client_sock, "ACK", 3, 0);
                } else {
                    send(client_sock, "ERROR", 5, 0);
                }
            } else if (strcmp(ext, ".zip") == 0) {
                int s4_sock = connect_to_server("127.0.0.1", S4_PORT);
                if (s4_sock >= 0) {
                    printf("S1: Forwarding .zip to S4\n");
                    send_file(s4_sock, arg1, arg2);
                    close(s4_sock);
                    send(client_sock, "ACK", 3, 0);
                } else {
                    send(client_sock, "ERROR", 5, 0);
                }
            }
        }
        // Add handlers for other commands (downlf, removef, downltar, dispfnames) here
    }
}

int main() {
    int server_sock, client_sock;
    struct sockaddr_in server_addr, client_addr;
    socklen_t addr_size = sizeof(client_addr);

    server_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (server_sock < 0) {
        perror("S1: Socket creation failed");
        exit(1);
    }

    // Set SO_REUSEADDR to allow reusing the port immediately
    int opt = 1;
    if (setsockopt(server_sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        perror("S1: Setsockopt failed");
        close(server_sock);
        exit(1);
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    server_addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(server_sock, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("S1: Bind failed");
        exit(1);
    }
    if (listen(server_sock, 5) < 0) {
        perror("S1: Listen failed");
        exit(1);
    }

    printf("S1 server listening on port %d...\n", PORT);

    while (1) {
        client_sock = accept(server_sock, (struct sockaddr*)&client_addr, &addr_size);
        if (fork() == 0) {
            close(server_sock);
            prcclient(client_sock);
            close(client_sock);
            exit(0);
        }
        close(client_sock);
    }

    close(server_sock);
    return 0;
}

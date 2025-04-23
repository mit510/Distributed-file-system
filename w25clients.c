#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <fcntl.h>
#include <arpa/inet.h> // Added for inet_addr

#define SERVER_PORT 5000
#define BUFFER_SIZE 1024

int main() {
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        perror("Socket creation failed");
        exit(1);
    }

    struct sockaddr_in server_addr = {AF_INET, htons(SERVER_PORT), inet_addr("127.0.0.1")};
    if (connect(sock, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("Connection failed");
        close(sock);
        exit(1);
    }
    printf("Connected to S1 on port %d\n", SERVER_PORT);

    char command[BUFFER_SIZE];
    while (1) {
        printf("w25clients$ ");
        fgets(command, sizeof(command), stdin);
        command[strcspn(command, "\n")] = 0;

        printf("Sending command: %s\n", command);

        // Handle uploadf command
        if (strncmp(command, "uploadf", 7) == 0) {
            char *filename = strtok(command + 8, " ");
            char *dest_path = strtok(NULL, " ");
            if (!filename || !dest_path) {
                printf("Invalid uploadf command format\n");
                send(sock, "ERROR", 5, 0);
                continue;
            }

            int fd = open(filename, O_RDONLY);
            if (fd < 0) {
                perror("File open failed");
                send(sock, "ERROR", 5, 0);
                continue;
            }

            // Send the full command format expected by S1 (uploadf:filename:dest_path)
            char full_command[BUFFER_SIZE];
            snprintf(full_command, sizeof(full_command), "uploadf:%s:%s", filename, dest_path);
            send(sock, full_command, strlen(full_command), 0);

            printf("Sending file %s\n", filename);
            char buffer[BUFFER_SIZE];
            int bytes;
            while ((bytes = read(fd, buffer, sizeof(buffer))) > 0) {
                send(sock, buffer, bytes, 0);
            }
            // Send an EOF marker to signal end of file
            send(sock, "EOF", 3, 0);
            close(fd);
        } else {
            // Send other commands as-is (you can extend this for downlf, etc.)
            send(sock, command, strlen(command), 0);
        }

        // Receive and print server response
        char response[BUFFER_SIZE];
        int bytes_received = recv(sock, response, sizeof(response) - 1, 0);
        if (bytes_received > 0) {
            response[bytes_received] = '\0';
            printf("Server: %s\n", response);
        } else if (bytes_received == 0) {
            printf("Server closed the connection\n");
            break;
        } else {
            perror("Receive failed");
            break;
        }
    }

    close(sock);
    return 0;
}

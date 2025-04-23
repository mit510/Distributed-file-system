#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <dirent.h>

#define PORT 5002
#define BUFFER_SIZE 1024
#define BASE_DIR "/home/patel77b/Project/S3"

// Function to create directory if it doesn't exist
void create_directory(const char *path) {
    struct stat st = {0};
    if (stat(path, &st) == -1) {
        mkdir(path, 0700);
    }
}

// Function to handle file storage
void store_file(int client_sock, char *filename, char *dest_path) {
    char full_path[BUFFER_SIZE];
    snprintf(full_path, sizeof(full_path), "%s/%s", BASE_DIR, dest_path);
    create_directory(full_path);

    strcat(full_path, "/");
    strcat(full_path, filename);

    int fd = open(full_path, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    char buffer[BUFFER_SIZE];
    int bytes;
    while ((bytes = recv(client_sock, buffer, sizeof(buffer), 0)) > 0) {
        write(fd, buffer, bytes);
    }
    close(fd);
    send(client_sock, "ACK", 3, 0);
}

int main() {
    int server_sock, client_sock;
    struct sockaddr_in server_addr, client_addr;
    socklen_t addr_size = sizeof(client_addr);

    // Create socket
    server_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (server_sock < 0) {
        perror("Socket creation failed");
        exit(1);
    }

    // Configure server address
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    server_addr.sin_addr.s_addr = INADDR_ANY;

    // Bind and listen
    if (bind(server_sock, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("Bind failed");
        exit(1);
    }
    if (listen(server_sock, 5) < 0) {
        perror("Listen failed");
        exit(1);
    }

    printf("S3 server listening on port %d...\n", PORT);

    while (1) {
        client_sock = accept(server_sock, (struct sockaddr*)&client_addr, &addr_size);
        if (client_sock < 0) {
            perror("Accept failed");
            continue;
        }

        char buffer[BUFFER_SIZE];
        recv(client_sock, buffer, sizeof(buffer), 0);
        char *command = strtok(buffer, ":");
        char *arg1 = strtok(NULL, ":");
        char *arg2 = strtok(NULL, ":");

        if (strcmp(command, "uploadf") == 0) {
            store_file(client_sock, arg1, arg2);
        } else if (strcmp(command, "downlf") == 0) {
            char full_path[BUFFER_SIZE];
            snprintf(full_path, sizeof(full_path), "%s/%s", BASE_DIR, arg1);
            int fd = open(full_path, O_RDONLY);
            if (fd >= 0) {
                char file_buffer[BUFFER_SIZE];
                int bytes;
                while ((bytes = read(fd, file_buffer, sizeof(file_buffer))) > 0) {
                    send(client_sock, file_buffer, bytes, 0);
                }
                close(fd);
            }
            send(client_sock, "ACK", 3, 0);
        } else if (strcmp(command, "removef") == 0) {
            char full_path[BUFFER_SIZE];
            snprintf(full_path, sizeof(full_path), "%s/%s", BASE_DIR, arg1);
            remove(full_path);
            send(client_sock, "ACK", 3, 0);
        } else if (strcmp(command, "downltar") == 0) {
            // Create a tar file of all .txt files (requires tar command or library)
            system("tar -cvf text.tar ~/S3/*.txt");
            int fd = open("text.tar", O_RDONLY);
            char file_buffer[BUFFER_SIZE];
            int bytes;
            while ((bytes = read(fd, file_buffer, sizeof(file_buffer))) > 0) {
                send(client_sock, file_buffer, bytes, 0);
            }
            close(fd);
            remove("text.tar");
            send(client_sock, "ACK", 3, 0);
        } else if (strcmp(command, "dispfnames") == 0) {
            DIR *dir;
            struct dirent *entry;
            char full_path[BUFFER_SIZE];
            snprintf(full_path, sizeof(full_path), "%s/%s", BASE_DIR, arg1);
            dir = opendir(full_path);
            char file_list[BUFFER_SIZE] = "";
            if (dir) {
                while ((entry = readdir(dir)) != NULL) {
                    if (strstr(entry->d_name, ".txt")) {
                        strcat(file_list, entry->d_name);
                        strcat(file_list, "\n");
                    }
                }
                closedir(dir);
            }
            send(client_sock, file_list, strlen(file_list), 0);
            send(client_sock, "ACK", 3, 0);
        }

        close(client_sock);
    }

    close(server_sock);
    return 0;
}

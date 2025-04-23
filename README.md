**Distributed File System**

A socket-based distributed file system implemented in C, designed to handle multi-client file uploads across four servers. Server 1 (S1) stores .c files locally and forwards .pdf, .txt, and .zip files to Servers 2–4 (S2–S4) using socket communication, ensuring efficient file distribution and robust error handling.

**1.	Features**

Multi-Client Support: Handles concurrent client connections for seamless file uploads.

File Distribution: S1 manages .c files locally and delegates .pdf, .txt, and .zip files to S2–S4 based on file type.

Socket Communication: Utilizes TCP sockets for reliable client-server and server-server interactions.

Error Handling: Implements checks for file type validation, connection failures, and transfer errors.

Unit Testing: Includes test cases to verify file transfer reliability and server functionality.

**2.	Tech Stack**

Language: C

Networking: Socket Programming (TCP)

Tools: Git, GCC (for compilation)

**3.	Prerequisites**

GCC compiler (e.g., gcc on Linux or MinGW on Windows)

Standard C libraries (sys/socket.h, netinet/in.h, arpa/inet.h)

Git for cloning the repository

Linux or Unix-like environment recommended (e.g., Ubuntu, WSL)

**4.	Setup and Installation**

Clone the repository:git clone https://github.com/yourusername/distributed-file-system.git

cd distributed-file-system

Compile the server and client programs:gcc S1.c -o S1

gcc S2.c -o S2

gcc S3.c -o S3

gcc S4.c -o S4

gcc w25clients.c -o w25clients

Create storage directories for each server: mkdir ~/S1 ~/S2 ~/S3 ~/S4 ~/S1_temp

Run the servers in separate terminal windows:./S1

./S2

./S3

./S4

Run the client shell:./w25clients

**5.	Usage**

Start all four servers (S1–S4) before running the client.

In the w25clients shell, use the uploadf command to upload files:uploadf sample.c ~/S1

uploadf document.pdf ~/S1

S1 processes the file:

Stores .c files in ~/S1.

Forwards .pdf to S2 (~/S2),

          .txt to S3 (~/S3),  
          
           .zip to S4 (~/S4).

Check server logs for transfer status or errors.

Temporary files are stored in ~/S1_temp during processing.

**6.	Project Structure**

distributed-file-system/

├── S1.c            # Main server handling .c files and forwarding

├── S2.c            # Secondary server for .pdf files

├── S3.c            # Secondary server for .txt files

├── S4.c            # Secondary server for .zip files

├── w25clients.c    # Client shell with uploadf command

├── tests/          # Unit test cases

│   └── test_file_transfer.c

├── LICENSE         # MIT License

└── README.md       # Project documentation

**7.	Testing**

Run unit tests to verify file transfer and error handling:

gcc tests/test_file_transfer.c -o test_file_transfer

./test_file_transfer

**8.	Tests cover:**

Valid and invalid file types

Multi-client file uploads

Server connectivity and forwarding

Error scenarios (e.g., missing servers, invalid paths)

**Contact**

Mitkumar Patel – patel77b@uwindsor.ca 

**Acknowledgments**

Developed as part of a coursework assignment at the University of Windsor, Winter 2025

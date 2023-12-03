#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <iostream>
#include <fstream>

using namespace std;
int const BUFFERSIZE = 100;
void DieWithUserMessage(const char *msg, const char *detail) {
    fputs(msg, stderr);
    fputs(": ", stderr);
    fputs(detail, stderr);
    fputc('\n', stderr);
    exit(1);
}
void DieWithSystemMessage(const char *msg) {
    perror(msg);
    exit(1);
}
int main(int argc, char *argv[]) {
    if (argc < 3 || argc > 4) // Test for correct number of arguments
        DieWithUserMessage("Parameter(s)",
                           "<Server Address> <filepath> [<Server Port>]");
//    127.0.0.1 home/text.txt 8008
    char *servIP = argv[1];
// First arg: server IP address (dotted quad)
//should be filepath
    char *filepath = "/home/madyelzainy/requests.txt"; // Second arg: string to echo
// Third arg (optional): server port (numeric). 7 is well-known echo port
    in_port_t servPort = (argc == 4) ? atoi(argv[3]) : 7;
// Create a reliable, stream socket using TCP
    int sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sock < 0)
        DieWithSystemMessage("creating socket failed");
// Construct the server address structure
    struct sockaddr_in servAddr;
// Server address
    memset(&servAddr, 0, sizeof(servAddr)); // Zero out structure
    servAddr.sin_family = AF_INET;
// IPv4 address family
// Convert address
    int rtnVal = inet_pton(AF_INET, servIP, &servAddr.sin_addr.s_addr);
    if (rtnVal == 0)
        DieWithUserMessage("converting IPv4 to binary address failed", "invalid address string");
    else if (rtnVal < 0)
        DieWithSystemMessage("converting IPv4 to binary address failed");
    servAddr.sin_port = htons(servPort);
// Server port
//read request from file
// Establish the connection to the echo server
    if (connect(sock, (struct sockaddr *) &servAddr, sizeof(servAddr)) < 0)
        DieWithSystemMessage("connection failed");
//    requests should be read from file
    string request="get /home/madyelzainy/CLionProjects/untitled3/Server/images.jpeg 8008";
//    string request="get /home/madyelzainy/CLionProjects/untitled3/Server/images.jpeg 127.0.0.1 (8008)";
//    string request="post /home/madyelzainy/CLionProjects/untitled3/Server/images.jpeg 127.0.0.1 (8008) ";
//   ana 3yzha kda
//    string request="post Server/images.jpeg 127.0.0.1 (8008) file as a string ana 3mlto ";

    size_t request_size = strlen("./c 127.0.0.1 /home/madyelzainy/CLionProjects/untitled3/Server/images.jpeg 8008");
    ssize_t numBytes = send(sock, request.c_str(), request.size(), 0);
    if (numBytes < 0)
        DieWithSystemMessage("sending file failed");
    else if (numBytes != request.size())
        DieWithUserMessage("sending file failed", "sent unexpected number of bytes");

    std::ofstream outputFile("received_file.txt", std::ios::binary); // Open the file for binary output

    char buffer[BUFFERSIZE];
    ssize_t totalBytesRcvd = 0;

    while (true) {
        ssize_t numBytesReceived = recv(sock, buffer, BUFFERSIZE, 0);
        if (numBytesReceived < 0)
            DieWithSystemMessage("receiving file failed");
        else if (numBytesReceived == 0)
            break; // Connection closed by the server

        totalBytesRcvd += numBytesReceived;
        outputFile.write(buffer, numBytesReceived); // Write received data to the file
    }

    outputFile.close();

    if (totalBytesRcvd == 0)
        DieWithUserMessage("receiving file", "connection closed prematurely");

    std::cout << "Received file successfully and saved as 'received_file.txt'" << std::endl;

    close(sock);
}
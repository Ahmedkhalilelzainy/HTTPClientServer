#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <iostream>

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
    char *echoString = argv[2]; // Second arg: string to echo
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
    size_t echoStringLen = strlen(echoString); // Determine input length
// Send the string to the server
//  if request is get send the request as a string and recieve response and a string(file) and save it using filestream
// if request is post take as file stream send in request body  and recieve response

    ssize_t numBytes = send(sock, echoString, echoStringLen, 0);
    if (numBytes < 0)
        DieWithSystemMessage("sending file failed");
    else if (numBytes != echoStringLen)
        DieWithUserMessage("sending file failed", "sent unexpected number of bytes");
// Receive the same string back from the server
    unsigned int totalBytesRcvd = 0; // Count of total bytes received
    fputs("Received: ", stdout);
// Setup to print the echoed string
// echoStringLen should be request
    while (totalBytesRcvd < echoStringLen) {
        char buffer[BUFFERSIZE]; // I/O buffer
/* Receive up to the buffer size (minus 1 to leave space for
a null terminator) bytes from the sender */
    // request send
        numBytes = recv(sock, buffer, BUFFERSIZE - 1, 0);
        if (numBytes < 0)
            DieWithSystemMessage("receiving file failed");
        else if (numBytes == 0)
            DieWithUserMessage("receiving file", "connection closed prematurely");
        totalBytesRcvd += numBytes; // Keep tally of total bytes
        buffer[numBytes] = '\0';
// Terminate the string!
        fputs(buffer, stdout);
// Print the echo buffer
    }
//
    fputc('\n', stdout); // Print a final linefeed
    exit(0);
}
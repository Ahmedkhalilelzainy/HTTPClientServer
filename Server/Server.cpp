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
//http get c.png 7070
//http get c.png 7070
static const int MAXPENDING = 5; // Maximum outstanding connection requests
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

//void HandleTCPClient(int clntSocket) {
//    char buffer[BUFFERSIZE]; // Buffer for echo string
//// Receive message from client
//    ssize_t numBytesRcvd = recv(clntSocket, buffer, BUFFERSIZE, 0);
//    if (numBytesRcvd < 0)
//        DieWithSystemMessage("recv() failed");// Send received string and receive again until end of stream
//    while (numBytesRcvd > 0) { // 0 indicates end of stream
//// Echo message back to client
//        ssize_t numBytesSent = send(clntSocket, buffer, numBytesRcvd, 0);
//        if (numBytesSent < 0)
//            DieWithSystemMessage("send() failed");
//        else if (numBytesSent != numBytesRcvd)
//            DieWithUserMessage("send()", "sent unexpected number of bytes");
//// See if there is more data to receive
//        numBytesRcvd = recv(clntSocket, buffer, BUFFERSIZE, 0);
//        if (numBytesRcvd < 0)
//            DieWithSystemMessage("recv() failed");
//    }
//    close(clntSocket); // Close client socket
//}


void processBuffer(const char *buffer, size_t bytesRead) {
    // Add your processing logic here
    // This function is called for each chunk of data read

    // Check if there is any data in the buffer before printing
    cout<<buffer<<"\n";
    if (bytesRead > 0) {
        printf("Processed buffer: %.*s\n", (int)bytesRead, buffer);
    }
}

void *HandleTCPClient(void *arg) {
    int clntSock = *((int *) arg);
    free(arg); // Free memory allocated for the argument

    char buffer[BUFFERSIZE]; // BUFFERSIZE is assumed to be defined somewhere
    ssize_t numBytesRcvd;

    // Receive and send data in a loop until the client closes the connection
    while ((numBytesRcvd = recv(clntSock, buffer, BUFFERSIZE - 1, 0)) > 0) {
        buffer[numBytesRcvd] = '\0'; // Null-terminate the received data
        printf("Received: %s", buffer);

        // Process the received data or perform other actions as needed

        // Echo the data back to the client
        ssize_t numBytesSent = send(clntSock, buffer, numBytesRcvd, 0);
        if (numBytesSent < 0)
            DieWithSystemMessage("send() failed");
        else if (numBytesSent != numBytesRcvd)
            DieWithUserMessage("send()", "sent unexpected number of bytes");
    }

    if (numBytesRcvd == 0) {
        // Client closed the connection
        printf("Client closed the connection.\n");
    } else if (numBytesRcvd < 0) {
        // Error occurred while receiving
        DieWithSystemMessage("recv() failed");
    }

    close(clntSock); // Close the client socket
    pthread_exit(NULL); // Exit the thread
}

//    std::string sourceFolderPath = "/home/madyelzainy/CLionProjects/untitled3/Server/";
//    std::string destinationFolderPath = "/home/madyelzainy/CLionProjects/untitled3/Client/";
//    std::string fileName = "test1.txt";
//
//    // Construct the full paths to the source and destination files
//    std::string sourceFilePath = sourceFolderPath + fileName;
//    std::string destinationFilePath = destinationFolderPath + fileName;
//
//    // Open the source file for reading
//    std::ifstream sourceFile(sourceFilePath);
//
//    // Check if the source file is open
//    if (!sourceFile.is_open()) {
//        std::cerr << "Error opening source file: " << sourceFilePath << std::endl;
//        return reinterpret_cast<void *>(1);
//    }
//
//    // Open the destination file for writing
//    std::ofstream destinationFile(destinationFilePath);
//
//    // Check if the destination file is open
//    if (!destinationFile.is_open()) {
//        std::cerr << "Error opening destination file: " << destinationFilePath << std::endl;
//        sourceFile.close();  // Close the source file before exiting
//        return reinterpret_cast<void *>(1);
//    }
//
//    // Read and write the contents of the file line by line
//    std::string line;
//    while (std::getline(sourceFile, line)) {
//        // Write each line to the destination file
//        destinationFile << line << std::endl;
//    }
//
//    // Close the files
//    sourceFile.close();
//    destinationFile.close();
//
//    std::cout << "File copied successfully from " << sourceFilePath << " to " << destinationFilePath << std::endl;
//    return  0;
//}

int main(int argc, char *argv[]) {
    if (argc != 2) // Test for correct number of arguments
        DieWithUserMessage("Parameter(s)", "<Server Port>");

    in_port_t servPort = atoi(argv[1]); // First arg: local port

    // Create socket for incoming connections
    int servSock; // Socket descriptor for server
    if ((servSock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
        DieWithSystemMessage("socket() failed");

    // Set socket option to allow reuse of local addresses
    int reuseAddr = 1;
    if (setsockopt(servSock, SOL_SOCKET, SO_REUSEADDR, &reuseAddr, sizeof(reuseAddr)) < 0)
        DieWithSystemMessage("setting socket options failed");

    // Construct local address structure
    struct sockaddr_in servAddr;
    memset(&servAddr, 0, sizeof(servAddr));
    servAddr.sin_family = AF_INET;
    servAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servAddr.sin_port = htons(servPort);

    // Bind to the local address
    if (bind(servSock, (struct sockaddr *) &servAddr, sizeof(servAddr)) < 0)
        DieWithSystemMessage("binding socket to port number failed");

    // Mark the socket so it will listen for incoming connections
    if (listen(servSock, MAXPENDING) < 0)
        DieWithSystemMessage("listening  failed");

    for (;;) { // Run forever
        struct sockaddr_in clientAddr; // Client address
        socklen_t clientAddrLen = sizeof(clientAddr);

        // Wait for a client to connect
        int *clientSock = (int *)malloc(sizeof(int)); // Allocate memory for the socket descriptor
        *clientSock = accept(servSock, (struct sockaddr *) &clientAddr, &clientAddrLen);
        if (*clientSock < 0)
            DieWithSystemMessage("accepting connection failed");

        // Create a new thread to handle the client
        pthread_t threadID;
        int returnValue = pthread_create(&threadID, NULL, HandleTCPClient, (void *)clientSock);
        if (returnValue != 0)
            DieWithSystemMessage("creating thread failed failed");

        // Detach the thread to automatically reclaim resources when it finishes
        pthread_detach(threadID);
    }

    // The main thread will never reach here, but you can use pthread_join to wait for the threads to finish if needed.

    return 0;
}

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
#include <sstream>

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





//void *HandleTCPClient(void *arg) {
////    int clntSock = *((int *) arg);
////    free(arg); // Free memory allocated for the argument
////
////    char buffer[BUFFERSIZE]; // BUFFERSIZE is assumed to be defined somewhere
////    ssize_t numBytesRcvd;
////
////    // Receive and send data in a loop until the client closes the connection
////    while ((numBytesRcvd = recv(clntSock, buffer, BUFFERSIZE - 1, 0)) > 0) {
////        buffer[numBytesRcvd] = '\0'; // Null-terminate the received data
////        printf("Received: %s", buffer);
////
////        // Process the received data or perform other actions as needed
////
////        // Echo the data back to the client
////        ssize_t numBytesSent = send(clntSock, buffer, numBytesRcvd, 0);
////        if (numBytesSent < 0)
////            DieWithSystemMessage("send() failed");
////        else if (numBytesSent != numBytesRcvd)
////            DieWithUserMessage("send()", "sent unexpected number of bytes");
////    }
////
////    if (numBytesRcvd == 0) {
////        // Client closed the connection
////        printf("Client closed the connection.\n");
////    } else if (numBytesRcvd < 0) {
////        // Error occurred while receiving
////        DieWithSystemMessage("recv() failed");
////    }
////
////    close(clntSock); // Close the client socket
////    pthread_exit(NULL); // Exit the thread
////}
//
//    std::string sourceFolderPath = "/home/madyelzainy/CLionProjects/untitled3/Server/";
//    std::string fileName = "images.jpeg";
//
//    // Construct the full path to the source file
//    std::string sourceFilePath = sourceFolderPath + fileName;
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
//    // Read the entire content of the file into a string
//    std::ostringstream fileContentStream;
//    fileContentStream << sourceFile.rdbuf(); // Read the entire file into the stream
//
//    // Close the source file
//    sourceFile.close();
//
//    // Get the content as a string
//    std::string fileContent = fileContentStream.str();
//    cout<<fileContent;
//    cout.flush();
//    // Now 'fileContent' contains the content of the file as a string
//
//    // Specify the destination file path
//    std::string destinationFolderPath = "/home/madyelzainy/CLionProjects/untitled3/Client/";
//    std::string destinationFileName = "images.jpeg";
//    std::string destinationFilePath = destinationFolderPath + destinationFileName;
//
//    // Open the destination file for writing
//    std::ofstream destinationFile(destinationFilePath, std::ios::binary);
//
//    // Check if the destination file is open
//    if (!destinationFile.is_open()) {
//        std::cerr << "Error opening destination file: " << destinationFilePath << std::endl;
//        return reinterpret_cast<void *>(1);
//    }
//
//    // Write the content to the destination file
//    destinationFile << fileContent;
//
//    // Close the destination file
//    destinationFile.close();
//
//    std::cout << "File content copied from " << sourceFilePath << " to " << destinationFilePath << std::endl;
//
//    return 0;
//}
#define BUFFERSIZE 1024

void ParseHttpGet(const std::string& httpRequest, std::string& filePath) {
    // Assuming a simple GET request format like "GET /path/to/file HTTP/1.1"
    std::istringstream ss(httpRequest);
    std::string method, path, version;
    ss >> method >> path ;
    cout<<method<<" "<<path<<"\n";

    if (method == "GET") {
        // Extract the file path from the request
        filePath = path.substr(1); // Remove leading '/'
    }
}

void ParseHttpPost(const std::string& httpRequest, std::string& filePath, std::string& fileContents) {
    // Assuming a simple POST request format like "POST /path/to/file HTTP/1.1\nContent-Length: 12\n\nHello, world!"
    std::istringstream ss(httpRequest);
    std::string method, path, version;
    ss >> method >> path ;
    if (method == "POST") {
        // Extract the file path from the request
        filePath = path.substr(1); // Remove leading '/'

        // Find the position of the double newline indicating the start of the content
        size_t contentStart = httpRequest.find("\n\n");

        if (contentStart != std::string::npos) {
            // Extract the content from the request
            fileContents = httpRequest.substr(contentStart + 2);
        }
    }
}

void *HandleTCPClient(void *arg) {
    int clientSock = *((int *)arg);
    free(arg);

    char buffer[BUFFERSIZE];

    // Receive and send data in a loop until the client closes the connection
    while (true) {
        ssize_t numBytesRcvd = recv(clientSock, buffer, BUFFERSIZE - 1, 0);

        if (numBytesRcvd > 0) {
            buffer[numBytesRcvd] = '\0'; // Null-terminate the received data

            std::string httpRequest(buffer);

            std::string method, path, version, filePath, fileContents;

            // Check if it's a GET or POST request
            if (httpRequest.find("GET") == 0) {
                ParseHttpGet(httpRequest, filePath);
                std::cout << "Received GET request for file: " << filePath << std::endl;
            } else if (httpRequest.find("POST") == 0) {
                ParseHttpPost(httpRequest, filePath, fileContents);
                std::cout << "Received POST request for file: " << filePath << std::endl;
                std::cout << "File contents: " << fileContents << std::endl;
            } else {
                std::cerr << "Unsupported HTTP method" << std::endl;
            }
        } else if (numBytesRcvd == 0) {
            std::cerr << "Client closed the connection." << std::endl;
            break;  // Exit the loop if the client closes the connection
        } else {
            std::cerr << "Error receiving data from client." << std::endl;
            break;  // Exit the loop if an error occurs
        }
    }

    close(clientSock);
    pthread_exit(NULL);
}



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

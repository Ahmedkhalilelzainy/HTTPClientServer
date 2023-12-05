#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <iostream>
#include <sstream>
#include "../helper.cpp"
using namespace std;
//http get c.png 7070
//http get c.png 7070
static const int MAXPENDING = 10; // Maximum outstanding connection requests
int const BUFFERSIZE = 1024;

//void *HandleTCPClient(void *arg) {
//    int clientSock = *((int *) arg);
//    free(arg); // Free memory allocated for the argument
//
//    char buffer[BUFFERSIZE]; // BUFFERSIZE is assumed to be defined somewhere
//    ssize_t numBytesRcvd;
//
//    // Receive and send data in a loop until the client closes the connection
//    while ((numBytesRcvd = recv(clientSock, buffer, BUFFERSIZE - 1, 0)) > 0) {
//        buffer[numBytesRcvd] = '\0'; // Null-terminate the received data
//        printf("Received: %s", buffer);
//
//        // Process the received data or perform other actions as needed
//
//        // Echo the data back to the client
//        ssize_t numBytesSent = send(clientSock, buffer, numBytesRcvd, 0);
//        if (numBytesSent < 0)
//            DieWithSystemMessage("send() failed");
//        else if (numBytesSent != numBytesRcvd)
//            DieWithUserMessage("send()", "sent unexpected number of bytes");
//    }
//
//    if (numBytesRcvd == 0) {
//        // ClientDir closed the connection
//        printf("ClientDir closed the connection.\n");
//    } else if (numBytesRcvd < 0) {
//        // Error occurred while receiving
//        DieWithSystemMessage("recv() failed");
//    }
//
//    close(clientSock); // Close the client socket
//    pthread_exit(NULL); // Exit the thread
//}

//HTTP/1.1 200 OK \r\n
//data
#define BUFFERSIZE 1024
int calculate_size(int data_size){
    string temp="HTTP/1.1 200 OK\\r\\n content-length: \\r\\n\\r\\n";
    int Request_size=data_size+temp.size();
    int Request_size_no_of_digits= to_string(Request_size).size();
    int Request_size_after_adding_no_of_digits= to_string(Request_size+Request_size_no_of_digits).size();
    if((Request_size_after_adding_no_of_digits)!=Request_size_no_of_digits){
        Request_size+=(Request_size_no_of_digits+1);
    }
    else{
        Request_size+=Request_size_no_of_digits;
    }
    return Request_size;
}

void extract_headers_and_data(const std::string& response, std::string& headers, std::string& data) {
    std::istringstream responseStream(response);

    // Read the status line (e.g., "HTTP/1.1 200 OK")
    std::string statusLine;
    std::getline(responseStream, statusLine);
    headers = statusLine;

    // Read the headers until an empty line is encountered
    std::string line;
    while (std::getline(responseStream, line) && !line.empty()) {
        headers += "\r\n" + line;
    }

    // Reset the stream to the beginning of the content
    responseStream.seekg(0, std::ios::beg);

    // Skip the status line and headers
    while (std::getline(responseStream, line) && !line.empty());

    // Read the rest of the stream as the data
    std::ostringstream dataStream;
    dataStream << responseStream.rdbuf();
    data = dataStream.str();
}

void build_response(std::string& response, const std::string& imageString, bool succeeded, bool includeImage) {
    std::ostringstream responseStream;

    if (succeeded) {
        responseStream << "HTTP/1.1 200 OK\r\n";

        if (includeImage) {
            responseStream << "Content-Length: " << imageString.size() << "\r\n";
        }

        responseStream << "\r\n";

        if (includeImage) {
            responseStream << imageString;
        }
    } else {
        responseStream << "HTTP/1.1 404 NotFound\r\n";
        responseStream << "Content-Length: 0\r\n";
        responseStream << "\r\n";
    }

    response = responseStream.str();
}

void ParseHttpGet(const std::string& httpRequest, std::string& filePath,string &file) {
    // Assuming a simple GET request format like "GET /path/to/file HTTP/1.1"
    std::istringstream ss(httpRequest);
    std::string method, path;
    ss >> method >> path ;
    filePath=path;
    file=change_file_to_string_image(path);
}

void ParseHttpPost(const std::string& httpRequest, std::string& filePath, std::string& fileContents) {
    // Assuming a simple POST request format like "POST /path/to/file HTTP/1.1\nContent-Length: 12\n\nHello, world!"
    std::istringstream ss(httpRequest);
    std::string method, path, version;
    ss >> method >> path >>fileContents ;
    if (method == "POST") {
        // Extract the file path from the request
        filePath = path.substr(1); // Remove leading '/'

        // Find the position of the double newline indicating the start of the content
        size_t contentStart = httpRequest.find("\\r\\n\\r\\n");

        if (contentStart != std::string::npos) {
            // Extract the content from the request
            fileContents = httpRequest.substr(contentStart + 2);
        }
    }
    saveString(fileContents, ExtractFilename(path));
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

            std::string method, path, version, filePath, fileContents,file="";
            // Check if it's a GET or POST request
            if (httpRequest.find("GET") == 0) {
                ParseHttpGet(httpRequest, filePath, file);
                int size=file.size();

                string response="HTTP/1.1 200 OK\\r\\n content-length: "+to_string(calculate_size(size))+"\\r\\n\\r\\n";

                send(clientSock, response.c_str(), response.size(), 0);
                while (!file.empty()) {
                    // Send the contents of 'file' in chunks
                    size_t bytesToSend = min(BUFFERSIZE, (int)file.length());

                    std::string buf = file.substr(0, bytesToSend);
                    send(clientSock, buf.c_str(), buf.size(), 0);
                    file.erase(0, bytesToSend);
                    // If there's more to send, continue the loop
                }

            } else if (httpRequest.find("POST") == 0) {
                ParseHttpPost(httpRequest, filePath, fileContents);
//                build_response(file, true, false);
            } else {
                std::cerr << "Unsupported HTTP method" << std::endl;
            }
        } else if (numBytesRcvd == 0) {
            std::cerr << "ClientDir closed the connection." << std::endl;
            break;  // Exit the loop if the client closes the connection
        } else {
//            std::cerr << "Error receiving data from client." << std::endl;
            break;  // Exit the loop if an error occurs
        }
    }

    close(clientSock);
    pthread_exit(NULL);
}



int main(int argc, char *argv[]) {
    if (argc != 2) // Test for correct number of arguments
        DieWithUserMessage("Parameter(s)", "<ServerDir Port>");

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
        struct sockaddr_in clientAddr; // ClientDir address
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
}

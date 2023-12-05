#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <iostream>
#include <sstream>
#include <mutex>
#include "../helper.cpp"

using namespace std;
//http get c.png 7070
//http get c.png 7070
static const int MAXPENDING = 10; // Maximum outstanding connection requests
int const BUFFERSIZE = 1024;
mutex count_mtx;
int clients = 0;
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

void ParseHttpGet(const std::string& httpRequest, std::string& filePath,string &file) {
    // Assuming a simple GET request format like "GET /path/to/file HTTP/1.1"
    std::istringstream ss(httpRequest);
    std::string method, path;
    ss >> method >> path ;
    filePath=path;
    file=change_file_to_string_image(path);
}




void *HandleTCPClient(void *arg) {
    int clientSock = *((int *)arg);
    free(arg);

    char buffer[BUFFERSIZE];
    count_mtx.lock();
    clients++;
    double time = 1.0 * DEFUALT_TIME / clients;
    if (time < MIN_TIME)
        time = MIN_TIME;
    count_mtx.unlock();

    auto currentTime = std::chrono::system_clock::now();

    // Convert the current time to milliseconds
    auto currentTimeMillis = std::chrono::time_point_cast<std::chrono::milliseconds>(currentTime);

    // Extract the count of milliseconds as a double
    double milliseconds = currentTimeMillis.time_since_epoch().count() / 1.0;
    // Receive and send data in a loop until the client closes the connection
    double currtime=milliseconds+time;
    // Receive and send data in a loop until the client closes the connection
    while (true) {
        auto currentTime1 = std::chrono::system_clock::now();

        // Convert the current time to milliseconds
        auto currentTimeMillis1 = std::chrono::time_point_cast<std::chrono::milliseconds>(currentTime1);

        // Extract the count of milliseconds as a double
        double milliseconds1 = currentTimeMillis1.time_since_epoch().count() / 1.0;
        if (milliseconds1>currtime)
        {
            cout << "time out finish  " << time << endl;
            close(clientSock);
            count_mtx.lock();
            clients--;
            count_mtx.unlock();
            pthread_exit(NULL);

        }
        ssize_t numBytesRcvd = recv(clientSock, buffer, BUFFERSIZE , 0);

        if (numBytesRcvd > 0) {
            buffer[numBytesRcvd] = '\0'; // Null-terminate the received data
            std::string httpRequest(buffer,numBytesRcvd);
            memset(buffer, 0, sizeof(buffer));
            std::string method, path, version, filePath, fileContents;
            // Check if it's a GET or POST request
            if (httpRequest.find("GET") == 0) {
                ParseHttpGet(httpRequest, filePath, fileContents);
                int size=fileContents.size();
                string response ;
                if(!fileContents.empty())
                     response="HTTP/1.1 200 OK\\r\\n content-length: "+to_string(calculate_size(size))+"\\r\\n\\r\\n";
                else
                     response="HTTP/1.1 404 NOTFOUND\\r\\n content-length: 44\\r\\n\\r\\n";
                send(clientSock, response.c_str(), response.size(), 0);
                while (!fileContents.empty()) {
                    // Send the contents of 'file' in chunks
                    size_t bytesToSend = min(BUFFERSIZE, (int)fileContents.length());
                bool first_time = true, data_begun_sending = false;

                    std::string buf = fileContents.substr(0, bytesToSend);
                    send(clientSock, buf.c_str(), buf.size(), 0);
                    fileContents.erase(0, bytesToSend);
                    // If there's more to send, continue the loop
                }

            } else if (httpRequest.find("POST") == 0) {
                ssize_t totalBytesRcvd = numBytesRcvd;
                ssize_t totalSize = -1;
                totalSize = extractContentSize(httpRequest);
                string response="HTTP/1.1 200 OK\\r\\n content-length: 38\\r\\n\\r\\n";
                send(clientSock,response.c_str(),response.size(),0);
                std::vector<char> file;
                string filename = extractFileName(httpRequest);
                while (true) {
                    ssize_t numBytesReceived = recv(clientSock, buffer, min(BUFFERSIZE,(int)(totalSize-totalBytesRcvd)), 0);
                    if (numBytesReceived < 0) {
                        DieWithSystemMessage("receiving file failed");
                    } else if (numBytesReceived == 0) {
                        break; // Connection closed by the server
                    }
                    totalBytesRcvd += numBytesReceived;
                    file.insert(file.end(), buffer, buffer + numBytesReceived);
                    if (totalSize <= totalBytesRcvd){
                        break;
                    }
                }
                saveBinaryData(file, filename);
                if (totalBytesRcvd == 0)
                    DieWithUserMessage("receiving file", "connection closed prematurely");

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
    count_mtx.lock();
    clients--;
    count_mtx.unlock();
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

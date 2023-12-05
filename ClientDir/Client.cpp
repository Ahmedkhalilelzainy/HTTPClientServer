#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <iostream>
#include <fstream>
#include "bits/stdc++.h"
#include "../helper.cpp"

using namespace std;
int const BUFFERSIZE = 1024;
int calculate_size(int data_size,int path_size){

    int Request_size=data_size+44+path_size;
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

void handleGet(const std::string& path, int sock) {
    string request = "GET " +
                     path +
                     " " +
                     "HTTP/1.1";
    std::string response;  // Variable to store anything before "\\r\\n\\r\\n"

    ssize_t numBytes = send(sock, request.c_str(), request.size(), 0);

    if (numBytes < 0) {
        DieWithSystemMessage("sending file failed");
    } else if (numBytes != request.size()) {
        DieWithUserMessage("sending file failed", "sent unexpected number of bytes");
    }

    char buffer[BUFFERSIZE];
    ssize_t totalBytesRcvd = 0;
    ssize_t totalSize = -1;
    bool first_time = true, data_begun_sending = false;
    std::vector<char> file;
    std::string filename = extractFileName(request);

    while (true) {
        ssize_t numBytesReceived = recv(sock, buffer, BUFFERSIZE, 0);

        if (numBytesReceived < 0) {
            DieWithSystemMessage("receiving file failed");
        } else if (numBytesReceived == 0) {
            break; // Connection closed by the server
        }
        totalBytesRcvd += numBytesReceived;

        if (first_time) {
            size_t headerEndPos = std::string(buffer, numBytesReceived).find("\\r\\n\\r\\n");
            if (headerEndPos != std::string::npos) {
                data_begun_sending = true;
                response += std::string(buffer, buffer + headerEndPos + 8);
                file.insert(file.end(), buffer + headerEndPos + 8, buffer + numBytesReceived);
            } else {
                response += std::string(buffer, buffer + numBytesReceived);
            }

            totalSize = extractContentSize(buffer);
            if (totalSize == -1) {
                DieWithSystemMessage("error: file size not received");
            }
            first_time = false;

            // Check for the specified string in the received data
            if (response.find("HTTP/1.1 404 NOTFOUND") != std::string::npos) {
                break;
            }
        } else if (data_begun_sending) {
            file.insert(file.end(), buffer, buffer + numBytesReceived);
        }

        if (totalSize <= totalBytesRcvd)
            break;
    }
    cout<<response<<"\n";
    // Skip calling saveBinaryData if the specified string is found
    if (response.find("HTTP/1.1 404 NOTFOUND") == std::string::npos) {
        // Uncomment the following line if you want to call saveBinaryData in other cases
         saveBinaryData(file, filename);
    }

    if (totalBytesRcvd == 0)
        DieWithUserMessage("receiving file", "connection closed prematurely");
}


void handlePost(const std::string&path,int sock){
    string data = change_file_to_string_image(path);
    if(data.empty()){
        cout<<"File Not found in client\n";
        return;
    }
    string request = "POST " +
              path +
              " " +
              "HTTP/1.1 \\r\\n " +
              "content-length: "+ to_string(calculate_size((int)data.size(),(int)path.size()))+"\\r\\n\\r\\n";
    send(sock, request.c_str(), request.size(), 0);
    char buffer[BUFFERSIZE];
    recv(sock, buffer, BUFFERSIZE, 0);
    buffer[BUFFERSIZE]='\0';
    cout<<buffer<<"\n";
    while (!data.empty()) {
        // Send the contents of 'file' in chunks
        size_t bytesToSend = min(BUFFERSIZE, (int)data.size());

        std::string buf = data.substr(0, bytesToSend);
        send(sock, buf.c_str(), buf.size(), 0);
        data.erase(0, bytesToSend);
    }

}

void handleResponse( string path, int sock, bool get) {
    if(get){
        handleGet(path,sock);
    }
    else{
        handlePost(path,sock);
    }
}






int main(int argc, char *argv[]) {
    if (argc < 2 || argc > 3) // Test for correct number of arguments
        DieWithUserMessage("Parameter(s)",
                           "<ServerDir Address> [<ServerDir Port>]");
//    127.0.0.1 home/text.txt 8008
    char *host_name = argv[1];
// First arg: server IP address (dotted quad)


// Third arg (optional): server port (numeric). 7 is well-known echo port
    in_port_t servPort = (argc == 3) ? atoi(argv[2]) : 7;
// Create a reliable, stream socket using TCP
    int sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sock < 0)
        DieWithSystemMessage("creating socket failed");
// Construct the server address structure
    struct sockaddr_in servAddr;
// ServerDir address
    memset(&servAddr, 0, sizeof(servAddr)); // Zero out structure
    servAddr.sin_family = AF_INET;
// IPv4 address family
// Convert address to binary form
    int rtnVal = inet_pton(AF_INET, host_name, &servAddr.sin_addr.s_addr);
    if (rtnVal == 0)
        DieWithUserMessage("converting IPv4 to binary address failed", "invalid address string");
    else if (rtnVal < 0)
        DieWithSystemMessage("converting IPv4 to binary address failed");
    servAddr.sin_port = htons(servPort);
// ServerDir port
    if (connect(sock, (struct sockaddr *) &servAddr, sizeof(servAddr)) < 0)
        DieWithSystemMessage("connection failed");
    //reading the command
    char cwd[PATH_MAX]; // PATH_MAX is a constant that represents the maximum path length

    if (getcwd(cwd, sizeof(cwd)) == nullptr) {
        DieWithSystemMessage("cwd error"); // Return an error code
    }
    string filepath(cwd);
    filepath+=("/requests.txt");

    // Open a file for reading
    ifstream inputFile(filepath);

    // Check if the file is opened successfully
    if (!inputFile.is_open()) {
        cerr << "Error opening file." << "\n";
        return 1; // Return an error code
    }
    string line;
    // Read the file content
    while(getline(inputFile, line)) {
        // Close the file

// getting the type of request and file path
        istringstream iss(line);
        vector<string> tokens;
        do {
            string token;
            iss >> token;
            tokens.push_back(token);
        } while (iss);

        string type_of_request = tokens[0];
        string path = tokens[1];
        string request;
        if (type_of_request == "client_get") {
            handleResponse(path,sock,true);
        } else if (type_of_request == "client_post") {
            handleResponse(path,sock,false);
        }
    }
    inputFile.close();
    close(sock);

}
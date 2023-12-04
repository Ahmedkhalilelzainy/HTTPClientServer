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

void handleResponse(string request,string path,int sock,bool get){
    string response="";
    ssize_t numBytes = send(sock, request.c_str(), request.size(), 0);
    if (numBytes < 0)
        DieWithSystemMessage("sending file failed");
    else if (numBytes != request.size())
        DieWithUserMessage("sending file failed", "sent unexpected number of bytes");

    char buffer[BUFFERSIZE];
    ssize_t totalBytesRcvd = 0;
    while (true) {
        ssize_t numBytesReceived = recv(sock, buffer, BUFFERSIZE-1, 0);

        if (numBytesReceived < 0){
            DieWithSystemMessage("receiving file failed");
        }
        else if (numBytesReceived == 0) {
            break; // Connection closed by the server
        }
        cout<<buffer<<"\n";
        totalBytesRcvd += numBytesReceived;

        string temp (buffer);
//        cout<<temp<<" fasla "<<"\n";
        response+=temp;
        if(response.find("\\r\\n\\r\\n")!=std::string::npos){

            break;
        }

    }
//    cout<<response<<"\n";

//    cout<<response;
//    saveString(response, ExtractFilename(path));
    if (totalBytesRcvd == 0)
        DieWithUserMessage("receiving file", "connection closed prematurely");

//    std::cout << "Received file successfully and saved as 'received_file.txt'" << std::endl;

}

//void handleResponse(string request,string path,int sock,bool get){
//    string response="";
//    ssize_t numBytes = send(sock, request.c_str(), request.size(), 0);
//    if (numBytes < 0)
//        DieWithSystemMessage("sending file failed");
//    else if (numBytes != request.size())
//        DieWithUserMessage("sending file failed", "sent unexpected number of bytes");
//
//
//    char buffer[BUFFERSIZE];
//    ssize_t totalBytesRcvd = 0;
//    while (true) {
//
//        ssize_t numBytesReceived = recv(sock, buffer, BUFFERSIZE-1, 0);
//        if (numBytesReceived < 0){
//            DieWithSystemMessage("receiving file failed");
//        }
//        else if (numBytesReceived == 0) {
//            break; // Connection closed by the server
//        }
//        totalBytesRcvd += numBytesReceived;
//        string temp(buffer);
//        response+=temp;
//        if(response.find("\n\n")!=0){
//            break;
//        }
//
//    }
//    cout<<"hena"<<response<<"\n";
////    saveString(response, ExtractFilename(path));
//    if (totalBytesRcvd == 0)
//        DieWithUserMessage("receiving file", "connection closed prematurely");
//
//    std::cout << "Received file successfully and saved as 'received_file.txt'" << std::endl;
//
//}





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
// Establish the connection to the echo server
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
        cerr << "Error opening file." << std::endl;
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
            request = "GET " +
                      path +
                      " " +
                      "HTTP/1.1";
            handleResponse(request,path,sock,true);
        } else if (type_of_request == "client_post") {
            string data;
            change_file_to_string(path,data);
            int Request_size =38+path.length()+data.length();
            int Request_size_no_of_digits= to_string(Request_size).size();
            int Request_size_after_adding_no_of_digits= to_string(Request_size+Request_size_no_of_digits).size();
            if((Request_size_after_adding_no_of_digits)!=Request_size_no_of_digits){
                Request_size+=(Request_size_no_of_digits+1);
            }
            else{
                Request_size+=Request_size_no_of_digits;
            }

            request = "POST " +
                      path +
                      " " +
                      "HTTP/1.1 \n " +
                      "Content-Length: "+ to_string(Request_size);
                      +"\n\n"+
                      data;
            handleResponse(request,path,sock,false);
        }
//        cout << request << "\n";
    }
    inputFile.close();
    close(sock);

}
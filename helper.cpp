#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <climits>
using namespace std;
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
//        DieWithSystemMessage("Error opening file");
void change_file_to_string(string filePath,string &fileContent){
    std::ifstream infile(filePath);
    if (!infile) {
        DieWithSystemMessage("Error opening file");
    }

    std::string content;
    std::string line;

    while (std::getline(infile, line)) {
        content += line +'\n'; // You can modify this line based on your requirements
    }
    content.pop_back();

    infile.close();
    fileContent=content;
}


void saveString(string content,string fileName){
    std::ofstream outfile(fileName);
    if (!outfile) {
        std::cerr << "Error opening file for writing: " << fileName << std::endl;
        return;
    }

    outfile << content;
    outfile.close();
}
string ExtractFilename(const std::string& filePath) {
    size_t lastSlash = filePath.find_last_of('/');
    if (lastSlash != std::string::npos) {
        return filePath.substr(lastSlash + 1);
    } else {
        // No slash found, return the entire string
        return filePath;
    }
}
int GetContentLength(const std::string& response,int&contentLength) {
    int pos = response.find("Content-Length:");
    if (pos != std::string::npos) {
        std::istringstream iss(response.substr(pos + 15)); // Skip "Content-Length: "
        iss >> contentLength;
    }
    return contentLength;
}

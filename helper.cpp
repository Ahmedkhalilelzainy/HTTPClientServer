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

void change_file_to_string(const std::string& sourceFilePath, std::string& fileContent) {
    // Open the source file for reading
    std::ifstream sourceFile(sourceFilePath);

    // Check if the source file is open
    if (!sourceFile.is_open()) {
        DieWithSystemMessage("Error opening source file");
    }

    // Read the entire content of the file into a string, removing trailing newline characters
    std::ostringstream fileContentStream;
    std::string line;

    while (getline(sourceFile, line)) {
        fileContentStream << line;  // Add newline between lines
    }

    // Close the source file
    sourceFile.close();

    // Get the content as a string
    fileContent = fileContentStream.str();

    std::cout << "File content size: " << fileContent.size() ;
}
void saveString(string fileContent,string fileName){
    char cwd[PATH_MAX]; // PATH_MAX is a constant that represents the maximum path length

    if (getcwd(cwd, sizeof(cwd)) == nullptr) {
        perror("getcwd() error");
        return; // Return an error code
    }
//     Specify the destination file path
    string destinationFolderPath(cwd);
    string destinationFilePath = destinationFolderPath +"/"+fileName;

    // Open the destination file for writing
    std::ofstream destinationFile(destinationFilePath,std::ios::binary);

    // Check if the destination file is open
    if (!destinationFile.is_open()) {
        std::cerr << "Error opening destination file: " << destinationFilePath << std::endl;
        DieWithSystemMessage("Error opening destination file");
        return;
    }

    // Write the content to the destination file
    destinationFile << fileContent;

    // Close the destination file
    destinationFile.close();

    std::cout << "File content copied from client to " << destinationFilePath << std::endl;
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

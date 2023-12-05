#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <climits>
#include <vector>
#include <filesystem>

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
    std::ifstream infile(filePath,std::ios::binary);
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
string change_file_to_string_image(const std::string& imageFilePath) {
    std::ifstream imageFile(imageFilePath, std::ios::binary);

    if (!imageFile) {
        std::cerr << "Error opening image file: " << imageFilePath << std::endl;
        return "";
    }

    std::ostringstream imageStream;
    imageStream << imageFile.rdbuf();
    return imageStream.str();
}

void saveBinaryData(const std::vector<char>& data, const std::string& fileName) {
    // Get the current working directory
    std::string currentDirectory = std::filesystem::current_path();

    // Construct the full path by appending the file name to the current directory
    std::string fullPath = currentDirectory + "/" + fileName;

    std::ofstream outfile(fullPath, std::ios::binary);
    if (!outfile) {
        DieWithSystemMessage("Error opening file for writing");
    }

    outfile.write(data.data(), data.size());
    outfile.close();
}
void saveString(const std::string& content, const std::string& fileName) {
    std::ofstream outfile(fileName, std::ios::binary);
    if (!outfile) {
        std::cerr << "Error opening file for writing: " << fileName << std::endl;
        return;
    }

    outfile.write(content.c_str(), content.size());
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

size_t extractContentSize(const std::string& httpResponse) {
    size_t contentSize = 0;

    // Find the "Content-Length:" header in the response
    int pos = httpResponse.find("content-length: ");
    if (pos != std::string::npos) {
        // Find the end of the line after the header
        pos = httpResponse.find_first_of("\\r\\n", pos);
        if (pos != std::string::npos) {
            // Extract the content size value after the header
            pos = httpResponse.find_first_of("0123456789", pos);
            if (pos != std::string::npos) {
                contentSize = std::stoull(httpResponse.substr(pos));
            }
        }
    }

    return contentSize;
}
std::string extractFileName(const std::string& httpRequest) {
    // Find the position of the first space after "GET "
    size_t startPos = httpRequest.find("GET ") + 4; // Move to the character after "GET "
    size_t endPos = httpRequest.find(" ", startPos);

    // Extract the substring between startPos and endPos
    if (startPos != std::string::npos && endPos != std::string::npos) {
        std::string filePath = httpRequest.substr(startPos, endPos - startPos);

        // Find the last occurrence of '/' to get the file name
        size_t lastSlashPos = filePath.find_last_of('/');
        if (lastSlashPos != std::string::npos) {
            return filePath.substr(lastSlashPos + 1);
        } else {
            // If no '/', return the full path as the file name
            return filePath;
        }
    }

    // Return an empty string if extraction fails
    return "";
}
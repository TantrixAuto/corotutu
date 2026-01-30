#include <iostream>
#include <string>
#include <fcntl.h>
#include <unistd.h>

void printFile();
int main() {
    printFile();
    return 0;    
}

void printFile() {
    int fd = open("test.txt", O_RDONLY);
    if(fd < 0) {
        std::cerr << "Error opening file" << std::endl;
        return;
    }

    std::cout << "File opened synchronously" << std::endl;
    char buffer[1024];
    ssize_t bytesRead = read(fd, buffer, sizeof(buffer));

    if (bytesRead < 0) {
        std::cerr << "Error reading file" << std::endl;
        return;
    }

    std::string c = std::string(buffer, bytesRead);
    std::cout << c << std::endl;

    close(fd);
}

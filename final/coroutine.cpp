#include "coroutine.hpp"
#include <fcntl.h>
#include <uv.h>

int r = 0;
uv_buf_t ubuf;
uv_fs_t freq;
char buffer[1024];
OpenAwaiter openAwaiter;
CreateAwaiter createAwaiter;
ReadAwaiter readAwaiter;
WriteAwaiter writeAwaiter;
void openFileCB(uv_fs_t* req);
void createFileCB(uv_fs_t* req);
void readFileCB(uv_fs_t* req);
void writeFileCB(uv_fs_t* req);

OpenAwaiter& open() {
    uv_loop_t* loop = uv_default_loop();
    r = uv_fs_open(loop, &freq, "test.txt", O_RDONLY, 0, openFileCB);
    if(r < 0) {
        std::cerr << "Error opening file:" << uv_strerror(r) << std::endl;
        throw std::runtime_error("uv_fs_open error");
    }

    return openAwaiter;
}

void openFileCB(uv_fs_t* req) {
    if(req->result < 0) {
        std::cerr << "Error opening file:" << uv_strerror(req->result) << std::endl;
        throw std::runtime_error("open file error");
    }
    openAwaiter.val = req->result;
    std::cout << "resuming open" << std::endl;
    openAwaiter.handle.resume();
}

CreateAwaiter& create() {
    uv_loop_t* loop = uv_default_loop();
    r = uv_fs_open(loop, &freq, "test.out", O_CREAT | O_WRONLY | O_TRUNC, S_IRUSR | S_IWUSR, createFileCB);
    if(r < 0) {
        std::cerr << "Error creating file:" << uv_strerror(r) << std::endl;
        throw std::runtime_error("uv_fs_open error");
    }

    return createAwaiter;
}

void createFileCB(uv_fs_t* req) {
    if(req->result < 0) {
        std::cerr << "Error opening file:" << uv_strerror(req->result) << std::endl;
        throw std::runtime_error("create file error");
    }
    createAwaiter.val = req->result;
    std::cout << "resuming create" << std::endl;
    createAwaiter.handle.resume();
}

ReadAwaiter& read(int fd) {
    uv_loop_t* loop = uv_default_loop();
    ubuf = uv_buf_init(buffer, sizeof(buffer));
    r = uv_fs_read(loop, &freq, fd, &ubuf, 1, 0, readFileCB);
    if(r < 0) {
        std::cerr << "Error reading file:" << uv_strerror(r) << std::endl;
        throw std::runtime_error("uv_fs_read error");
    }
    return readAwaiter;
}

void readFileCB(uv_fs_t* req) {
    if(req->result < 0) {
        std::cerr << "Error reading file:" << uv_strerror(req->result) << std::endl;
        throw std::runtime_error("read file error");
    }
    if(req->result > 0) {
        readAwaiter.val = std::string(buffer, req->result);
    }
    readAwaiter.handle.resume();
}

WriteAwaiter& write(int fd, std::string data) {
    uv_loop_t* loop = uv_default_loop();
    ubuf = uv_buf_init(data.data(), data.size());
    r = uv_fs_write(loop, &freq, fd, &ubuf, 1, -1, writeFileCB);
    if(r < 0) {
        std::cerr << "Error writing file:" << uv_strerror(r) << std::endl;
        throw std::runtime_error("uv_fs_write error");
    }
    return writeAwaiter;
}

void writeFileCB(uv_fs_t* req) {
    if(req->result < 0) {
        std::cerr << "Error writing file:" << uv_strerror(req->result) << std::endl;
        throw std::runtime_error("write file error");
    }
    writeAwaiter.handle.resume();
}

void close(int fd) {
    uv_loop_t* loop = uv_default_loop();
    r = uv_fs_close(loop, &freq, fd, NULL);
    if(r < 0) {
        std::cerr << "Error closing file:" << uv_strerror(r) << std::endl;
        return;
    }
}
// END: coroutine.cpp
//////////////////////////////////////////////////////////////

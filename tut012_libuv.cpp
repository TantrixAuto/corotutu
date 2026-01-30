#include <uv.h>
#include <iostream>
#include <fcntl.h>

int r = 0;
uv_loop_t* loop = nullptr;
int fd = -1;
uv_buf_t ubuf;
uv_fs_t freq;
char buffer[1024];
void openFileCB(uv_fs_t* req);
void readFileCB(uv_fs_t* req);
void open();
void read(uv_fs_t* req);
void close(uv_fs_t* req);

int main() {
    loop = uv_default_loop();
    open();
    std::cout << "starting loop" << std::endl;
    r = uv_run(loop, UV_RUN_DEFAULT);
    return 0;    
}

void open() {
    r = uv_fs_open(loop, &freq, "test.txt", O_RDONLY, 0, openFileCB);
    if(r < 0) {
        std::cerr << "Error opening file:" << uv_strerror(r) << std::endl;
        return;
    }

    return;
}

void openFileCB(uv_fs_t* req) {
    read(req);
}

void read(uv_fs_t* req) {
    fd = req->result;
    if(fd < 0) {
        std::cerr << "Error opening file:" << uv_strerror(fd) << std::endl;
        return;
    }
    std::cout << "File opened asynchronously" << std::endl;
    ubuf = uv_buf_init(buffer, sizeof(buffer));
    r = uv_fs_read(loop, req, fd, &ubuf, 1, 0, readFileCB);
    if(r < 0) {
        std::cerr << "Error reading file:" << uv_strerror(r) << std::endl;
        return;
    }
}

void readFileCB(uv_fs_t* req) {
    close(req);
}

void close(uv_fs_t* req) {
    std::cout << buffer << std::endl;
    r = uv_fs_close(loop, req, fd, NULL);
    if(r < 0) {
        std::cerr << "Error closing file:" << uv_strerror(r) << std::endl;
        return;
    }
}

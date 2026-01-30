#include <uv.h>
#include <iostream>
#include <fcntl.h>
#include <coroutine>

struct Future {
    struct promise_type {
        Future get_return_object() noexcept {
            return Future{};
        }

        auto initial_suspend() const noexcept {
            return std::suspend_never{};
        }

        auto final_suspend() const noexcept {
            return std::suspend_never{};
        }

        void unhandled_exception() {
            // handle any exceptions
        }
    };
};

struct OpenAwaiter {
    std::coroutine_handle<> handle;
    int fd = -1;
    bool await_ready() {
        return false;
    }
    auto await_suspend(std::coroutine_handle<Future::promise_type> h) {
        handle = h;
    }
    auto await_resume() {
        return fd;
    }
};

struct ReadAwaiter {
    std::coroutine_handle<> handle;
    std::string text;
    bool await_ready() {
        return false;
    }
    auto await_suspend(std::coroutine_handle<Future::promise_type> h) {
        handle = h;
    }
    auto await_resume() {
        return text;
    }
};

int r = 0;
uv_loop_t* loop = nullptr;
uv_buf_t ubuf;
uv_fs_t freq;
char buffer[1024];
OpenAwaiter openAwaiter;
ReadAwaiter readAwaiter;

Future printFile();
void openFileCB(uv_fs_t* req);
void readFileCB(uv_fs_t* req);
OpenAwaiter& open();
ReadAwaiter& read(int fd);
void close(int fd);

int main() {
    loop = uv_default_loop();
    std::cout << "1. calling printFile" << std::endl;
    printFile();
    std::cout << "2. starting loop" << std::endl;
    r = uv_run(loop, UV_RUN_DEFAULT);
    return 0;    
}

Future printFile() {
    int i = 0;
    auto fd = co_await open();
    std::cout << "4. File opened asynchronously via coroutine" << std::endl;
    std::string t = co_await read(fd);
    std::cout << t << std::endl;
    close(fd);
}

OpenAwaiter& open() {
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
    openAwaiter.fd = req->result;
    std::cout << "3. resuming open" << std::endl;
    openAwaiter.handle.resume();
}

ReadAwaiter& read(int fd) {
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
        readAwaiter.text = std::string(buffer, req->result);
    }
    readAwaiter.handle.resume();
}

void close(int fd) {
    r = uv_fs_close(loop, &freq, fd, NULL);
    if(r < 0) {
        std::cerr << "Error closing file:" << uv_strerror(r) << std::endl;
        return;
    }
}

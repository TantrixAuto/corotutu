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
        std::cout << "AWAITER: suspending" << std::endl;
        handle = h;
    }
    auto await_resume() {
        std::cout << "AWAITER: resuming" << std::endl;
        return fd;
    }
};

int r = 0;
uv_loop_t* loop = nullptr;
uv_buf_t ubuf;
uv_fs_t freq;
OpenAwaiter openAwaiter;

Future printFile();
void openFileCB(uv_fs_t* req);
OpenAwaiter& open();

int main() {
    loop = uv_default_loop();
    printFile();
    std::cout << "starting loop" << std::endl;
    r = uv_run(loop, UV_RUN_DEFAULT);
    std::cout << "stopped loop" << std::endl;
    return 0;    
}

Future printFile() {
    int i = 0;
    std::cout << "PRINTFILE: calling open()" << std::endl;
    auto fd = co_await open();
    std::cout << "PRINTFILE: File opened asynchronously via coroutine" << std::endl;
}

OpenAwaiter& open() {
    r = uv_fs_open(loop, &freq, "test.txt", O_RDONLY, 0, openFileCB);
    if(r < 0) {
        throw std::runtime_error("uv_fs_open error");
    }

    std::cout << "open-file initiated" << std::endl;
    return openAwaiter;
}

void openFileCB(uv_fs_t* req) {
    std::cout << "open-file callback received" << std::endl;
    if(req->result < 0) {
        throw std::runtime_error("open file error");
    }
    openAwaiter.fd = req->result;
    openAwaiter.handle.resume();
}

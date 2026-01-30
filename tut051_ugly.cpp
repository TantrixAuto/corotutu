/*
This code is butt-ugly.
It is written this way so that it is relatable to
the code fragments shown in the slides.
The next file is beautiful.
*/
#include <uv.h>
#include <iostream>
#include <fcntl.h>
#include <coroutine>

struct OpenAwaiter {
    std::coroutine_handle<> handle;
    int fd = -1;
    bool await_ready() {
        return false;
    }
    auto await_suspend(std::coroutine_handle<> h) {
        handle = h;
    }
    auto await_resume() {
        return fd;
    }
};

struct CreateAwaiter {
    std::coroutine_handle<> handle;
    int fd = -1;
    bool await_ready() {
        return false;
    }
    auto await_suspend(std::coroutine_handle<> h) {
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
    auto await_suspend(std::coroutine_handle<> h) {
        handle = h;
    }
    auto await_resume() {
        return text;
    }
};

struct WriteAwaiter {
    std::coroutine_handle<> handle;
    bool await_ready() {
        return false;
    }
    auto await_suspend(std::coroutine_handle<> h) {
        handle = h;
    }
    auto await_resume() {
        return; // returns void
    }
};

struct SaveFuture {
    struct promise_type {
        std::coroutine_handle<> continuation;
        SaveFuture get_return_object() noexcept {
            return SaveFuture{std::coroutine_handle<promise_type>::from_promise(*this)};
        }

        auto initial_suspend() const noexcept {
            return std::suspend_never{};
        }

        auto final_suspend() const noexcept {
            struct FinalAwaiter {
                bool await_ready() const noexcept { return false; }
                auto await_suspend(std::coroutine_handle<SaveFuture::promise_type> h2) noexcept {
                    h2.promise().continuation.resume();
                }
                void await_resume() const noexcept {}
            };
            return FinalAwaiter{};
        }

        void unhandled_exception() {
            // handle any exceptions
        }
    };

    std::coroutine_handle<promise_type> handle;
};

struct Future {
    struct promise_type {
        Future get_return_object() noexcept {
            return Future{};
        }

        auto initial_suspend() const noexcept {
            return std::suspend_never{};
        }

        auto await_transform(const SaveFuture& futureSave) const noexcept {
            struct FutureAwaiter {
                const SaveFuture& futureSave;
                bool await_ready() const noexcept { return false; }
                auto await_suspend(std::coroutine_handle<> h) {
                    futureSave.handle.promise().continuation = h;
                }
                void await_resume() const noexcept {}
            };
            return FutureAwaiter{futureSave};
        }

        auto& await_transform(OpenAwaiter& aw) const noexcept {
            return aw;
        }

        auto& await_transform(ReadAwaiter& aw) const noexcept {
            return aw;
        }

        auto final_suspend() const noexcept {
            return std::suspend_never{};
        }

        void unhandled_exception() {
            // handle any exceptions
        }
    };
};

Future printFile();
SaveFuture saveFile(std::string data);
OpenAwaiter& open();
CreateAwaiter& create();
ReadAwaiter& read(int fd);
WriteAwaiter& write(int fd, std::string data);
void close(int fd);
uv_loop_t* loop = nullptr;

int main() {
    loop = uv_default_loop();
    std::cout << "calling printFile" << std::endl;
    printFile();
    std::cout << "starting loop" << std::endl;
    int r = uv_run(loop, UV_RUN_DEFAULT);
    std::cout << "stopped loop" << std::endl;
    return 0;    
}

Future printFile() {
    int i = 0;
    auto fd = co_await open();
    std::cout << "File opened asynchronously via coroutine" << std::endl;
    std::string t = co_await read(fd);
    std::cout << t << std::endl;
    std::cout << "saving" << std::endl;
    co_await saveFile(t);
    std::cout << "saved" << std::endl;
    close(fd);
}

SaveFuture saveFile(std::string data) {
    int i = 0;
    std::cout << "creating" << std::endl;
    auto fd = co_await create();
    std::cout << "File created asynchronously via coroutine" << std::endl;
    co_await write(fd, data);
    close(fd);
}

//////////////////////////////////////////////////////////////
// Everything below this line will typically be reusable code
// in a separate library, either 3rd party, or written by you.
// Your application code is the 3 functions above.
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
    std::cout << "resuming open" << std::endl;
    openAwaiter.handle.resume();
}

CreateAwaiter& create() {
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
    createAwaiter.fd = req->result;
    std::cout << "resuming create" << std::endl;
    createAwaiter.handle.resume();
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

WriteAwaiter& write(int fd, std::string data) {
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
    r = uv_fs_close(loop, &freq, fd, NULL);
    if(r < 0) {
        std::cerr << "Error closing file:" << uv_strerror(r) << std::endl;
        return;
    }
}

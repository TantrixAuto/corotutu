#include "coroutine.hpp"
#include <uv.h>

Future<void> printFile();
Future<void> saveFile(std::string data);
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

Future<void> printFile() {
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

Future<void> saveFile(std::string data) {
    int i = 0;
    std::cout << "creating" << std::endl;
    auto fd = co_await create();
    std::cout << "File created asynchronously via coroutine" << std::endl;
    co_await write(fd, data);
    close(fd);
}

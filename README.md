# corotutu: A C++ Coroutine Tutorial
This project contains all sample files provided along with the C++ coroutine tutorial video.

## List of files
| Name                                           | Description.                          |
|------------------------------------------------|---------------------------------------|
| [tut011_plain.cpp](tut011_plain.cpp)                          | Most basic file IO operation using plain POSIX API.    |
| [tut012_libuv.cpp](tut012_libuv.cpp)                          | Asynchronous file IO operation using libuv.    |
| [tut021_basic.cpp](tut021_basic.cpp)                          | Asynchronous file open operation using awaiters.    |
| [tut022_full.cpp](tut022_full.cpp)                          | All asynchronous file IO operations (open, read, close) using awaiters.    |
| [tut051_ugly.cpp](tut051_ugly.cpp)                          | Asynchronous file IO operations (open, create, read, write, close) using promises and awaiters, un-refactored code.    |
| [tut052_beautiful.cpp](tut052_beautiful.cpp)                          | Asynchronous file IO operations (open, create, read, write, close) using promises and awaiters, refactored code.    |
| [final/](final/)                          | Final coroutine code.    |

 
## How to install libuv
| OS    | Command   |
|-------|-----------|
| Linux | ```sudo apt install libuv1-dev``` |
| Macos | ```brew install libuv``` |

## How to build and run
| Step    | Command   |
|-------|-----------|
| Build | ```clang++ -g --std=c++20 tut012_libuv.cpp -luv``` |
| Run   | ```./a.out``` |



## How to contact:
| OS    | Command   |
|-------|-----------|
| LinkedIn | https://www.linkedin.com/in/renjipanicker |
| email   | renjipanicker@gmail.com |

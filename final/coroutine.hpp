#pragma once

#include <iostream>
#include <coroutine>

template<typename TypeT>
struct AwaiterT {
    std::coroutine_handle<> handle;
    TypeT val;
    bool await_ready() {
        return false;
    }
    auto await_suspend(std::coroutine_handle<> h) {
        handle = h;
    }
    auto await_resume() {
        return val;
    }
};

template<>
struct AwaiterT<void> {
    std::coroutine_handle<> handle;
    bool await_ready() {
        return false;
    }
    auto await_suspend(std::coroutine_handle<> h) {
        handle = h;
    }
    void await_resume() {
        return; // return void
    }
};

template<typename TypeT>
struct Future {
    struct promise_type {
        std::coroutine_handle<> continuation;
        Future get_return_object() noexcept {
            return Future{std::coroutine_handle<promise_type>::from_promise(*this)};
        }

        auto initial_suspend() const noexcept {
            return std::suspend_never{};
        }

        template<typename T>
        auto await_transform(const Future<T>& f) const noexcept {
            struct FutureAwaiter {
                const Future<T>& future;
                bool await_ready() const noexcept { return false; }
                auto await_suspend(std::coroutine_handle<> h) {
                    future.handle.promise().continuation = h;
                }
                void await_resume() const noexcept {}
            };
            return FutureAwaiter{f};
        }

        template<typename T>
        auto& await_transform(AwaiterT<T>& aw) const noexcept {
            return aw;
        }

        auto final_suspend() const noexcept {
            struct FinalAwaiter {
                bool await_ready() const noexcept { return false; }

                auto await_suspend(std::coroutine_handle<typename Future::promise_type> h2) noexcept {
                    if(h2.promise().continuation) {
                        h2.promise().continuation.resume();
                    }
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

using OpenAwaiter = AwaiterT<int>;
using CreateAwaiter = AwaiterT<int>;
using ReadAwaiter = AwaiterT<std::string>;
using WriteAwaiter = AwaiterT<void>;
OpenAwaiter& open();
CreateAwaiter& create();
ReadAwaiter& read(int fd);
WriteAwaiter& write(int fd, std::string data);
void close(int fd);

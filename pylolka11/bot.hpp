/*
 * SPDX-License-Identifier: MIT
 * Copyright (c) 2026 Nos0kkk
 */

#pragma once
#include <pybind11/pytypes.h>
#include <python3.14/refcount.h>
#include <string>
#include <vector>
#include <optional>
#include <map>
#include <python3.14/Python.h>
#include <pybind11/embed.h>
#include <pybind11/stl.h>
#include <pybind11/eval.h>
#include <pybind11/cast.h>
#include <memory>
#include <atomic>

namespace py = pybind11;

namespace lol {
    enum class event {
        on_ready,
        on_message
    };

    class message {
        public:
        message() = default;
        message(py::object obj) : obj_(std::move(obj)) {}

        std::string content() const {
            return obj_.attr("content").cast<std::string>();
        }

        std::string author() const {
            return obj_.attr("author").attr("name").cast<std::string>();
        }

        bool bot() const {
            return obj_.attr("author").attr("bot").cast<bool>();
        }

        int64_t author_id() const {
            return obj_.attr("author").attr("id").cast<int64_t>();
        }

        std::string author_username() const {
            return "<@" + std::to_string(obj_.attr("author").attr("id").cast<int64_t>()) + ">";
        }

        std::string channel_name() const {
            return obj_.attr("channel").attr("name").cast<std::string>();
        }

        int64_t channel_id() const {
            return obj_.attr("channel").attr("id").cast<int64_t>();
        }

        bool pinned() const {
            return obj_.attr("pinned").cast<bool>();
        }

        void channel_send(const std::string& content) const {
            py::object coro = obj_.attr("channel").attr("send")(content);
            auto asyncio = py::module_::import("asyncio");
            asyncio.attr("ensure_future")(coro);
        }

        int64_t guild_id() const {
            return obj_.attr("guild").attr("id").cast<int64_t>();
        }

        std::string guild_name() const {
            return obj_.attr("guild").attr("name").cast<std::string>();
        }

        py::object raw() const { return obj_; }

        private:
        py::object obj_;
    };

    struct slash_options {
        std::string name;
        std::string description;
        std::map<std::string, std::string> options;
    };

    class client {
        public:

        struct options {
            std::atomic<bool> logs;
        } options;

        client(const std::string& token, std::optional<char> prefix = std::nullopt) : token_(token) {
            // if (!Py_IsInitialized()) {
            //     Py_Initialize();
            // }

            py_guard = std::make_unique<py::scoped_interpreter>();

            PyRun_SimpleString(
                "def _make_async(f, name):\n  async def wrapper(*a, **kw):\n    return f(*a, **kw)\n  wrapper.__name__ = name\n  return wrapper"
            );

            PyObject* main_mod = PyImport_ImportModule("__main__");
            PyObject* func = PyObject_GetAttrString(main_mod, "_make_async");
            make_async_func = py::reinterpret_steal<py::object>(func);
            Py_DECREF(main_mod);

            lolka = py::module_::import("lolka");

            ints = lolka.attr("Intents")();
            ints.attr("message_content") = true;
            ints.attr("messages") = true;
            ints.attr("guilds") = true;
            ints.attr("guild_messages") = true;

            cli = lolka.attr("Client")(py::arg("intents") = ints);

            options.logs = false;
        }

        void run() {
            cli.attr("run")(token_);
        }

        template<typename E_> void on_event(const event& e, E_&& callback) {
            std::string name_str;
            if (e == event::on_message) name_str = "on_message";
            else if (e == event::on_ready) name_str = "on_ready";
            else return;

            if (options.logs) {
                printf("[pylolka11] Registering event '%s', is_invocable<context> = %d\n",
                    name_str.c_str(),
                    (int)std::is_invocable_v<E_, const message&>);
                fflush(stdout);
            }

            py::object sync_cb;

            if constexpr (std::is_invocable_v<E_, const message&>) {
                if (options.logs) {
                    printf("[pylolka11] -> using adapter branch\n");
                    fflush(stdout);
                }
                auto adapter = [cb = std::forward<E_>(callback), this](const py::object& obj){
                    if (options.logs) {
                        printf("[pylolka11] on_message callback invoked!\n");
                        fflush(stdout);
                    }
                    message ctx(obj);
                    cb(ctx);
                };
                sync_cb = py::object(py::cpp_function(adapter, py::name(name_str.c_str())));
            } else {
                if (options.logs) {
                    printf("[pylolka11] -> using direct branch\n");
                    fflush(stdout);
                }
                sync_cb = py::object(py::cpp_function(std::forward<E_>(callback), py::name(name_str.c_str())));
            }

            auto async_cb = make_async_func(sync_cb, name_str);
            async_cb.attr("__name__") = name_str;

            handlers_.push_back(cli.attr("event")(async_cb));
            if (options.logs) {
                printf("[pylolka11] Event '%s' registered\n", name_str.c_str());
                fflush(stdout);
            }
        }

        template<typename F> void on_slash(const slash_options& opt, F&& callback);

        private:
        std::string token_;
        py::object cli;
        py::module_ lolka;
        py::object ints;
        std::vector<py::object> handlers_;
        std::unique_ptr<py::scoped_interpreter> py_guard;
        py::object make_async_func;
    };
}
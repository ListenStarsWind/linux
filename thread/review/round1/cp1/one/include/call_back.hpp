#pragma once

#include <memory>
#include <string>
#include <utility>

struct call_back_base {
    virtual void call() = 0;
    virtual ~call_back_base() = default;
    virtual operator std::string() const {
        return {};
    };
};

template <typename Func>
struct call_back : public call_back_base {
    Func _f;
    call_back(Func&& f) : _f(std::move(f)) {};
    void call() override {
        _f();
    }
    operator std::string() const override {
        return static_cast<std::string>(_f);
    }
};

struct function {
    using task_base_ptr_t = std::unique_ptr<call_back_base>;
    task_base_ptr_t _task;

    template <typename Func>
    void set_task(Func&& task_) {
        _task = std::make_unique<call_back<Func>>(std::forward<Func>(task_));
    }

    void operator()() {
        _task->call();
    }

    explicit operator std::string() const {
        return static_cast<std::string>(*_task);
    }
};
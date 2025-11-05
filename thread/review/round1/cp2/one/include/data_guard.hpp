#include <string>

#include "call_back.hpp"

class data_guard_base {
    using function = ::function;
    using self_t = data_guard_base;

   public:
    data_guard_base() = default;

    template <typename InputFunc, typename OutputFunc>
    data_guard_base(InputFunc&& in_func, OutputFunc** out_func)
        : _input(std::forward<InputFunc>(in_func)), _output(std::forward<OutputFunc>(out_func)) {}

    virtual ~data_guard_base() = default;

    // 如果数据可以执行
    virtual void operator()() = 0;

    // 如果数据可以被转化为字符串
    virtual explicit operator std::string() = 0;

    // 入数据时执行什么样的方法
    void input_func() {
        _input();
    }

    // 出数据时执行什么样的方法
    void output_func() {
        _output();
    }

    // 重新调整入方法
    template <typename Func>
    void reset_input_func(Func&& task_) {
        _input.reset(std::forward<Func>(task_));
    }

    // 重新调整出方法
    template <typename Func>
    void reset_output_func(Func&& task_) {
        _output.reset(std::forward<Func>(task_));
    }

    void swap(self_t& that) {
        _input.swap(that._input);
        _output.swap(that._output);
    }

    // 禁止赋值
    self_t& operator=(const self_t& that) = delete;

    // 允许移动赋值
    self_t& operator=(self_t&& that) {
        if (this != &that) {
            this->swap(that);
        }
        return *this;
    }

   private:
    function _input = []() {};
    function _output = []() {};
};

template <typename T>
class data_guard : public data_guard_base {
    using data_t = T;
    using self_t = data_guard<data_t>;
    using data_ptr_t = std::unique_ptr<data_t>;

   public:
    data_guard() = default;

    template <typename... Args>
    data_guard(Args&&... args) : _data(std::make_unique<data_t>(std::forward<Args>(args)...)) {}

    template <typename InputFunc, typename OutputFunc, typename... Args>
    data_guard(InputFunc&& in_func, OutputFunc&& out_func, Args&&... args)
        : data_guard_base(std::forward<InputFunc>(in_func), std::forward<OutputFunc>(out_func)),
          _data(std::make_unique<data_t>(std::forward<Args>(args)...)) {}

    self_t& operator=(const self_t& that) = delete;
    self_t& operator=(self_t&& that) {
        data_guard_base::operator=(that);
        _data = std::move(that._data);
        return *this;
    }

    template <typename... Args>
    void input(Args&&... args) {
        data_guard_base::input_func();
        _data = std::make_unique<data_t>(std::forward<Args>(args)...);
    }

    data_ptr_t output()
    {
        data_guard_base::output_func();
        return std::move(_data);
    }

    ~data_guard() override = default;

    explicit operator std::string() override {
        return static_cast<std::string>(*_data);
    }

   private:
    data_ptr_t _data;
};
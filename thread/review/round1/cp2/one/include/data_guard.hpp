#include <memory>

#include "move_only_function.hpp"

template <typename T>
class data_guard {
    using data_t = T;
    using self_t = data_guard<data_t>;

   public:
    using function_t = move_only_function<void(const data_t&)>;
    using data_ptr_t = std::unique_ptr<data_t>;

    data_guard() = default;

    template <typename InputFunc, typename OutputFunc>
    data_guard(
        InputFunc&& in_func = [](const data_t&) {}, OutputFunc&& out_func = [](const data_t&) {})
        : _input(std::forward<InputFunc>(in_func)), _output(std::forward<OutputFunc>(out_func)) {}

    template <typename... Args, typename InputFunc, typename OutputFunc>
    data_guard(
        Args&&... args, InputFunc&& in_func = [](const data_t&) {},
        OutputFunc&& out_func = [](const data_t&) {})
        : _data(std::make_unique<data_t>(std::forward<Args>(args)...)),
          _input(std::forward<InputFunc>(in_func)),
          _output(std::forward<OutputFunc>(out_func)) {}

    ~data_guard() = default;

    template <typename... Args>
    void input(Args&&... args) {
        auto _new_data = std::make_unique<data_t>(std::forward<Args>(args)...);
        _input(*_new_data);
        _data = std::move(_new_data);
    }

    data_ptr_t output() {
        _output(*_data);
        return std::move(_data);
    }

    // 重新调整入方法
    void reset_input_func(function_t&& task_) {
        _input = std::move(task_);
    }

    // 重新调整出方法
    void reset_output_func(function_t&& task_) {
        _output = std::move(task_);
    }

    // 禁止赋值
    self_t& operator=(const self_t& that) = delete;

    // 允许移动赋值
    self_t& operator=(self_t&& that) {
        _data = std::move(that._data);
        _input = std::move(that._input);
        _output = std::move(that._output);
        return *this;
    }

   private:
    data_ptr_t _data;
    function_t _input;
    function_t _output;
};

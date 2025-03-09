#include <iostream>
#include <tuple>
#include <pthread.h>


// 此时我在语言层正好学到了C++11的模版参数包
// 由于我嫌线程行为函数传参方式太麻烦, 所以随手写的

namespace wind {
    // 参数包装器
    template<typename... Args>
    struct Data {
        std::tuple<Args...> args;
        Data(Args&&... a) : args(std::forward<Args>(a)...) {}
    };

    template<class Behavior, class ResultType = decltype(std::declval<Behavior>()())>
    class thread {
        pthread_t id;

        // 定义为静态成员函数, 防止this指针捣乱(this指针和C不兼容)
        template<typename... Args>
        static void* transit(void* p) {
            auto* thread_data = static_cast<Data<Args...>*>(p);
            ResultType* result = new ResultType;
            *result = std::apply([](auto&&... args) {
                return Behavior()(std::forward<decltype(args)>(args)...);
            }, thread_data->args);
            delete thread_data;
            return result;
        }

    public:
        template<typename... Args>
        thread(Args&&... args) {
            auto* thread_data = new Data<Args...>(std::forward<Args>(args)...);
            pthread_create(&id, nullptr, transit<Args...>, thread_data);
        }

        ResultType join() {
            void* result_ptr;
            pthread_join(id, &result_ptr);
            ResultType* result = static_cast<ResultType*>(result_ptr);
            ResultType ret = std::move(*result);
            delete result;
            return ret;
        }
    };

    // 抽象基类，作为线程行为的接口
    template<typename T>
    struct routine {
        virtual ~routine() = default; // 虚析构函数，确保正确清理
        virtual T operator()() { return T(); } // 默认无参行为，可选重写
        virtual T operator()(Args&&... args) = 0; // 纯虚函数，用户必须实现
    };
}
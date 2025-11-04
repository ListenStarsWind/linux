#include <format>
#include <random>
#include <string>

#include "blocking_queue.hpp"
#include "call_back.hpp"
#include "exprtk.hpp"  // https://github.com/ArashPartow/exprtk

struct task {
    using value_t = double;
    using symbol_table_t = exprtk::symbol_table<value_t>;
    using parser_t = exprtk::parser<value_t>;
    using expression_t = exprtk::expression<value_t>;
    using string = std::string;

    task(std::string expression) : _flag(false), _expression(expression) {};

    string operator()() {
        // 表示进行过计算过程
        _flag = true;

        // 将符号表注册到表达式对象
        expression_t expression;
        expression.register_symbol_table(_symbol_table);

        // 编译表达式
        if (!_parser.compile(_expression, expression)) {
            _error = std::format("编译错误: {}\n", _parser.error());
            return {};
        }

        // 求值
        value_t result = expression.value();
        return _result = std::to_string(result) + "\n";
    }

    explicit operator string() const {
        if (_flag) {
            if (_error.empty()) {
                return _expression + "=" + _result;
            } else {
                return _error;
            }
        } else {
            return _expression;
        }
    }

    bool _flag;
    string _expression;
    string _error;
    string _result;

    inline static symbol_table_t _symbol_table;
    inline static parser_t _parser;
};

struct user_task {
    task _t;
    user_task(const task& t) : _t(t) {}
    explicit operator std::string() const {
        return static_cast<std::string>(_t);
    }
};

using queue_t = blocking_queue<::function>;

void* producer(void* p) {
    auto queue = static_cast<queue_t*>(p);

    static thread_local std::mt19937_64 gen(std::random_device{}());
    static thread_local std::uniform_real_distribution<double> value_dist(-1e150, 1e150);
    static thread_local std::uniform_int_distribution<int> opid_dist(0, 3);
    static thread_local const char ops[] = {'+', '-', '*', '/'};

    while (true) {
        // 生成两个随机数和一个运算符
        std::string left = std::to_string(value_dist(gen));
        std::string right = std::to_string(value_dist(gen));
        char op = ops[opid_dist(gen)];
        task t(left + op + right);

        auto task = [t = std::move(t)]() mutable {
            t();
            std::cout << static_cast<std::string>(t);
        };

        queue->emplace(task);
    }
    return nullptr;
}

void* consumer(void* p) {
    auto queue = static_cast<queue_t*>(p);
    while (true) {
        auto t = queue->front();
        t();
    }
    return nullptr;
}

int main() {
    pthread_t p, c;
    auto queue = std::make_unique<queue_t>();
    pthread_create(&c, nullptr, consumer, queue.get());
    pthread_create(&p, nullptr, producer, queue.get());

    pthread_join(c, nullptr);
    pthread_join(p, nullptr);

    return 0;
}
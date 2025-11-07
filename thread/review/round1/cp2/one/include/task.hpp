#pragma once

#include "exprtk.hpp"

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
            // _error = (boost::format("编译错误: %1%\n") % _parser.error()).str();
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

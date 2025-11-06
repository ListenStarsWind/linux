#pragma once

#include <utility>

template <typename Sig>
class move_only_function;

// 模版偏特化
// 将特定模式的类型进行解构
template <typename R, typename... Args>
class move_only_function<R(Args...)> {
    // 对于 R(Args...)有两种解读, 它们本质上都是同一种描述
    // 你知道, 世界上总有一些东西是无法用语言完整表述的
    // 1. R(Args...) 是函数类型
    // 它描述了特定结构的函数, 例如对于形如void Fync(int)
    // 的函数来说, Func的类型就是 void(int)
    // 但是R(Args...)相比其他类型, 有一些奇怪的行为
    // 比如, 不能直接实例化或者赋值
    // 这是因为, R(Args...)所描述的对象群体: 函数
    // 本身行为就和一般对象很不一样, 函数它是可执行的
    // 是在代码段的, 是main函数开始前就已经准备好的
    // 为了保护函数, 函数本身是静态的, 无法进行改变
    // 所谓的函数指针本质上不是改变函数, 而是引用某个函数
    // 而且他也不是函数, 是函数指针,所以说 R(Args...) 类型
    // 的奇怪不是因为这个类型本身奇怪, 而是函数这种对象本身
    // 比较特殊
    // 2. R(Args...) 是模式匹配符, 不是类型
    // R(Args...)不是真正的类型, 它只是对函数指针的一种封装
    // 在底层, 它最终会被编译器解析成函数指针来理解
    // 它只是换了一种跟简单的方式来表达和描述函数的形式

    // 手动实现虚表
    // 其本质是创了一个结构体, 只不过这个结构体里面全是函数指针
    // 这些函数指针描述了底层对象--在这里是函数对象的 操作方法
    // 一种C风格的东西, 好处是自由度更高, 可以定制出跟灵活的东西
    struct vtable_t {
        // 其中的泛型指针 void* 实际上指的就是函数对象的指针
        R (*invoke)(void*, Args...);  // 调用底层的执行方法 (可能需要更多参数的支持)
        void (*destroy)(void*);       // 析构底层执行方法中的资源
    };

    // 针对具体函数类型 F 定义特定的操作方法实现
    template <typename F>
    static vtable_t make_vtable() {
        return vtable_t{
            // 把指向方法的泛型指针强转解引用变为可执行方法调用
            [](void* obj, Args... args) -> R {
                return (*reinterpret_cast<F*>(obj))(std::forward<Args>(args)...);
            },
            // 复用函数对象本身的析构方法, 或许它是具有可执行属性的普通类型
            [](void* obj) {
                delete static_cast<F*>(obj);  // 析构底层方法 (万能引用那里的对象都是new出来的)
            }
        };
    }

    void* _obj = nullptr;         // 底层的函数对象指针
    vtable_t* _vtable = nullptr;  // 虚表指针

    using self_t = move_only_function<R(Args...)>;

   public:
    move_only_function() = default;

    // 将某个底层可执行对象拷贝或者移动过来
    // 具体是谁, 要看用户使用情况, 因为这里
    // 实际上是完美引用
    template <typename F>
    move_only_function(F&& f) {
        // 相同类型使用同一个虚表
        static vtable_t vt = make_vtable<typename std::decay<F>::type>();
        _obj = new typename std::decay<F>::type(std::forward<F>(f));
        _vtable = &vt;
    }

    // 把别人的底层对象拿过来
    // noexcept 保证不抛出异常
    // STL 规定 声明noexcept时插入
    // 右值对象将使用移动构造, 否则用拷贝
    move_only_function(self_t&& other) noexcept {
        if (this == &other) return;

        // 把资源拿过来
        _obj = other._obj;
        _vtable = other._vtable;

        // 置空对方
        other._obj = nullptr;
        other._vtable = nullptr;
    }

    ~move_only_function() {
        clear();
    }

    // 调用
    R operator()(Args... args) const {
        return _vtable->invoke(_obj, std::forward<Args>(args)...);
    }

    // 是否有函数
    explicit operator bool() const {
        return _obj != nullptr;
    }

    // 移动赋值
    self_t& operator=(self_t&& other) noexcept {
        if (this != &other) {
            clear();
            _obj = other._obj;
            _vtable = other._vtable;
            other._obj = nullptr;
            other._vtable = nullptr;
        }
        return *this;
    }

   private:
    void clear() {
        if (_obj && _vtable) {
            _vtable->destroy(_obj);  // 复用虚表析构底层对象
        }
        _obj = nullptr;
        _vtable = nullptr;
    }
};
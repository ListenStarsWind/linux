// 这是一个基于 LLVM 项目中 move_only_function 的模拟实现
// 它的目的是为了解决原 std::function中 对于不可拷贝函数对象的不友好问题
// 我经常用到很多 unique_ptr
// 结论, 日常还是应该用std::function, 因为我发现 move_only_function会有更多奇怪的现象

#include <memory>       // 引入智能指针
#include <type_traits>  // 引入检查和处理类型的工具

// 这是一个抽象的接口基类, 它的职责是描述一个可执行对象的接口
// 主要包括调用参数和返回值这两类
// Generic: 通用, 泛型; Callable：可调用的, 可执行的
template <typename RetT, typename... ArgTs>
class GenericCallable {
   public:
    virtual ~GenericCallable() = default;    // 我们知道, 实现多态析构函数必须要是虚函数
    virtual RetT call(ArgTs&&... Args) = 0;  // 对执行方法的简要包装
};

// 这是一个具体的功能实现类, 其目的是采用模版的方式将底层执行对象以unique_ptr的形式进行存储,
// 并实现上面的 GenericCallable 接口 Impl：Implementation 的缩写，即“实现”
template <typename CallableT, typename RetT, typename... ArgTs>
class GenericCallableImpl : public GenericCallable<RetT, ArgTs...> {
   public:
    template <typename CallableInitT>
    GenericCallableImpl(CallableInitT&& Callable)
        : Callable(std::forward<CallableInitT>(Callable)) {}

    RetT call(ArgTs&&... Args) override {
        // void时编译器会自动忽略return
        return Callable(std::forward<ArgTs>(Args)...);
    }

   private:
    // 底层可执行对象将会以自己原有的类型(加智能指针)的形式进行保存
    CallableT Callable;  // 实际保存的“功能对象”。
};

template <typename FnT>
class move_only_function;

// 模版偏特化
// 将特定模式的类型进行解构
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
template <typename RetT, typename... ArgTs>
class move_only_function<RetT(ArgTs...)> {
private:
  using GenericCallable = GenericCallable<RetT, ArgTs...>;
  using GenericCallablePtr = std::unique_ptr<GenericCallable>;
  // 简化名字，方便使用，指的是上面的具体“功能”实现类。

  // 模版类型简化和省略
  template <typename CallableT>
  using GenericCallableImpl = GenericCallableImpl<CallableT, RetT, ArgTs...>;

public:
  move_only_function() = default;   // 默认构造函数（创建一个空的功能对象）。
  move_only_function(std::nullptr_t) {} // 可以用空指针来构造（也创建一个空的功能对象）。
  move_only_function(move_only_function &&) = default; // **允许移动**：可以把里面的“功能”对象转移给另一个 move_only_function。
  move_only_function(const move_only_function &) = delete; // **禁止复制**：这个对象只能被“移动”，不能被“复制”。
  move_only_function &operator=(move_only_function &&) = default; // **允许移动赋值**。
  move_only_function &operator=(const move_only_function &) = delete; // **禁止复制赋值**。

  // 构造函数：接受一个任意类型的“功能对象”并包装起来。
  template <typename CallableT>
  move_only_function(CallableT &&Callable)
      // 使用智能指针（std::unique_ptr）以可执行对象原本的类型进行存储管理
      : C(std::make_unique<GenericCallableImpl<std::decay_t<CallableT>>>(
               std::forward<CallableT>(Callable))) {}

  // **重载圆括号操作符 (())**：让这个对象看起来像一个真正的函数，可以直接调用
  RetT operator()(ArgTs... Params) const {
    // void 时编译器自动忽略return
    return C->call(std::forward<ArgTs>(Params)...);
  }

  // **显式类型转换操作符 (bool)**：可以判断这个对象是否真的存着一个“功能”。
  // 尽管智能指针可以转化为布尔值, 但这样写是为了得到绝对纯净标准的布尔值
  // 在视觉上, 也能向人强调, 我这里要的是一个布尔值, 而不是什么指针
  explicit operator bool() const { return !!C; } // 检查智能指针 C 是否非空。

private:
  // 使用智能指针 C 来存放和管理被包装起来的“功能”接口对象。
   GenericCallablePtr C;
};

// 下面都是针对可执行对象属性为const的实现

template <typename RetT, typename... ArgTs>
class GenericConstCallable {
   public:
    virtual ~GenericConstCallable() = default;   
    virtual RetT call(ArgTs&&... Args) const = 0;  
};

template <typename CallableT, typename RetT, typename... ArgTs>
class GenericConstCallableImpl : public GenericConstCallable<RetT, ArgTs...> {
   public:
    template <typename CallableInitT>
    GenericConstCallableImpl(CallableInitT&& Callable)
        : Callable(std::forward<CallableInitT>(Callable)) {}

    RetT call(ArgTs&&... Args) const override {
        return Callable(std::forward<ArgTs>(Args)...);
    }

   private:
    CallableT Callable; 
};

template <typename RetT, typename... ArgTs>
class move_only_function<RetT(ArgTs...)const> {
private:
  using GenericCallable = GenericConstCallable<RetT, ArgTs...>;
  using GenericCallablePtr = std::unique_ptr<const GenericCallable>;

  // 模版类型简化和省略
  template <typename CallableT>
  using GenericCallableImpl = GenericConstCallableImpl<CallableT, RetT, ArgTs...>;

public:
  move_only_function() = default;   
  move_only_function(std::nullptr_t) {} 
  move_only_function(move_only_function &&) = default; 
  move_only_function(const move_only_function &) = delete; 
  move_only_function &operator=(move_only_function &&) = default; 
  move_only_function &operator=(const move_only_function &) = delete; 

  template <typename CallableT>
  move_only_function(CallableT &&Callable)
      : C(std::make_unique<const GenericCallableImpl<std::decay_t<CallableT>>>(
               std::forward<CallableT>(Callable))) {}

  RetT operator()(ArgTs... Params) const {
    return C->call(std::forward<ArgTs>(Params)...);
  }

  explicit operator bool() const { return !!C; } 

private:
   GenericCallablePtr C;
};

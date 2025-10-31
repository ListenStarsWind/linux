#include "my_thread.hpp"
#include <iostream>
#include <string>
#include <chrono> // 包含 chrono 头文件
#include <thread>
#include <unistd.h> // For sleep, although std::this_thread::sleep_for is better
#include <sys/syscall.h> // For syscall(SYS_gettid)

// 移除 using namespace std::chrono_literals;

// 1. 测试带返回值的线程 (Result_t = int)
int calculate_sum(int a, int b) {
    // 使用 gettid 获取当前线程的 LWPID
    std::cout << "子线程 [" << syscall(SYS_gettid) << "]：开始计算 " << a << " + " << b << "..." << std::endl;
    // 修复：使用 std::chrono::milliseconds(100) 替换 100ms
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    int result = a + b;
    std::cout << "子线程 [" << syscall(SYS_gettid) << "]：计算完成，结果为 " << result << std::endl;
    return result;
}

// 2. 测试无返回值的线程 (Result_t = void)
void print_message(const std::string& msg) {
    std::cout << "子线程 [" << syscall(SYS_gettid) << "]：正在处理消息: " << msg << std::endl;
    // 修复：使用 std::chrono::milliseconds(50) 替换 50ms
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    std::cout << "子线程 [" << syscall(SYS_gettid) << "]：消息处理完毕。" << std::endl;
}

int main() {
    std::cout << "主线程 [" << syscall(SYS_gettid) << "]：启动自定义线程测试..." << std::endl;

    // =========================================================
    // 1. 测试 myThread<int>
    // =========================================================
    try {
        std::cout << "\n--- 测试带返回值线程 (myThread<int>) ---" << std::endl;
        
        // 使用 create 启动线程，传递参数
        auto thread_int = myThread<int>::create(calculate_sum, 10, 20);
        
        if (thread_int) {
            std::cout << "主线程：线程已启动。LWPID (存储在 tid 中): " << thread_int->get_tid() << std::endl;
            
            // 等待线程结束并获取结果
            auto result_ptr = thread_int->join();
            
            if (result_ptr) {
                std::cout << "主线程：线程汇合成功，计算结果为: " << *result_ptr << std::endl;
            } else {
                std::cout << "主线程：线程汇合成功，但结果指针为空（可能发生异常）。" << std::endl;
            }
        } else {
            std::cerr << "主线程：错误，无法创建 myThread<int> 实例。" << std::endl;
        }
    } catch (const std::exception& e) {
        std::cerr << "异常捕获: " << e.what() << std::endl;
    }


    // =========================================================
    // 2. 测试 myThread<void>
    // =========================================================
    try {
        std::cout << "\n--- 测试无返回值线程 (myThread<void>) ---" << std::endl;

        // 使用 create 启动线程，传递字符串参数
        auto thread_void = myThread<void>::create(print_message, std::string("Hello Custom Thread!"));

        if (thread_void) {
            // 验证 tid 字段在特化版本中也被成功利用和赋值
            std::cout << "主线程：线程已启动。LWPID (通过委托赋值): " << thread_void->get_tid() << std::endl;

            // 等待线程结束 (void::join)
            thread_void->join();

            std::cout << "主线程：void 线程汇合成功。" << std::endl;
        } else {
            std::cerr << "主线程：错误，无法创建 myThread<void> 实例。" << std::endl;
        }
    } catch (const std::exception& e) {
        std::cerr << "异常捕获: " << e.what() << std::endl;
    }
    
    std::cout << "\n主线程 [" << syscall(SYS_gettid) << "]：所有测试完成，程序正常退出。" << std::endl;

    return 0;
}

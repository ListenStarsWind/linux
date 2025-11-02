#include <cassert>
#include <csignal>
#include <iostream>
#include <memory>
#include <thread>
#include <vector>

#include "clone_pthread.hpp"

using namespace std::chrono_literals;

// ========================================
// 1. 基本功能
// ========================================
void test_basic() {
    std::cout << "=== Test 1: 基本功能 ===\n";

    auto t1 = myThread<int>::create([] {
        std::this_thread::sleep_for(10ms);
        return 123;
    });
    assert(t1);
    auto res = t1->join();
    assert(res && *res == 123);
    std::cout << "  有返回值: " << *res << "\n";

    auto t2 = myThread<void>::create([] { std::cout << "  [void] Hello!\n"; });
    assert(t2);
    t2->join();
    std::cout << "  void 线程 join 成功\n";
}

// ========================================
// 2. 移动语义
// ========================================
void test_move() {
    std::cout << "=== Test 2: 移动语义 ===\n";
    auto ptr = std::make_unique<int>(777);

    auto t = myThread<void>::create([p = std::move(ptr)]() mutable {
        assert(p && *p == 777);
        std::cout << "  移动值: " << *p << "\n";
    });
    assert(t);
    t->join();
    assert(ptr == nullptr);
    std::cout << "  移动语义 OK\n";
}

// ========================================
// 3. 异常安全
// ========================================
void test_exception() {
    std::cout << "=== Test 3: 异常安全 ===\n";
    auto t = myThread<int>::create([] {
        throw std::runtime_error("test");
        return 0;
    });
    assert(t);
    auto res = t->join();
    assert(!res);
    std::cout << "  异常被捕获，join 返回 nullptr\n";
}

// ========================================
// 4. 小规模并发（仅 4 线程，2 核安全）
// ========================================
void test_concurrency() {
    std::cout << "=== Test 4: 小并发（4 线程）===\n";
    std::vector<myThread<int>::self_ptr_t> threads;
    std::atomic<int> sum{0};

    for (int i = 0; i < 4; ++i) {
        threads.push_back(myThread<int>::create([i, &sum] {
            sum += i;
            return i * 10;
        }));
    }

    int total = 0;
    for (auto& t : threads) {
        auto res = t->join();
        assert(res);
        total += *res;
    }

    assert(sum == (0 + 1 + 2 + 3));
    assert(total == (0 + 10 + 20 + 30));
    std::cout << "  4 线程正确: sum=" << sum << ", total=" << total << "\n";
}

// ========================================
// 5. 栈溢出保护（轻量）
// ========================================
static volatile sig_atomic_t g_segv = 0;

void test_stack_guard() {
    std::cout << "=== Test 5: 栈溢出保护 ===\n";
    std::signal(SIGSEGV, [](int) { g_segv = true; });

    auto t = myThread<void>::create([] {
        auto bomb = [](auto&& self, int n) -> void {
            char x[2048] = {0};
            (void)x;
            if (n > 1000) return;  // 2MB 内必炸
            self(self, n + 1);
        };
        bomb(bomb, 0);
    });

    assert(t);
    t->join();
    std::this_thread::sleep_for(50ms);
    assert(g_segv);
    std::cout << "  守护页触发 SIGSEGV，保护生效！\n";
    std::signal(SIGSEGV, SIG_DFL);
}

// ========================================
// 主函数
// ========================================
int main() {
    std::cout << "myThread 2核安全测试开始...\n\n";
    try {
        test_basic();
        test_move();
        test_exception();
        test_concurrency();
        test_stack_guard();
        std::cout << "\n所有测试通过！2核系统安全运行\n";
    } catch (...) {
        std::cerr << "测试异常！\n";
        return 1;
    }
    return 0;
}

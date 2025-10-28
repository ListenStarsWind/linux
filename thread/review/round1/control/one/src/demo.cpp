#include <pthread.h>
#include <unistd.h>

#include <iostream>

using namespace std;

void* handler(void* arg) {
    auto end = *static_cast<int*>(arg);
    auto result = new int(0);
    for (int i = 0; i <= end; ++i) {
        *result += i;
    }
    return static_cast<void*>(result);
}

int main() {
    pthread_t tid;
    int end = 100;
    pthread_create(&tid, nullptr, handler, &end);
    int* result = nullptr;
    pthread_join(tid, reinterpret_cast<void**>(&result));
    cout << *result << endl;
    delete result;
    return 0;
}

// #include <pthread.h>  // 线程库头文件
// #include <unistd.h>

// #include <iostream>

// using namespace std;

// // 全局变量 count, 负责计数循环次数
// int count = 0;

// void* thread_behavior(void* arg) {
//     auto& text = *static_cast<string*>(arg);
//     while (true) {
//         cout << text << " " << count << endl;
//         sleep(1);
//     }
// }

// int main() {
//     pthread_t tid;

//     // 在堆上创建一个字符串存储打印文本
//     auto text = new string();

//     // 将其传入副线程的执行逻辑中
//     pthread_create(&tid, nullptr, thread_behavior, text);

//     while (true) {
//         // 以按位与的形式抹除掉除最低位和次低位之外的数字, 这样 flag 的范围始终都是 0 ~ 3
//         int flag = count & 3;
//         switch (flag) {
//             case 0:
//                 text->operator=("因为太阳将要毁伤");
//                 break;
//             case 1:
//                 text->operator=("因为月亮将要坠落");
//                 break;
//             case 2:
//                 text->operator=("因为我们从未分离");
//                 break;
//             case 3:
//                 text->operator=("因为我们终将重逢");
//                 break;
//         }
//         count++;
//         sleep(1);
//     }

//     return 0;
// }
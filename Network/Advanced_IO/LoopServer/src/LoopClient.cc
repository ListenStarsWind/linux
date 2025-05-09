#include "Protocols.hpp"
#include "Sockst.hpp"
#include <ctime>
#include <iostream>
#include <string>

using namespace std;

int main()
{
    socket_ sockfd;
    sockfd.create_();
    sockfd.connect_(8888, "175.24.175.224");

    srand(time(nullptr));
    const int count = 5;
    const string ops("+-*/%&");

    // 统一输出格式
    cout << "=========================== 测试开始 ===========================\n" << endl;

    for (int i = 0; i < count; ++i)
    {
        cout << "------------------------ 测试 " << i + 1 << " ------------------------" << endl;

        // 生成随机数和运算符
        int x = rand() % 100;
        int y = rand() % 100;
        char op = ops[rand() % ops.size()];

        // 创建表达式并打印
        expression temp(x, op, y);
        cout << "运算表达式: ";
        temp.print();

        // 打包消息并发送
        auto message = pack(temp);
        // message += message;
        cout << "发出报文: " << endl;
        cout << "-------------------------- 原始报文 --------------------------" << endl;
        cout << message << endl;

        ssize_t len = 0;
        while (true)
        {
            len += write(sockfd, message.c_str() + len, message.size() - len);
            if (static_cast<size_t>(len) == message.size()) break;
        }

        // 读取返回结果
        char buffer[1024];
        len = read(sockfd, buffer, sizeof(buffer) - 1);
        buffer[len] = 0;
        string out_stream(buffer);
        string end;
        unpack(out_stream, &end);
        result r(end);

        // 打印结果
        cout << "------------------------- 计算结果 --------------------------" << endl;
        r.print();
        cout << "============================== 完成 =============================" << endl;
        cout << endl;

        usleep(400000);  // 休眠0.4秒
    }
    sockfd.close_();
    cout << "=========================== 测试结束 ===========================\n";
    return 0;
}
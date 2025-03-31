#include"TcpClient.hpp"
#include<ctime>

using namespace std;

int main()
{
    socket_ sockfd;
    sockfd.create_();
    sockfd.connect_(8888, "120.55.90.240");

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

// int main()
// {
//     socket_ sockfd;
//     sockfd.create_();
//     sockfd.connect_(8888, "120.55.90.240");

//     srand(time(nullptr));
//     const int count = 5;
//     const string ops("+-*/%&");
//     for(int i = 0; i < count; ++i)
//     {
//         cout << "测试: "<<i+1<<endl;
//         int x = rand() % 100;
//         int y = rand() % 100;
//         char op = ops[rand() % ops.size()];
//         expression temp(x, op, y);
//         temp.print();
//         auto message = pack(temp);
//         cout << "发出一份报文↓"<<endl;
//         cout << message << endl;

//         ssize_t len = 0;
//         // 防止客户端缓冲区放不下大字符串, 而写缺失, 对于本项目, 实际不可能有这种情况
//         while(true)
//         {
//           len += write(sockfd, message.c_str() + len, message.size() - len);
//          if(static_cast<size_t>(len) == message.size()) break;
//         }

//         // 这里我们也不一定读到完整报文  但为了图省事就不这么严谨了, 默认成功
//         char buffer[1024];
//         len = read(sockfd, buffer, sizeof(buffer) - 1);
//         buffer[len] = 0;
//         string out_stream(buffer);
//         string end;
//         unpack(out_stream, &end);
//         result r(end);
//         r.print();
//         cout <<"=============="<<endl;
//         cout << endl;

//         usleep(400000);  // 休眠0.4秒
//     }

//     sockfd.close_();
//     return 0;
// }
#include"Protocols.hpp"
#include<iostream>
#include<jsoncpp/json/json.h>

using namespace std;

int main()
{
    Json::Value set;
    set["t"] = "hello";
    set["t"] = "hello";

    Json::Value root;
    root["x"] = 100;
    root["y"] = 200;
    root["op"] = "+";
    root["desc"] = "this ia a + oper";
    root["te"] = set;

    Json::StyledWriter w;
    // Json::FastWriter w;
    string res = w.write(root);
    cout << res;

    Json::Value v;
    Json::Reader r;
    r.parse(res, v);

    int x = v["x"].asInt();
    int y = v["y"].asInt();
    string op = v["op"].asString();
    std::string s = v["desc"].asString();
    Json::Value temp = v["te"];
    string test = temp["t"].asString();

    cout << x <<endl;
    cout << y <<endl;
    cout << op <<endl;
    cout << s <<endl;
    cout << test << endl;

    return 0;
}

// int main()
// {
//     expression e(100, '+', 300);
//     result r(400, 0);
//     auto e_str_in = pack(e);
//     auto r_str_in = pack(r);
//     auto in = e_str_in + r_str_in; // 模拟多个报文相连的情况

//     // 网络传输

//     string e_str_out, r_str_out;
//     expression e_; result r_;
//     if(unpack(in, &e_str_out))
//     {
//        e_.init(e_str_out);
//     }
//     if(unpack(in, &r_str_out))
//     {
//         r_.init(r_str_out);
//     }

//     if(e_) e_.print();
//     if(r_) r_.print();

//     return 0;
// }

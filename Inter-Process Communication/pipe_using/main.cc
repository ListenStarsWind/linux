#include<vector>
#include<string>
#include"taskpool.hpp"
#include"tasks.hpp"

using namespace std;

typedef void(*func)();

int main()
{
    // vector<func> tasks = {f1, f2, f3, f4};
    vector<func> tasks = {f, f, f, f};
    wind::taskpool t(tasks);
    int i = 5;
    int j = 0;
    while(i--)
    {
        t.call(j);
        j++;
        if(j == tasks.size())
        {
            j = 0;
        }
    }
    return 0;
}
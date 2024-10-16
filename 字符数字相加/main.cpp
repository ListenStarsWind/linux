#include<string>
#include <algorithm>
using std::string;


class Solution {
public:
    string addStrings(string num1, string num2) {
        int addend_num1 = 0;
        int addend_num2 = 0;
        int sum = 0;
        int carry = 0;
        int current_num1 = num1.size() - 1;
        int current_num2 = num2.size() - 1;
        string ret;
        while (current_num1 >= 0 || current_num2 >= 0) {
            if (current_num1 >= 0)
                addend_num1 = num1[current_num1--] - '0';
            else
                addend_num1 = 0;
            if (current_num2 >= 0)
                addend_num2 = num2[current_num2--] - '0';
            else
                addend_num2 = 0;
            sum = addend_num1 + addend_num2 + carry;
            carry = sum / 10;
            sum = sum % 10;
            ret += (sum + '0');
        }
        if (carry != 0)
            ret += (carry + '0');
        reverse(ret.begin(), ret.end());
        return ret;
    }
};

int main()
{
    string s1("11");
    string s2("123");
    Solution().addStrings(s1, s2);
    return 0;
}
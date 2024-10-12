class Solution {
public:
    int firstUniqChar(string s) {
        int Count[26] = { 0 };
        for (auto ch : s) {
            Count[ch - 'a']++;
        }
        int i = 0;
        for (; i < s.size(); i++) {
            if (Count[s[i] - 'a'] == 1)
                return i;
        }
        return -1;
    }
};
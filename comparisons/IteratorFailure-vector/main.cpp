#include<iostream>
#include<string>
#include<vector>

// 演示迭代器外部失效
void realization4()
{
	std::vector<int> v;
	//wind::vector<int> v;
	v.push_back(0);
	v.push_back(1);
	v.push_back(2);
	v.push_back(2);
	v.push_back(3);
	v.push_back(3);
	//wind::vector<int>::iterator it = v.begin();
	std::vector<int>::iterator it = v.begin();
	// 目标：删除所有偶数元素
	while (it != v.end())
	{
		if (*it % 2 == 0)
		{
			v.erase(it);
		}
		++it;
	}
	for (auto val : v)
	{
		std::cout << val << " ";
	}
	std::cout << std::endl;
	// 输出结果：1  2  3  3
	// it -> 0    0 1 2 2 3 3   删除 迭代器指向内容发生改变 it -> 1  外部循环再加一 it -> 2
	// it -> 2    1 2 2 3 3     删除 迭代器指向内容发生改变 it -> 2  外部循环再加一 it -> 3
	// it -> 3    1 2 3 3       外部循环加一 it -> 3
	// it -> 3    1 2 3 3       外部循环加一 it -> 3
	// 解决方案： erase是有返回值的，可以用来修正外部迭代器
}

void realization5()
{
	std::vector<int> v;
	//wind::vector<int> v;
	v.push_back(1);
	v.push_back(1);
	v.push_back(1);
	v.push_back(2);
	//wind::vector<int>::iterator it = v.begin();
	std::vector<int>::iterator it = v.begin();
	// 目标：删除所有偶数元素
	while (it != v.end())
	{
		if (*it % 2 == 0)
		{
			v.erase(it);
		}
		++it;
	}
	for (auto val : v)
	{
		std::cout << val << " ";
	}
	std::cout << std::endl;
	// 结果：触发assert警告
	// 最后一个元素被删除之后，迭代器已经指向非法区域
	// 其值为随机值
	// 当为偶数时，再次进入erase，触发范围检查警告
}

// 解决方案：删除之后对迭代器进行修正
void realization6()
{
	std::vector<int> v;
	//wind::vector<int> v;
	v.push_back(0);
	v.push_back(1);
	v.push_back(2);
	v.push_back(3);
	v.push_back(4);
	v.push_back(4);
	//wind::vector<int>::iterator it = v.begin();
	std::vector<int>::iterator it = v.begin();
	// 目标：删除所有偶数元素
	while (it != v.end())
	{
		if (*it % 2 == 0)
		{
			it = v.erase(it);
		}
		else
		{
			++it;
		}
	}
	for (auto val : v)
	{
		std::cout << val << " ";
	}
	std::cout << std::endl;
}


// 刚刚使用的是我们自己实现的vector
// 现在换成VS自己的看一看
// 我们发现此时4 5号序列都报错
// VS对迭代器进行了强制检查
// 只要发现经过erase的迭代器
// 没有被修正，直接操作（比如++）就报错
// 待会看看能不能演示insert，看看效果

// g++则是更像我们实现的逻辑
// 注意：若使用了C++11的语法
// 需要编译时带上选项`-std=c++11`
int main()
{
realization6();
	return 0;
}

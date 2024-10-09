# 进度条

## 纲要

![f765ee5caab883cad42305126d5611e](https://md-wind.oss-cn-nanjing.aliyuncs.com/md/202410081845970.png)

整个项目我们主体采用面向对象的思想。设计一个类，可以存储项目的相关信息和方法，可以想到，可能会有多个对象，即多个事件的进度条，所以我们在进度条这个类里声明了一个用于存储事件名称的成员变量`_EventName`，那要用什么做初始命名呢？我用一个静态成员变量`_Count`去统计已经实例化的对象个数，并把它们的序列号作为事件的默认名，我个人觉得类`statisticians`比`ProgressBar`更重要。`statisticians`的主要职责是统计其它类的运行状态，它是一种基层信息的反馈机制，我个人觉得它完善之后肯定是很有用的。

```cpp
```ProgressBar.h
#pragma once

#include <iostream>
#include <string>
#include <unistd.h>
#include <time.h>
#include <stdio.h>

namespace wind
{
	class statisticians
	{
	public:
		statisticians();
		statisticians(const statisticians& obj);
		~statisticians();
		statisticians& operator=(const statisticians& obj);
		bool IsOverflow()const;
		statisticians& operator++();
		long Using();
	private:
		int _using;
		int _incorrect;
	};


	class ProgressBar
	{
	public:
		ProgressBar();
		ProgressBar(const ProgressBar& obj);
		~ProgressBar();
		void print(int(*Progress)());
		std::string InitName();
	private:
		std::string _EventName;
		char _Body;
		char _Push;
		int _Refresh;
		static statisticians _Count;
	};
}



```ProgressBar.cpp
#include"ProgressBar.h"

wind::statisticians wind::ProgressBar::_Count;

wind::ProgressBar::ProgressBar()
	:_EventName(InitName())
	,_Body('-')
	,_Push('>')
	, _Refresh(165)
{
	++_Count;
	std::cout << "You can name this event: Yes or No?" << std::endl;
	std::string response;
	std::getline(std::cin, response);

	if (response == "Yes" || response == "yes" || response == "y"  || response == "Y") 
	{
		std::cout << "Please enter the event name: ";
		std::getline(std::cin, _EventName); 
	}
	else
	{
		std::cout << "Default naming has been used: " << _EventName << std::endl;
	}
}

wind::statisticians::statisticians()
	:_using(1)
	,_incorrect(1)
{}

wind::statisticians::statisticians(const statisticians& obj)
	:_using(obj._using)
    ,_incorrect(obj._incorrect)
{}

wind::statisticians::~statisticians()
{}

wind::statisticians& wind::statisticians::operator=(const statisticians& obj)
{
	_using = obj._using;
	_incorrect = obj._incorrect;
}

bool wind::statisticians::IsOverflow()const
{
	if (_incorrect != 1)
		return true;
	else
		return false;
}

wind::statisticians& wind::statisticians::operator++()
{
	_using++;
	if (_using == 0)
	{
		++_incorrect;
		std::cout << "Warning: the variable has overflowed,		\
					the overflow counter is enabled, and		\
					the overflow counter's current data is"		\
			<< _incorrect << std::endl;
	}
	return *this;
}

long wind::statisticians::Using()
{
	if (IsOverflow())
		return (_using - 1) * _incorrect;
	else
		return _using - 1;
}

std::string wind::ProgressBar::InitName()
{
	long num = _Count.Using();
	std::string str = std::to_string(num);
	return str;
}

wind::ProgressBar::ProgressBar(const ProgressBar& obj)
	:_EventName(InitName())
	, _Body('-')
	, _Push('>')
	, _Refresh(60)
{
	++_Count;
	std::cout << "You can name this event: Yes or No?" << std::endl;
	std::string response;
	std::getline(std::cin, response);

	if (response == "Yes" || response == "yes" || response == "y" || response == "Y")
	{
		std::cout << "Please enter the event name: ";
		std::getline(std::cin, _EventName);
	}
	else
	{
		std::cout << "Default naming has been used: " << _EventName << std::endl;
	}
}

wind::ProgressBar::~ProgressBar()
{}

void wind::ProgressBar::print(int(*Progress)())
{
	while (1)
	{
		clock_t start = clock();
		int i = Progress();
		std::string str;
		str.push_back('[');
		int j = 0;
		for (j = 0; j < i; j++)
		{
			str.push_back(_Body);
		}
		if (j < 99)
			str.push_back(_Push);
		else
			str.push_back(_Body);
		for (; j < 100; j++)
		{
			str.push_back(' ');
		}
		str.push_back(']');
		str.push_back('[');
		std::string s = std::to_string(i);
		str += s;
		str.push_back('%');
		str.push_back(']');
		str.push_back('\r');
		int k = 1000000 / _Refresh;
		std::cout << str;
		fflush(stdout);
		clock_t end = clock();
		usleep(k - (end - start));
		if (i == 100)
			break;
	}
	std::cout << std::endl;
}

```test.cpp
#include"ProgressBar.h"


int f()
{
	static int i = -1;
	++i;
	return i;
}

void ProgressBarTest1()
{
	wind::ProgressBar i;
	i.print(f);
}

int main()
{
	ProgressBarTest1();
	return 0;
}    
```

我写完觉得好像没什么好讲的，成员变量几乎都是使用初始化列表的方式定义的，可以优化一点，构造函数给点参数，提供更多字符选择方案，我在该项目下的其它文件里写了色块的实验程序，可以让进度条变成彩色的，不过我就不优化了，我觉得这个进度条再优化就是换换壳子了，所以我就不优化了。



# 完
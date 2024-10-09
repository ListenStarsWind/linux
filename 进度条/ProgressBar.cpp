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
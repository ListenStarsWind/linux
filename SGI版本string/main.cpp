#include<iostream>
#include<string>

void test4()
{
	std::string s;
  s.reserve(100);
	std::cout << "initial capacity:";
	size_t old = s.capacity();
	std::cout << old << std::endl;
	int cir = 0;
	for (; cir < 100; cir++)
	{
		s.push_back('c');
		size_t n = s.capacity();
		if (n != old)
		{
			std::cout << "trigger expansion,The new capacity is:";
			std::cout << n << std::endl;
			old = n;
		}
	}
}

int main()
{
	test4();
	return 0;
}

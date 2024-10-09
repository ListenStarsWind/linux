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

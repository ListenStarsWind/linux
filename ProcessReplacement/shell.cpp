#include"CentralControl.h"

#ifdef SHELL

#define USINGCPLUSPLUS

#ifdef USINGCPLUSPLUS

#include<vector>
#include<string>
#include<iostream>
#include<stdlib.h>
#include<unistd.h>
#include<sys/types.h>
#include<sys/wait.h>


#define BEGIN "["
#define END "]"

extern char** environ;

namespace wind
{
	class shell
	{
		typedef std::string working_directory;
		typedef std::string work_pwd;
		typedef std::vector<std::string> command;
		typedef std::vector<std::string> local_variable_pool;
		typedef std::vector<std::string> environment_variable_pool;
		typedef shell self;

		std::string get_relative_path()
		{
			size_t pos = _absolute_path.rfind('/');
			if (pos == 0)
			{
				if (_absolute_path.size() > 1)
					return _absolute_path.substr(pos + 1);
				else
					return "/";
			}
			else
			{
				return _absolute_path.substr(pos + 1);
			}
		}

		std::string get_prompt()
		{
			if (std::string(getenv("USER")) == std::string("root"))
				return "#";
			else
				return "$";
		}

		std::vector<std::string> command_extraction()
		{
			std::string tmp;
			std::vector<std::string> ret;
			getline(std::cin, tmp);
			auto it = tmp.begin();
			auto Do = it;
			auto Done = it;
			while (it != tmp.end())
			{
				if (*it == ' ')
				{
					Done = it;
					ret.push_back(std::string(Do, Done));
					++it;
					Do = it;
				}
				else
				{
					++it;
				}
			}
			Done = it;
			ret.push_back(std::string(Do, Done));

			if (ret.size() > 1)
			{
				if (ret[0] == std::string("echo"))
				{
					if (ret[1][0] == '$')
					{
						char* i = (char*)malloc(sizeof(char) * ret[1].size());
						size_t j = 0;

						// test
						//std::cout << ret[1] << std::endl;

						auto it = ret[1].begin();
						++it;
						while (it != ret[1].end())
						{
							i[j++] = *it;
							++it;
						}
						i[j] = '\0';

						if (std::string(i) != "?")
						{
							// test
							//std::cout << i << std::endl;

							char* p = getenv(i);
							std::string s;
							if (p != NULL)
							{
								s = p;
								ret[1] = s;
							}
							else
							{
								ret.pop_back();
							}
						}
						else
						{
							ret[1] = std::to_string(_exit_code);
						}

						free(i);
					}
				}
			}

			// test
			/*for (auto e : ret)
			{
				std::cout << e << " ";
			}
			std::cout << std::endl;*/

			// ls --color -a -l

			_cmd = ret;

			return ret;
		}

	public:
		shell()
			:_absolute_path(getenv("PWD"))
			, _exit_code(0)
		{}

		~shell() {}

		void command_line_prompt()
		{
			std::cout << BEGIN << getenv("USER") << "@" << getenv("HOSTNAME") << " ";
			std::cout << get_relative_path() << END << get_prompt() << " ";
		}

		void runing()
		{
			auto cmp = command_extraction();

			if (!cmp.empty())
			{
				if (cmp[0] == std::string("cd"))
				{
					char* d = NULL;
					if (cmp.size() == 1)
					{
						d = getenv("HOME");
					}
					else
					{
						d = (char*)malloc(sizeof(char) * (cmp[1].size() + 1));
						if (d == NULL)
						{
							perror("failed malloc");
						}

						size_t i = 0;
						for (auto e : cmp[1])
						{
							d[i++] = e;
						}
						d[i] = '\0';

						// test
						//std::cout << d << std::endl;

						int ret = chdir(d);

						if (ret == 0)
						{
							_absolute_path = d;
						}
						else
						{
							perror("failed chdir");
						}
						free(d);
						_exit_code = 0;
					}
				}
				else if (cmp[0] == std::string("pwd"))
				{
					// man getcwd
					std::cout << _absolute_path << std::endl;
					_exit_code = 0;
				}
				else if (cmp[0] == std::string("export"))
				{
					if (cmp.size() != 1)
					{
						_envp.push_back(cmp[1]);
						putenv((char*)(_envp.back().c_str()));
					}
					_exit_code = 0;
				}
				else
				{
					pid_t id = fork();

					if (id == 0)
					{
						char** p = (char**)malloc(sizeof(char*) * (cmp.size() + 1));
						size_t i = 0;
						for (auto e1 : cmp)
						{
							char* q = (char*)malloc(sizeof(char) * (e1.size() + 1));
							p[i++] = q;
							size_t j = 0;
							for (auto e2 : e1)
							{
								q[j++] = e2;
							}
							q[j] = '\0';
						}
						p[i] = NULL;

						execvpe(*p, p, environ);
						perror("failed exec");
						exit(1);
					}
					else if (id > 0)
					{
						int status = 0;
						pid_t ret = waitpid(id, &status, 0);

						if (ret > 0)
						{
							if (WIFEXITED(status))
							{
								_exit_code = WEXITSTATUS(status);

								// test
								//std::cout << _exit_code << std::endl;
							}
							else
							{
								_exit_code = status & 0x7f;

								// test
								//std::cout << _exit_code << std::endl;
							}
						}
						else
						{
							perror("failed waitpid");
						}

					}
					else
					{
						perror("failled fork");
					}
				}
			}
		}

	private:
		command _cmd;
		int _exit_code;
		working_directory _absolute_path;
		local_variable_pool _locp;
		environment_variable_pool _envp;
	};
};


int main()
{
	wind::shell bash;
	while (1)
	{
		bash.command_line_prompt();
		bash.runing();
	}
	return 0;
}

#else

#include<stdio.h> // printf
#include<stdlib.h> // getenv
#include<string.h> // strlen

#define LEFT "["
#define RIGHT "]"

char* const get_user_name()
{
	return getenv("USER");
}

char* const get_host_name()
{
	return getenv("HOSTNAME");
}

char* const get_working_directory()
{
	return getenv("PWD");
}

char* const get_prompt_directory()
{
	char* const pwd = get_working_directory();
	size_t sz = strlen(pwd);
	char* p = (char*)malloc(sizeof(char) * ++sz);
	if (p == NULL)
	{
		perror("failed malloc");
	}
	size_t current = 0;
	while (current < sz)
	{
		p[current] = pwd[current];
		current++;
	}
	p[current] = '\0';

	char* prev = NULL;
	char* curr = strtok(p, "/");
	while (curr = strtok(NULL, "/"))
	{
		prev = curr;
	}
	char* ret = NULL;
	if (prev == NULL)
	{
		ret = "/";
	}
	else
	{
		size_t len = strlen(prev);
		char* ptr = (char*)malloc(sizeof(char) * ++len);
		if (ptr == NULL)
		{
			perror("failed malloc");
		}
		memcpy(ptr, prev, sizeof(char) * len);
		ret = ptr;
	}
	free(p);
	return ret;
}

char* const get_prompt()
{
	if (strstr(get_user_name(), "root"))
		return "#";
	else
		return "$";
}

char** get_command()
{
	getline();
}

int main()
{
	char* p = get_prompt_directory();
	printf(LEFT "%s@%s %s" RIGHT "%s", get_user_name(), get_host_name(), p, get_prompt());
	free(p);
	return 0;
}

#endif

#endif


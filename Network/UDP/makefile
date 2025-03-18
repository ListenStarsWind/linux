.PHONY:all
all:udpclient udpserver

udpclient:udpclient.cc
	@g++ $^ -o $@ -lpthread -std=c++17
udpserver:main.cc
	@g++ $^ -o $@ -std=c++17
.PHONY:clean
clean:
	@rm -f udpserver udpclient
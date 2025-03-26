.PHONY:all
all:tcpclient tcpserver

tcpclient:tcpclient.cc
	@g++ $^ -o $@ -std=c++17 -g
tcpserver:main.cc
	@g++ $^ -o $@ -lpthread -std=c++17 -g
.PHONY:clean
clean:
	@rm -f tcpserver tcpclient
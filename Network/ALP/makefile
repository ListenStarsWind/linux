CC = g++
CFLAGS = -Wall -ljsoncpp
CLIENT_STD = -std=c++11
SERVER_STD = -std=c++11
CLIENT_SRC = ClientCal.cc
SERVER_SRC = ServerCal.cc
CLIENT_TARGET = clientcal
SERVER_TARGET = servercal

.PHONY: all clean

all: $(SERVER_TARGET) $(CLIENT_TARGET)

$(CLIENT_TARGET): $(CLIENT_SRC)
	@$(CC) $(CFLAGS) $(CLIENT_STD) $^ -o $@

$(SERVER_TARGET): $(SERVER_SRC)
	@$(CC) $(CFLAGS) $(SERVER_STD) $^ -o $@

clean:
	@rm -f $(SERVER_TARGET) $(CLIENT_TARGET)

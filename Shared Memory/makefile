.PHONY:all
all:a b

a:processa.cc
	@g++ $^ -o $@
b:processb.cc
	@g++ $^ -o $@

.PHONY:clean
clean:
	@rm -f a b
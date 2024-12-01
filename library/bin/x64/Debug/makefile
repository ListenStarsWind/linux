dynamic-lib=libwindmethod.so 
static-lib=libwindmath.a

.PHONY:all
all:$(static-lib) $(dynamic-lib)
	

$(static-lib):windmath.o
	@ar -rc $@ $^
windmath.o:windmath.cpp 
	@g++ -c $^

$(dynamic-lib):windlog.o windprint.o
	@g++ $^ -shared -o $@

windlog.o:windlog.cpp 
	@g++ windlog.cpp -fPIC -c

windprint.o:windprint.cpp 
	@g++ $^ -fPIC -c

.PHONY:clean 
clean:
	@rm -rf *.o *.a *.so windlib

.PHONY:output
output:
	@mkdir -p ./windlib/include 
	@mkdir -p ./windlib/windmath 
	@mkdir -p ./windlib/windmethod 
	@cp *.h ./windlib/include 
	@cp libwindmath.a ./windlib/windmath
	@cp libwindmethod.so ./windlib/windmethod 

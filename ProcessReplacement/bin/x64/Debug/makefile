ProcessReplacement:./../../../*.cpp
	@g++ $^ -o $@ -std=c++14

shell:ProcessReplacement.out 
	@mv $^ $@ ; rm -rf ProcessReplacement 

.PHONY:all
all:out ProcessReplacement

out:code.cxx
	@g++ $^ -o $@

.PHONY:clean
clean:
	@rm -rf ProcessReplacement ProcessReplacement.out out shell 
execute:
	@./ProcessReplacement

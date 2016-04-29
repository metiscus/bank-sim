CXX=g++
CXXFLAGS=-g -O0 -Wall -Wextra -MD -std=c++11

default: banksim

banksim_SRC=\
	account.cpp\
	bank.cpp\
	loan.cpp\
	main.cpp\

banksim_OBJ=$(banksim_SRC:.cpp=.o)

banksim: $(banksim_OBJ) $(banksim_SRC)
	$(CXX) $(CXXFLAGS) -o banksim $(banksim_OBJ)

clean:
	-rm -f *.o *.d banksim

-include *.d
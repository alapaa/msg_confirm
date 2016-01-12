CC = g++
INC = ../inc
CXXFLAGS = -g -std=c++11 -Wall -Weffc++ -fPIC -I$(INC)

vpath %.cpp ../src

COMMON_OBJ = msg_confirm.o socket_wrapper.o
#LIBOBJ = $(COMMON_OBJ) pmtumodule.o
OBJ = $(COMMON_OBJ)

LIB = -lpthread

PROGNAME = msg_confirm


.PHONY: all
all: $(PROGNAME)

$(PROGNAME): $(OBJ)
	$(CC) $(CXXFLAGS) $(OBJ) $(LIB) -o $@

.PHONY: clean
clean:
	rm -f *.o *.so $(PROGNAME) *~

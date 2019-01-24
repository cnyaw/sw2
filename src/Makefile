
TARGET    = ../lib/libsw2.a

CXX       = g++
AR        = ar

FLAGS     = -Wall -D_linux_

INCDIR    = ../include

SRC      := $(notdir $(wildcard ./*.cpp))
OBJ      := $(SRC:.cpp=.o)
DEP      := $(OBJ:%.o=%.d)

all: $(TARGET)

$(TARGET): $(OBJ)

-include $(DEP)

clean:
	@rm -f $(TARGET) $(OBJ) $(DEP)

%.o: %.cpp
	$(CXX) $(FLAGS) -I$(INCDIR) -MMD -c -o $*.o $<

%.a:
	@mkdir -p ../lib
	$(AR) -cr $@ $^

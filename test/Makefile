
TARGET    = test.exe

CXX       = g++
FLAGS     = -Wall -D_linux_

LIBDIR    = ../lib
INCDIR    = ../include

SRC      := $(notdir $(wildcard ./*.cpp)) $(foreach name, $(notdir $(wildcard CppUnitLite/*.cpp)), CppUnitLite/$(name))
OBJ      := $(SRC:.cpp=.o)
DEP      := $(OBJ:%.o=%.d)

all: $(TARGET)

$(TARGET): $(LIBDIR)/libsw2.a $(OBJ)
	$(CXX) $(OBJ) -L$(LIBDIR) -lsw2 -lz -lpthread -o $@

-include $(DEP)

clean:
	@rm -f $(TARGET) $(OBJ) $(DEP)

%.o: %.cpp
	$(CXX) $(FLAGS) -I$(INCDIR) -MMD -c -o $*.o $<

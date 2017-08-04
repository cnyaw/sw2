
TARGET    = test.exe

CXX       = g++
FLAGS     = -Wall -D_linux_

LIBDIR    = ../lib
INCDIR    = ../include

CPPFILES := $(notdir $(wildcard ./*.cpp)) $(foreach name, $(notdir $(wildcard CppUnitLite/*.cpp)), CppUnitLite/$(name))
OFILES   := $(CPPFILES:.cpp=.o)

all: $(TARGET)

$(TARGET): $(LIBDIR)/libsw2.a $(OFILES)
	$(CXX) $(OFILES) -L$(LIBDIR) -lsw2 -lz -lpthread -o $@

clean:
	@rm -f $(TARGET) $(OFILES)

%.o: %.cpp
	$(CXX) $(FLAGS) -I$(INCDIR) -c -o $*.o $<


TARGET    = ../lib/libsw2.a

CXX       = g++
AR        = ar

FLAGS     = -Wall -D_linux_

INCDIR    = ../include

CPPFILES := $(notdir $(wildcard ./*.cpp))
OFILES   := $(CPPFILES:.cpp=.o)

all: $(TARGET)

$(TARGET): $(OFILES)

clean:
	@rm -f $(TARGET) $(OFILES)

%.o: %.cpp
	$(CXX) $(FLAGS) -I$(INCDIR) -c -o $*.o $<

%.a:
	@mkdir -p ../lib
	$(AR) -cr $@ $^

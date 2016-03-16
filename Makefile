# crash_test makefile
# requires CAENVMElib

CC	= g++
CFLAGS	= -Wall -g -DLINUX -fPIC -std=c++11
LDFLAGS = -lCAENVME 
SOURCES = $(shell echo ./*cc)
OBJECTS = $(SOURCES: .cc=.o)
CPP	= crashit

all: $(SOURCES) $(CPP)

$(CPP) : $(OBJECTS)
	$(CC) $(OBJECTS) $(CFLAGS) $(LDFLAGS) $(LIBS) -o $(CPP)

clean: 
	rm $(CPP)

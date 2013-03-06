.PHONY : all clean
CFLAGS = -g -Wall -O
LDFLAGS = -L./lib -ljsoncpp
XX = g++
TARGET = pathfinder
INCLUDEDIR = "./"
all : \
	main.cpp\
	pathfinder.cpp
		$(XX) $(CFLAGS) -I$(INCLUDEDIR) -o $(TARGET) $^ $(LDFLAGS)

clean :
	rm pathfinder 

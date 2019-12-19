
TARGET =kikihttp
CC =g++ -L. -g -O3 -Wall -std=c++11
INCLUDE =$(shell find ./ -name "*.h")
SOURCE =$(shell find ./ -name "*.cpp")
OBJS =$(SOURCE:%.cpp=%.o)
LIBS =-lkikilib
CFLAGS =-I../../kikilib

$(TARGET):$(OBJS)
	$(CC) -o $(TARGET) $(OBJS) $(LIBS)
	
%.o: %.cpp $(INCLUDE)
	$(CC) $(CFLAGS) -c $< -o $@ $(LIBS)
	
all:$(TARGET)

clean:
	rm -rf $(OBJS) $(TARGET)

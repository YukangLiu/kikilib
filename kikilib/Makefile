
TARGET =libkikilib.so
CC =g++ -fPIC -g -O3 -Wall -std=c++11
INCLUDE =$(shell find ./ -name "*.h")
SOURCE =$(shell find ./ -name "*.cpp")
OBJS =$(SOURCE:%.cpp=%.o)
LIBS = -lpthread

$(TARGET):$(OBJS)
	$(CC) -shared -o $(TARGET) $(OBJS) $(LIBS)
	
%.o: %.cpp $(INCLUDE)
	$(CC) -c $< -o $@ $(LIBS)
	
all:$(TARGET)

clean:
	rm -rf $(OBJS)$(TARGET)

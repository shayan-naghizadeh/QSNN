CXX = g++
CXXXFLAGS = -std=c++11 -Wall

TARGET = SNNP

SRCS = test.cpp SNN_MNIST.cpp weights.cpp

HDRS = weights.h SNN_MNIST.h

$(TARGET) : $(SRCS) $(HDRS)
	$(CXX) $(CXXXFLAGS) -o $(TARGET) $(SRCS)

clean:
	rm -f $(TARGET)
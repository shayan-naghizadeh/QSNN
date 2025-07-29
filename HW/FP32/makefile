CXX = g++
CXXXFLAGS = -std=c++11 -Wall

TARGET = SNN

SRCS = SNN_MNIST.cpp weights.cpp spikes.cpp

HDRS = weights.h spikes.h

$(TARGET) : $(SRCS) $(HDRS)
	$(CXX) $(CXXXFLAGS) -o $(TARGET) $(SRCS)

clean:
	rm -f $(TARGET)
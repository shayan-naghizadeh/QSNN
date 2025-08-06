CXX = g++
CXXXFLAGS = -std=c++11 -Wall

TARGET = SNNP

SRCS = snn.cpp spikes.cpp weights.cpp

HDRS = weights.h spikes.h

$(TARGET) : $(SRCS) $(HDRS)
	$(CXX) $(CXXXFLAGS) -o $(TARGET) $(SRCS)

clean:
	rm -f $(TARGET)
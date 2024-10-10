CXX = g++
CXXFLAGS = -Wall -Wextra -std=c++11 -O0 -g
LIBS = `pkg-config --cflags --libs opencv4`

TARGET = screen_capture

all: $(TARGET)

$(TARGET): $(TARGET).cpp
	$(CXX) $(CXXFLAGS) -o $@ $< $(LIBS)

clean:
	rm -f $(TARGET)


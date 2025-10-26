CXX = g++
CXXFLAGS = -std=c++17 -Wall -Isrc `pkg-config --cflags gtkmm-3.0`
LDFLAGS = `pkg-config --libs gtkmm-3.0`
TARGET = system-monitor
SRCDIR = src
SOURCES = $(wildcard $(SRCDIR)/*.cpp)
OBJECTS = $(SOURCES:$(SRCDIR)/%.cpp=build/%.o)

$(TARGET): $(OBJECTS)
	$(CXX) $(OBJECTS) -o $(TARGET) $(LDFLAGS)

build/%.o: $(SRCDIR)/%.cpp | build
	$(CXX) $(CXXFLAGS) -c $< -o $@

build:
	mkdir -p build

clean:
	rm -rf build $(TARGET)

.PHONY: clean
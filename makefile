CXX = g++
CXXFLAGS = -std=c++17 -O2 -Wall
INCLUDES = -I./inc
SRC = src/main.cpp src/stock.cpp src/risk.cpp src/portfolio.cpp
OBJ_DIR = build/obj
OBJ = $(patsubst src/%.cpp,$(OBJ_DIR)/%.o,$(SRC))
TARGET = build/risk_engine

all: $(TARGET)

$(TARGET): $(OBJ)
	mkdir -p $(dir $(TARGET))
	$(CXX) $(CXXFLAGS) $(OBJ) -o $(TARGET)

$(OBJ_DIR)/%.o: src/%.cpp
	mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

clean:
	rm -rf $(OBJ_DIR)
	rm -rf build/risk_engine

.PHONY: all clean


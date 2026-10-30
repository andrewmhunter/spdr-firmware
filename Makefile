
TARGET := libspdr-firmware.a
SRCS := src/Flag.cpp src/Instruction.cpp src/Register.cpp src/Line.cpp \
	src/InstructionSet.cpp src/Mode.cpp src/MicroSequence.cpp \
	src/instructions.cpp

BUILD_DIR := build
INCLUDE_DIR := $(HOME)/.local/include/SpdrFirmware
OBJECTS := $(SRCS:%=$(BUILD_DIR)/%.o)
DEPS := $(OBJECTS:.o=.d)

# CXX := clang++

CXXFLAGS := -std=c++20 -g -Iinclude -c -MMD -MP -Wall -pedantic

# $(BUILD_DIR)/testing: $(TARGET) src/main.cpp

$(BUILD_DIR)/$(TARGET): $(OBJECTS)
	ar rs $@ $(OBJECTS)

$(BUILD_DIR)/%.cpp.o: %.cpp
	mkdir -p $(dir $@)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c $< -o $@

.PHONY: clean
clean:
	rm -r $(BUILD_DIR)

.PHONY: install
install:
	mkdir -p $(INCLUDE_DIR)
	cp -r include/. $(INCLUDE_DIR)
	cp -r $(BUILD_DIR)/$(TARGET) $(HOME)/.local/lib/$(TARGET)

-include $(DEPS)


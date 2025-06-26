ifndef NT_API_PATH
	NT_API_PATH := $(HOME)/distingNT_API_V8
endif

INCLUDE_PATH := $(NT_API_PATH)/include/

# Source and output files
inputs := $(wildcard *.cpp)
outputs := $(patsubst %.cpp,plugins/%.o,$(inputs))
deps := $(patsubst %.cpp,plugins/%.d,$(inputs))

# Compiler flags
CXX := arm-none-eabi-c++
CXXFLAGS := -std=c++11 -mcpu=cortex-m7 -mfpu=fpv5-d16 -mfloat-abi=hard -mthumb -fno-rtti -fno-exceptions -Os -fPIC -Wall -I$(INCLUDE_PATH)

all: $(outputs)

clean:
	rm -f $(outputs) $(deps)

# Generate object files
plugins/%.o: %.cpp
	mkdir -p $(@D)
	$(CXX) $(CXXFLAGS) -c -o $@ $<

# Generate dependency files
plugins/%.d: %.cpp
	mkdir -p $(@D)
	$(CXX) $(CXXFLAGS) -M -o $@ $<

# Include dependency files
-include $(deps)

debug-path:
	@echo "NT_API_PATH resolves to: $(NT_API_PATH)"
	@echo "Include flag: -I$(INCLUDE_PATH)"
	@ls -la $(INCLUDE_PATH)

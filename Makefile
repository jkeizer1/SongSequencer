#ifndef NT_API_PATH
#	NT_API_PATH := $(HOME)/distingNT_API_V8
#endif

#INCLUDE_PATH := $(NT_API_PATH)/include/distingnt
#INCLUDE_PATH := $(NT_API_PATH)/include/
INCLUDE_PATH := .

inputs := $(wildcard *cpp)
outputs := $(patsubst %.cpp,plugins/%.o,$(inputs))

all: $(outputs)

clean:
	rm -f $(outputs)

plugins/%.o: %.cpp
	mkdir -p $(@D)
	arm-none-eabi-c++ -std=c++11 -mcpu=cortex-m7 -mfpu=fpv5-d16 -mfloat-abi=hard -mthumb -fno-rtti -fno-exceptions -Os -fPIC -Wall -I$(INCLUDE_PATH) -c -o $@ $^

debug-path:
	@echo "NT_API_PATH resolves to: $(NT_API_PATH)"
	@echo "Include flag: -I$(INCLUDE_PATH)"
	@ls -la $(INCLUDE_PATH)

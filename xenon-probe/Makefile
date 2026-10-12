.SUFFIXES:
ifndef DEVKITXENON
$(error DEVKITXENON is not set; use the Free60 libXenon container or install the SDK)
endif
include $(DEVKITXENON)/rules
TARGET := cubex360_toolchain_probe
BUILD := build
SOURCES := src
INCLUDES :=
LIBDIRS :=
CFLAGS = -g -O2 -Wall $(MACHDEP) $(INCLUDE)
CXXFLAGS = $(CFLAGS)
LDFLAGS = -g $(MACHDEP) -Wl,--gc-sections -Wl,-Map,$(notdir $@).map
LIBS := -lxenon -lm
ifneq ($(BUILD),$(notdir $(CURDIR)))
export OUTPUT := $(CURDIR)/$(TARGET)
export VPATH := $(foreach dir,$(SOURCES),$(CURDIR)/$(dir))
export DEPSDIR := $(CURDIR)/$(BUILD)
CFILES := $(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.c)))
CPPFILES := $(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.cpp)))
export OFILES := $(CPPFILES:.cpp=.o) $(CFILES:.c=.o)
export INCLUDE := $(foreach dir,$(INCLUDES),-I$(CURDIR)/$(dir)) -I$(CURDIR)/$(BUILD) -I$(LIBXENON_INC)
export LIBPATHS := $(foreach dir,$(LIBDIRS),-L$(dir)/lib) -L$(LIBXENON_LIB)
export LD := $(CC)
.PHONY: all clean
all: $(BUILD) $(OUTPUT).elf32
$(BUILD):
	@mkdir -p $@
	@$(MAKE) --no-print-directory -C $(BUILD) -f $(CURDIR)/Makefile
clean:
	rm -rf $(BUILD) $(OUTPUT).elf $(OUTPUT).elf32 $(OUTPUT).elf32.map
else
DEPENDS := $(OFILES:.o=.d)
$(OUTPUT).elf32: $(OUTPUT).elf
-include $(DEPENDS)
endif

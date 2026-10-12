.SUFFIXES:
ifndef DEVKITXENON
$(error DEVKITXENON is not set; use Free60/libXenon container)
endif
include $(DEVKITXENON)/rules
TARGET := cubex360_core_smoke
BUILD := build
SOURCES := src
INCLUDES := include
LIBDIRS :=
CFLAGS = -O2 -Wall -Wextra $(MACHDEP) $(INCLUDE)
CXXFLAGS = $(CFLAGS)
LDFLAGS = $(MACHDEP) -Wl,--gc-sections
LIBS := -lxenon -lm
ifneq ($(BUILD),$(notdir $(CURDIR)))
export OUTPUT := $(CURDIR)/$(TARGET)
export VPATH := $(CURDIR)/$(SOURCES)
export DEPSDIR := $(CURDIR)/$(BUILD)
CFILES := $(notdir $(wildcard $(SOURCES)/*.c))
export OFILES := $(CFILES:.c=.o)
export INCLUDE := -I$(CURDIR)/$(INCLUDES) -I$(CURDIR)/$(BUILD) -I$(LIBXENON_INC)
export LIBPATHS := -L$(LIBXENON_LIB)
export LD := $(CC)
.PHONY: all clean
all: $(BUILD) $(OUTPUT).elf32
$(BUILD):
	@mkdir -p $@
	@$(MAKE) --no-print-directory -C $(BUILD) -f $(CURDIR)/Makefile
clean:
	rm -rf $(BUILD) $(OUTPUT).elf $(OUTPUT).elf32
else
DEPENDS := $(OFILES:.o=.d)
$(OUTPUT).elf: $(OFILES)
$(OUTPUT).elf32: $(OUTPUT).elf
-include $(DEPENDS)
endif

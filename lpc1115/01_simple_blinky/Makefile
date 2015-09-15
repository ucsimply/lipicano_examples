 
# program name
RESULT := simple_blinky

# target uC - LPC1115
# defines are required by LPCOpen headers
DEFCPU := -DCORE_M0 -DCHIP_LPC11XXLV -DENABLE_UNTESTED_CODE

# Verbose output
VERBOSE := 1
# Debug build
DEBUG := 1

# CPU type
CPUTYPE  := cortex-m0
CPUFLAGS := -mthumb -mcpu=$(CPUTYPE) $(DEFCPU)

# Linker script
LDSCRIPT := lpc1115.ld

# toolset
CROSS_PREFIX := arm-none-eabi-
CC_normal := $(CROSS_PREFIX)gcc
LD_normal := $(CROSS_PREFIX)gcc
AS := $(CROSS_PREFIX)as
OBJCOPY := $(CROSS_PREFIX)objcopy
OBJDUMP := $(CROSS_PREFIX)objdump
SIZE := $(CROSS_PREFIX)size
RM := rm

# include directories
INCDIR := include
INCDIR += ../../include/lpc11xx

# source directories
SRCDIR := source

# source files
SOURCES := vectors.c
SOURCES += crp.c
SOURCES += reset.c
SOURCES += lowinit.c
SOURCES += main.c

# include header files
INCLUDES := $(foreach dir,$(INCDIR),-I $(dir))

WARNS := -Wall
# WARNS += -Wextra
WARNS += -Wshadow
WARNS += -Wpointer-arith
WARNS += -Wbad-function-cast
WARNS += -Wmissing-prototypes
WARNS += -Wmissing-declarations
WARNS += -Wcast-align
WARNS += -Wredundant-decls

ifeq ($(VERBOSE),1)
  CC = $(CC_normal)
  LD = $(LD_normal)
else
  CC = @echo " CC " $@ && $(CC_normal)
  LD = @echo " LD " $@ && $(LD_normal)
endif

ifeq ($(DEBUG),1)
  OPTIM := -gdwarf-2 -g3 -O0
else
  OPTIM := -O2
endif

CFLAGS := -std=c99
CFLAGS += -ffunction-sections
CFLAGS += -fdata-sections
CFLAGS += -fno-builtin 
CFLAGS += $(CPUFLAGS)
CFLAGS += $(WARNS)
CFLAGS += $(OPTIM)
CFLAGS += $(INCLUDES)

LDFLAGS := -T $(LDSCRIPT)
LDFLAGS += -nostartfiles
LDFLAGS += -mthumb
LDFLAGS += -mcpu=$(CPUTYPE)
LDFLAGS += -Wl,-Map=$(RESULT).map
LDFLAGS += -Wl,--gc-sections
#LDFLAGS += -Wl,--print-gc-sections

# object files
OBJECTS := $(SOURCES:.c=.o)

vpath %.c $(SRCDIR)
vpath %.h $(INCDIR)

all : $(RESULT).bin

$(RESULT).bin : $(RESULT).elf
	$(OBJCOPY) -O binary $(RESULT).elf $(RESULT).bin
	$(OBJCOPY) -O ihex $(RESULT).elf $(RESULT).hex
	$(OBJDUMP) -S $(RESULT).elf >$(RESULT).lst
	fix_checksum $(RESULT).bin
	$(SIZE) $(OBJECTS) $(RESULT).elf

$(RESULT).elf : $(OBJECTS)
	$(LD) $(LDFLAGS) $^ -o $@

-include make.dep

clean :
	$(RM) -f *.o
	$(RM) -f $(RESULT).*

dep : $(SOURCES)
	$(CC) -E -MM $(DEFCPU) $(INCLUDES) $^ >make.dep

# obj file general rule
%.o : %.c
	$(CC) $(CFLAGS) -c $< -o $@
	
.PHONY : all, clean, dep

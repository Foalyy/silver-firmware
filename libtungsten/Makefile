# Default config
ifndef ROOTDIR
	ROOTDIR=.
endif
ifndef CORTEX_M
	CORTEX_M=4
endif
ifndef CHIP_FAMILY
	CHIP_FAMILY=sam4l
endif
ifndef CHIP_MODEL
	# ls2x, ls4x or ls8x, indicates the quantity of memory (see datasheet 2.2 Configuration Summary)
	CHIP_MODEL=ls4x
endif
ifndef PACKAGE
	PACKAGE=64
endif
ifndef OPENOCD_CFG
	OPENOCD_CFG=$(ROOTDIR)/$(LIBNAME)/openocd.cfg
endif
ifndef CARBIDE
	CARBIDE=false
endif
ifndef DEBUG
	DEBUG=false
endif
ifndef BOOTLOADER
	BOOTLOADER=false
endif
ifndef CREATE_MAP
	CREATE_MAP=false
endif
ifndef SERIAL_PORT
	SERIAL_PORT=/dev/ttyACM0
endif

# Library
LIBNAME=libtungsten
CORE_MODULES=pins_$(CHIP_FAMILY)_$(PACKAGE) ast bpm bscif core dma error flash gpio interrupt_priorities pm scif usb wdt
LIB_MODULES=$(CORE_MODULES) $(MODULES)

# Compilation objects
LIB_OBJS=$(addprefix $(ROOTDIR)/$(LIBNAME)/$(CHIP_FAMILY)/,$(addsuffix .o,$(LIB_MODULES)))
UTILS_OBJS=$(addprefix $(ROOTDIR)/$(LIBNAME)/utils/,$(addsuffix .o,$(UTILS_MODULES)))
USER_OBJS=$(addsuffix .o,$(USER_MODULES))

# Carbide-specific options
ifeq ($(strip $(CARBIDE)), true)
	LIB_OBJS+=libtungsten/carbide/carbide.o
	PACKAGE=64
endif

# Toolchain
CXX=$(TOOLCHAIN_PATH)arm-none-eabi-g++
OBJCOPY=$(TOOLCHAIN_PATH)arm-none-eabi-objcopy
GDB=$(TOOLCHAIN_PATH)arm-none-eabi-gdb
SIZE=$(TOOLCHAIN_PATH)arm-none-eabi-size
OBJDUMP=$(TOOLCHAIN_PATH)arm-none-eabi-objdump
OPENOCD=openocd

# Architecture options
ARCH_FLAGS=-mthumb -mcpu=cortex-m$(CORTEX_M)

# Startup code
STARTUP=$(ROOTDIR)/$(LIBNAME)/$(CHIP_FAMILY)/startup.cpp

# Defines passed to the preprocessor using -D
ifeq ($(strip $(CHIP_MODEL)),ls2x)
	N_FLASH_PAGES=256
else ifeq ($(strip $(CHIP_MODEL)),ls4x)
	N_FLASH_PAGES=512
else ifeq ($(strip $(CHIP_MODEL)),ls8x)
	N_FLASH_PAGES=1024
else
$(error Unknown CHIP_MODEL $(CHIP_MODEL), please use ls2x, ls4x or ls8x)
endif
PREPROC_DEFINES=-DPACKAGE=$(PACKAGE) -DBOOTLOADER=$(BOOTLOADER) -DDEBUG=$(DEBUG) -DN_FLASH_PAGES=$(N_FLASH_PAGES)

# Compilation flags
# Note : do not use -O0, as this might generate code too slow for some peripherals (notably the SPI controller)
ifeq ($(strip $(DEBUG)), true)
	OPTFLAGS=-Og -g
else
	OPTFLAGS=-Os -ffunction-sections -fdata-sections
endif
CXXFLAGS=$(ARCH_FLAGS) $(STARTUP_DEFS) \
	-I$(ROOTDIR)/$(LIBNAME) \
	-I$(ROOTDIR)/$(LIBNAME)/$(CHIP_FAMILY) \
	-I$(ROOTDIR)/$(LIBNAME)/utils \
	-I$(ROOTDIR)/$(LIBNAME)/carbide \
	-std=c++11 -Wall $(OPTFLAGS) $(PREPROC_DEFINES) $(ADD_CXXFLAGS)

# Linking flags
ifndef LD_SCRIPT_NAME
	LD_SCRIPT_NAME=usercode_$(CHIP_MODEL).ld
	ifeq ($(strip $(BOOTLOADER)), true)
		LD_SCRIPT_NAME=usercode_bootloader_$(CHIP_MODEL).ld
	endif
endif
ifeq ($(strip $(CREATE_MAP)), true)
	MAP=-Wl,-Map=$(NAME).map
endif
LFLAGS=--specs=nano.specs --specs=nosys.specs -L. -L$(ROOTDIR)/$(LIBNAME) -L$(ROOTDIR)/$(LIBNAME)/$(CHIP_FAMILY) -L$(ROOTDIR)/$(LIBNAME)/carbide -L$(ROOTDIR)/$(LIBNAME)/ld_scripts -T $(LD_SCRIPT_NAME) -Wl,--gc-sections $(MAP)



### RULES

.PHONY: flash pause reset debug flash-debug objdump codeuploader upload bootloader flash-bootloader debug-bootloader objdump-bootloader openocd clean clean-all _echo_config _echo_comp_lib_objs _echo_comp_user_objs


## Usercode-related rules

# Default rule, compile the firmware in Intel HEX format, ready to be flashed/uploaded
# https://en.wikipedia.org/wiki/Intel_HEX
all: $(NAME).hex
	@echo ""
	@echo "== Finished compiling successfully!"

_echo_config:
	@echo "== Configuration summary :"
	@echo ""
	@echo "    CARBIDE=$(CARBIDE)"
	@echo "    PACKAGE=$(PACKAGE)"
	@echo "    CHIP_MODEL=$(CHIP_MODEL)"
	@echo "    DEBUG=$(DEBUG)"
	@echo "    BOOTLOADER=$(BOOTLOADER)"
	@echo "    MODULES=$(MODULES)"
	@echo "    UTILS_MODULES=$(UTILS_MODULES)"
	@echo "    EXT_MODULES=$(EXT_MODULES)"
	@echo "    CREATE_MAP=$(CREATE_MAP)"
	@echo "    LD_SCRIPT_NAME=$(LD_SCRIPT_NAME)"
	@echo ""
	@echo "(if configuration has changed since the last compilation, remember to perform 'make clean' first)"

_echo_comp_lib_objs:
	@echo ""
	@echo "== Compiling library modules..."

_echo_comp_user_objs:
	@echo ""
	@echo "== Compiling user-defined modules..."

# Compile user code in the standard ELF format
$(NAME).elf: _echo_config $(NAME).cpp $(STARTUP) $(STARTUP_DEP) _echo_comp_lib_objs $(LIB_OBJS) $(UTILS_OBJS) _echo_comp_user_objs $(USER_OBJS)
	@echo ""
	@echo "== Compiling ELF..."
	$(CXX) $(CXXFLAGS) $(LFLAGS) $(NAME).cpp $(STARTUP) $(STARTUP_DEP) $(LIB_OBJS) $(UTILS_OBJS) $(USER_OBJS) -o $@

# Convert from ELF to iHEX format
$(NAME).hex: $(NAME).elf
	@echo ""
	@echo "== Converting ELF to Intel HEX format"
	$(OBJCOPY) -O ihex $^ $@
	@echo "Binary size :" `$(SIZE) -d $(NAME).elf | tail -n 1 | cut -f 4` "bytes"

# Compile library
$(ROOTDIR)/$(LIBNAME)/$(CHIP_FAMILY)/%.o: $(ROOTDIR)/$(LIBNAME)/$(CHIP_FAMILY)/%.cpp $(ROOTDIR)/$(LIBNAME)/$(CHIP_FAMILY)/%.h $(ROOTDIR)/$(LIBNAME)/$(CHIP_FAMILY)/pins_$(CHIP_FAMILY)_$(PACKAGE).cpp $(ROOTDIR)/$(LIBNAME)/$(CHIP_FAMILY)/interrupt_priorities.cpp
	$(CXX) $(CXXFLAGS) $(LFLAGS) -c $< -o $@

# Compile other modules
%.o: %.cpp %.h
	$(CXX) $(CXXFLAGS) $(LFLAGS) -c $< -o $@

# Start OpenOCD, which is used to reset/flash the chip and as a remote target for GDB
openocd:
	$(OPENOCD) -f $(OPENOCD_CFG)

# Flash the firmware into the chip using OpenOCD
flash: $(NAME).hex
	@echo ""
	@echo "== Flashing into chip (make sure OpenOCD is started in background using 'make openocd')"
	echo "reset halt; flash write_image erase unlock $(NAME).hex; reset run; exit" | netcat localhost 4444

# Flash the firmware into the chip by automatically starting a temporary OpenOCD instance
autoflash: $(NAME).hex
	$(OPENOCD) -f $(OPENOCD_CFG) & (sleep 1; echo "reset halt; flash write_image erase unlock $(NAME).hex; reset run; exit" | netcat localhost 4444) > /dev/null; killall openocd

# Erase the chip's flash using OpenOCD
erase:
	@echo "== Erasing the chip's flash..."
	echo "reset halt; flash erase_sector 0 0 last; exit" | netcat localhost 4444

# Pause the chip execution using OpenOCD
pause:
	@echo "== Halting the chip execution..."
	echo "reset halt; exit" | netcat localhost 4444

# Reset the chip using OpenOCD
reset:
	@echo "== Resetting the chip..."
	echo "reset run; exit" | netcat localhost 4444

# Open GDB through OpenOCD to debug the firmware
debug: pause
	$(GDB) -ex "set print pretty on" -ex "target extended-remote localhost:3333" $(NAME).elf

# Flash and debug the firmware
flash-debug: flash debug

# Show the disassembly of the compiled program
objdump: $(NAME).hex
	$(OBJDUMP) -dSC $(NAME).elf | less


## Codeuploader-related rules

# Compile the codeuploader
codeuploader:
	make -C $(ROOTDIR)/$(LIBNAME)/codeuploader

# Upload user code via bootloader with the default channel : USB
upload: upload-usb

# Upload through USB
upload-usb: $(NAME).hex codeuploader
	$(ROOTDIR)/$(LIBNAME)/codeuploader/codeuploader $(NAME).hex

# Upload through serial port
upload-serial: $(NAME).hex codeuploader
	$(ROOTDIR)/$(LIBNAME)/codeuploader/codeuploader $(NAME).hex $(SERIAL_PORT)


## Bootloader-related rules

# Compile the bootloader
bootloader: $(ROOTDIR)/$(LIBNAME)/bootloader/bootloader_config.h
	make -C $(ROOTDIR)/$(LIBNAME)/bootloader
	@echo "Bootloader size :" `$(SIZE) -d libtungsten/bootloader/bootloader.elf | tail -n 1 | cut -f 4` "bytes"

# Flash the bootloader into the chip using OpenOCD
flash-bootloader: bootloader
	echo "reset halt; flash write_image erase unlock $(ROOTDIR)/$(LIBNAME)/bootloader/bootloader.hex; reset run; exit" | netcat localhost 4444

# Flash the bootloader into the chip by automatically starting a temporary OpenOCD instance
autoflash-bootloader: bootloader
	$(OPENOCD) -f $(OPENOCD_CFG) & (sleep 1; echo "reset halt; flash write_image erase unlock $(ROOTDIR)/$(LIBNAME)/bootloader/bootloader.hex; reset run; exit" | netcat localhost 4444) > /dev/null; killall openocd

# Debug the bootloader
debug-bootloader: pause
	$(GDB) -ex "set print pretty on" -ex "target extended-remote localhost:3333" $(ROOTDIR)/$(LIBNAME)/bootloader/bootloader.elf

# Flash and debug the bootloader
flash-debug-bootloader: flash-bootloader debug-bootloader

# Show the disassembly of the compiled bootloader
objdump-bootloader: bootloader
	$(OBJDUMP) -d $(ROOTDIR)/$(LIBNAME)/bootloader/bootloader.elf | less


## USBCom

usbcom_dump:
	make -C $(ROOTDIR)/$(LIBNAME)/usbcom usbcom_dump
	$(ROOTDIR)/$(LIBNAME)/usbcom/usbcom_dump

usbcom_write:
	make -C $(ROOTDIR)/$(LIBNAME)/usbcom usbcom_write
	$(ROOTDIR)/$(LIBNAME)/usbcom/usbcom_write


## Cleaning rules

# The 'clean' rule can be redefined in your Makefile to add your own logic, but remember :
# 1/ to define it AFTER the 'include libtungsten/Makefile' instruction
# 2/ to add the 'clean-all' dependency
clean: clean-all

clean-all: 
	rm -f $(NAME).elf $(NAME).map $(NAME).hex *.o $(ROOTDIR)/$(LIBNAME)/*.o $(ROOTDIR)/$(LIBNAME)/utils/*.o $(ROOTDIR)/$(LIBNAME)/$(CHIP_FAMILY)/*.o $(ROOTDIR)/$(LIBNAME)/carbide/*.o
	cd $(ROOTDIR)/$(LIBNAME)/bootloader; rm -f bootloader.elf bootloader.hex *.o
	cd $(ROOTDIR)/$(LIBNAME)/codeuploader; rm -f codeuploader *.o
	make -C $(ROOTDIR)/$(LIBNAME)/usbcom clean

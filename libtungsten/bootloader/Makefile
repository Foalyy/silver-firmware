## libtungsten project compilation config
# Remember to do 'make clean' whenever you change anything in this Makefile

NAME=bootloader
#DEBUG=true

# Available modules : adc dac eic gloc i2c spi tc trng usart wdt.
# Some modules such as gpio and flash are already compiled by default
# and must not be added here.
MODULES=usart

# Path to the toolchain. If the tools (such as arm-none-eabi-g++) are
# not in your default $PATH, you MUST define their location here.
# (don't forget the trailing slash)
TOOLCHAIN_PATH=/home/foaly/Software/gcc-arm-none-eabi/bin/

# Include the main lib makefile
ROOTDIR=../..
LDSCRIPTNAME=bootloader.ld
ADD_CXXFLAGS=-fno-exceptions
include ../Makefile

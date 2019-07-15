## CONFIG

NAME=silver

BOOTLOADER=false
DEBUG=true
#CARBIDE=true
PACKAGE=64

# Available modules : adc dac eic gloc i2c spi tc trng usart wdt
# If not specified, all modules will be compiled
MODULES=spi

# Available utils modules : RingBuffer
# If not specified, all utils modules will be compiled
UTILS_MODULES=

USER_MODULES=\
	gui \
	sync \
	sync_usb \
	context \
	drivers/oled_ssd1306/oled \
	drivers/oled_ssd1306/font_small \
	drivers/oled_ssd1306/font_medium \
	drivers/oled_ssd1306/font_large \
	drivers/lora/lora

TOOLCHAIN_PATH=/home/foaly/Software/gcc-arm-none-eabi/bin/

# Include the main lib makefile
include libtungsten/Makefile

# Custom rules
clean: clean-all
	rm -f drivers/*.o
	rm -f drivers/lora/*.o
	rm -f drivers/oled_ssd1306/*.o

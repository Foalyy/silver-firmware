#ifndef _FLASH_H_
#define _FLASH_H_

#include <stdint.h>

// This module gives access to the Flash memory embedded in the chip
namespace Flash {

    // Config registers memory space base address and flash memory base address
    const uint32_t FLASH_BASE = 0x400A0000;
    const uint32_t FLASH_ARRAY_BASE = 0x00000000;
    const uint32_t USER_PAGE_BASE = FLASH_ARRAY_BASE + 0x00800000;
    const int FLASH_PAGE_SIZE_BYTES = 512; // bytes
    const int FLASH_PAGE_SIZE_WORDS = 128; // words
    // N_FLASH_PAGES is a preprocessor word defined in the library Makefile which depend on CHIP_MODEL
    const int FLASH_PAGES = N_FLASH_PAGES; // pages

    // Register offsets
    const uint32_t OFFSET_FCR =         0x00; // Flash Control Register
    const uint32_t OFFSET_FCMD =        0x04; // Flash Command Register
    const uint32_t OFFSET_FSR =         0x08; // Flash Status Register
    const uint32_t OFFSET_FPR =         0x0C; // Flash Parameter Register
    const uint32_t OFFSET_FVR =         0x10; // Flash Version Register
    const uint32_t OFFSET_FGPFRHI =     0x14; // Flash General Purpose Fuse Register Hi -- not implemented
    const uint32_t OFFSET_FGPFRLO =     0x18; // Flash General Purpose Fuse Register Lo
    const uint32_t OFFSET_CTRL =        0x408; // PicoCache Control Register
    const uint32_t OFFSET_SR =          0x40C; // PicoCache Status Register
    const uint32_t OFFSET_MAINT0 =      0x420; // PicoCache Maintenance Register 0
    const uint32_t OFFSET_MAINT1 =      0x424; // PicoCache Maintenance Register 1
    const uint32_t OFFSET_MCFG =        0x428; // PicoCache Monitor Configuration Register
    const uint32_t OFFSET_MEN =         0x42C; // PicoCache Monitor Enable Register
    const uint32_t OFFSET_MCTRL =       0x430; // PicoCache Monitor Control Register
    const uint32_t OFFSET_MSR =         0x434; // PicoCache Monitor Status Register
    const uint32_t OFFSET_PVR =         0x4FC; // Version Register
    
    // Constants
    const uint32_t FSR_FRDY = 0;
    const uint32_t FSR_HSMODE = 6;
    const uint32_t FCMD_CMD = 0;
    const uint32_t FCMD_CMD_NOP = 0;
    const uint32_t FCMD_CMD_WP = 1;
    const uint32_t FCMD_CMD_EP = 2;
    const uint32_t FCMD_CMD_CPB = 3;
    const uint32_t FCMD_CMD_LP = 4;
    const uint32_t FCMD_CMD_UP = 5;
    const uint32_t FCMD_CMD_EA = 6;
    const uint32_t FCMD_CMD_WGPB = 7;
    const uint32_t FCMD_CMD_EGPB = 8;
    const uint32_t FCMD_CMD_SSB = 9;
    const uint32_t FCMD_CMD_PGPFB = 10;
    const uint32_t FCMD_CMD_EAGPF = 11;
    const uint32_t FCMD_CMD_QPR = 12;
    const uint32_t FCMD_CMD_WUP = 13;
    const uint32_t FCMD_CMD_EUP = 14;
    const uint32_t FCMD_CMD_QPRUP = 15;
    const uint32_t FCMD_CMD_HSEN = 16;
    const uint32_t FCMD_CMD_HSDIS = 17;
    const uint32_t FCMD_PAGEN = 8;
    const uint32_t FCMD_KEY = 0xA5 << 24;

    // General-purpose fuses
    using Fuse = uint8_t;
    const int N_FUSES = 16;

    // These fuses are used by the bootloader and are reserved if the bootloader is enabled
    // They need to be defined here for the Core::resetToBootloader() helper function
    const Fuse FUSE_BOOTLOADER_FW_READY = 0;
    const Fuse FUSE_BOOTLOADER_FORCE = 1;
    const Fuse FUSE_BOOTLOADER_SKIP_TIMEOUT = 2;
    const int BOOTLOADER_N_RESERVED_FUSES = 3;


    // Module API
    bool isReady();
    uint32_t read(uint32_t address);
    void readPage(int page, uint32_t data[]);
    void erasePage(int page);
    void clearPageBuffer();
    void writePage(int page, const uint32_t data[]);
    void readUserPage(uint32_t data[]);
    void eraseUserPage();
    void writeUserPage(const uint32_t data[]);
    void writeFuse(Fuse fuse, bool state);
    bool getFuse(Fuse fuse);
    void enableHighSpeedMode();
    void disableHighSpeedMode();

}


#endif
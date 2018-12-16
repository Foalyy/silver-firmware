MEMORY
{
  FLASH (rx) : ORIGIN = 0x4000, LENGTH = 0x1C000 /* 128K (0x20000) minus bootloader*/
  RAM (rwx) : ORIGIN = 0x20000000, LENGTH = 0x8000 /* 32K */
}

INCLUDE common.ld
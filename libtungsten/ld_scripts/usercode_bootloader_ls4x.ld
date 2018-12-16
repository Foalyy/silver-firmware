MEMORY
{
  FLASH (rx) : ORIGIN = 0x4000, LENGTH = 0x3C000 /* 256K (0x40000) minus bootloader*/
  RAM (rwx) : ORIGIN = 0x20000000, LENGTH = 0x8000 /* 32K */
}

INCLUDE common.ld
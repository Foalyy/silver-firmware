MEMORY
{
  FLASH (rx) : ORIGIN = 0x4000, LENGTH = 0x7C000 /* 512K (0x80000) minus bootloader*/
  RAM (rwx) : ORIGIN = 0x20000000, LENGTH = 0x10000 /* 64K */
}

INCLUDE common.ld
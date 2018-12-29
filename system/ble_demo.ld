/* Linker script to configure memory regions. */

SEARCH_DIR(.)
GROUP(-lgcc -lc -lnosys)

MEMORY
{
  /*FLASH (rx) : ORIGIN = 0x0, LENGTH = 0x40000*/
  FLASH (rx) : ORIGIN = 0x0001B000, LENGTH = 0x25000 /* SoftDevice S130 uses 0x1B000 bytes at start of Flash */
  /*RAM (rwx) :  ORIGIN = 0x20000000, LENGTH = 0x4000*/
  RAM (rwx) : ORIGIN = 0x20001FE8, LENGTH = 0x2018 /* SoftDevice S130 uses at least 0x13C8 bytes at start of RAM */
}

SECTIONS
{
  .fs_data :
  {
    PROVIDE(__start_fs_data = .);
    KEEP(*(.fs_data))
    PROVIDE(__stop_fs_data = .);
  } > RAM
  .pwr_mgmt_data :
  {
    PROVIDE(__start_pwr_mgmt_data = .);
    KEEP(*(.pwr_mgmt_data))
    PROVIDE(__stop_pwr_mgmt_data = .);
  } > RAM
} INSERT AFTER .data;

INCLUDE "nrf5x_common.ld"

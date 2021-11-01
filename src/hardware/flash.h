#ifndef FLASH_H
#define FLASH_H

#include <stdint.h>

#define FLASH_WREN  0x06    // WriteEnable 
#define FLASH_WRDI  0x04
#define FLASH_PP    0x02    // PageProgram 
#define FLASH_SE    0x20    // SectorErase 
#define FLASH_READ  0x03    // Read 
#define FLASH_FREAD 0x03    // FastRead  
#define FLASH_RDID  0x9F    // ReadIdentification 
#define FLASH_RDSR  0x05    // ReadStatusRegister 
#define FLASH_RDPD  0xab    // ReleaseFromDeepPowerDown 
#define FLASH_DPD   0xb9    // DeepPowerDown 

void spiflash_sector_erase(uint32_t addr);

void spiflash_write_data(uint32_t addr, uint8_t* data, uint32_t length);

void spiflash_read_data(uint32_t addr, uint8_t* data, uint32_t length);

void spiflash_init();

void spiflash_sleep();
void spiflash_wakeup();

#endif /* FLASH_H */

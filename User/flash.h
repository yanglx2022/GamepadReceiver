/**********************
* 读写FLASH
**********************/
#ifndef __FLASH_H
#define __FLASH_H
#ifdef __cplusplus
 extern "C" {
#endif

#include "stm32f10x.h"

// FLASH容量(K)
#define FLASH_SIZE			64
// 扇区大小(byte)
#if FLASH_SIZE < 256
	#define FLASH_SECTOR_SIZE 	1024
#else 
	#define FLASH_SECTOR_SIZE		2048
#endif	

// FLASH分配(共64K)
#define	FLASH_BOOT_SIZE				(1024*17)
#define FLASH_APP_SIZE				(1024*12)
#define FLASH_FS_SIZE					(1024*23)
#define FLASH_UDISK_SIZE			FLASH_APP_SIZE

#define	FLASH_BOOT_BASE				(0x08000000)
#define	FLASH_APP_BASE				(FLASH_BOOT_BASE + FLASH_BOOT_SIZE)
#define	FLASH_FS_BASE					(FLASH_APP_BASE + FLASH_APP_SIZE)
#define	FLASH_UDISK_BASE			(FLASH_FS_BASE + FLASH_FS_SIZE)

// 非代码区域
#define USER_SECTOR_COUNT			((FLASH_FS_SIZE + FLASH_UDISK_SIZE) / FLASH_SECTOR_SIZE)
#define USER_FLASH_BASE				FLASH_FS_BASE


// 从指定地址开始读数据
void Flash_Read(uint32_t address, uint8_t* buffer, uint32_t length);
// 从指定地址开始写数据(address与length必须为2的倍数)
void Flash_Write(uint32_t address, const uint8_t* buffer, uint32_t length);


#ifdef __cplusplus
}
#endif
#endif /*__FLASH_H */



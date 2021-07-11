/**********************
* ��дFLASH
**********************/
#ifndef __FLASH_H
#define __FLASH_H
#ifdef __cplusplus
 extern "C" {
#endif

#include "stm32f10x.h"

// FLASH����(K)
#define FLASH_SIZE			64
// ������С(byte)
#if FLASH_SIZE < 256
	#define FLASH_SECTOR_SIZE 	1024
#else 
	#define FLASH_SECTOR_SIZE		2048
#endif	

// FLASH����(��64K)
#define	FLASH_BOOT_SIZE				(1024*17)
#define FLASH_APP_SIZE				(1024*12)
#define FLASH_FS_SIZE					(1024*23)
#define FLASH_UDISK_SIZE			FLASH_APP_SIZE

#define	FLASH_BOOT_BASE				(0x08000000)
#define	FLASH_APP_BASE				(FLASH_BOOT_BASE + FLASH_BOOT_SIZE)
#define	FLASH_FS_BASE					(FLASH_APP_BASE + FLASH_APP_SIZE)
#define	FLASH_UDISK_BASE			(FLASH_FS_BASE + FLASH_FS_SIZE)

// �Ǵ�������
#define USER_SECTOR_COUNT			((FLASH_FS_SIZE + FLASH_UDISK_SIZE) / FLASH_SECTOR_SIZE)
#define USER_FLASH_BASE				FLASH_FS_BASE


// ��ָ����ַ��ʼ������
void Flash_Read(uint32_t address, uint8_t* buffer, uint32_t length);
// ��ָ����ַ��ʼд����(address��length����Ϊ2�ı���)
void Flash_Write(uint32_t address, const uint8_t* buffer, uint32_t length);


#ifdef __cplusplus
}
#endif
#endif /*__FLASH_H */



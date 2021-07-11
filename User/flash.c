/**********************
* 读写FLASH
**********************/
#include "flash.h"

// 写数据时用到的缓存
static uint8_t FLASH_BUFFER[FLASH_SECTOR_SIZE];


// 从指定地址开始读数据
void Flash_Read(uint32_t address, uint8_t* buffer, uint32_t length)
{
	for(int i = 0; i < length / 2; i++)
	{
		*(uint16_t*)(buffer + 2 * i) = *(uint16_t*)(address);
		address += 2;
	}
	if (length % 2 > 0)
	{
		buffer[length - 1] = *(uint8_t*)(address);
	}
}

// 从指定地址开始写数据(address与length必须为2的倍数)
void Flash_Write(uint32_t address, const uint8_t* buffer, uint32_t length)
{
	// 非法地址
	if (address < FLASH_BASE || (address + length) > (FLASH_BASE + 1024 * FLASH_SIZE))
	{
		return;
	}
	uint32_t sector_addr = (address - FLASH_BASE) / FLASH_SECTOR_SIZE;		// 扇区编号
	uint32_t sector_offset = (address - FLASH_BASE) % FLASH_SECTOR_SIZE;	// 扇区内偏移
	uint32_t sector_bytes = FLASH_SECTOR_SIZE - sector_offset;						// 要写入扇区的字节数
	if (sector_bytes > length)
	{
		sector_bytes = length;
	}
	// 解锁
	FLASH_Unlock();
	// 逐扇区写
	while(1)
	{
		Flash_Read(sector_addr * FLASH_SECTOR_SIZE + FLASH_BASE, FLASH_BUFFER, FLASH_SECTOR_SIZE);	// 读整个扇区
		uint8_t erase = 0;
		for(int i = 0; i < sector_bytes; i++)		// 校验数据
		{
			if (FLASH_BUFFER[i + sector_offset] != 0xFF)
			{
				erase = 1;	// 需要擦除
				break;
			}
		}
		if (erase == 1)	// 需要擦除
		{
			FLASH_ErasePage(sector_addr * FLASH_SECTOR_SIZE + FLASH_BASE);	// 擦除扇区
			for(int i = 0; i < sector_bytes; i++)					// 复制数据
			{
				FLASH_BUFFER[i + sector_offset] = buffer[i];	  
			}
			for(int i = 0; i < FLASH_SECTOR_SIZE; i += 2)	// 写入整个扇区
			{
				FLASH_ProgramHalfWord(sector_addr * FLASH_SECTOR_SIZE + FLASH_BASE + i, *(uint16_t*)(FLASH_BUFFER + i));
			}
		}
		else						// 不需要擦除
		{
			for(int i = 0; i < sector_bytes; i += 2)			// 写入扇区
			{
				FLASH_ProgramHalfWord(address + i, *(uint16_t*)(buffer + i));
			}
		}
		if (sector_bytes == length)  // 	完成写入
		{
			break;
		}
		sector_addr++;								// 扇区编号增1
		sector_offset = 0;            // 扇区内偏移置0 
		buffer += sector_bytes;       // 数据指针偏移
		address += sector_bytes;  		// 写地址偏移	 
		length -= sector_bytes;       // 更新剩余字节数
		if (length > FLASH_SECTOR_SIZE)
		{
			sector_bytes = FLASH_SECTOR_SIZE;	// 下一个扇区写不完
		}
		else
		{
			sector_bytes = length;						// 下一个扇区可写完
		}
	}
	// 上锁
	FLASH_Lock();
}



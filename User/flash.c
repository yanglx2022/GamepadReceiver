/**********************
* ��дFLASH
**********************/
#include "flash.h"

// д����ʱ�õ��Ļ���
static uint8_t FLASH_BUFFER[FLASH_SECTOR_SIZE];


// ��ָ����ַ��ʼ������
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

// ��ָ����ַ��ʼд����(address��length����Ϊ2�ı���)
void Flash_Write(uint32_t address, const uint8_t* buffer, uint32_t length)
{
	// �Ƿ���ַ
	if (address < FLASH_BASE || (address + length) > (FLASH_BASE + 1024 * FLASH_SIZE))
	{
		return;
	}
	uint32_t sector_addr = (address - FLASH_BASE) / FLASH_SECTOR_SIZE;		// �������
	uint32_t sector_offset = (address - FLASH_BASE) % FLASH_SECTOR_SIZE;	// ������ƫ��
	uint32_t sector_bytes = FLASH_SECTOR_SIZE - sector_offset;						// Ҫд���������ֽ���
	if (sector_bytes > length)
	{
		sector_bytes = length;
	}
	// ����
	FLASH_Unlock();
	// ������д
	while(1)
	{
		Flash_Read(sector_addr * FLASH_SECTOR_SIZE + FLASH_BASE, FLASH_BUFFER, FLASH_SECTOR_SIZE);	// ����������
		uint8_t erase = 0;
		for(int i = 0; i < sector_bytes; i++)		// У������
		{
			if (FLASH_BUFFER[i + sector_offset] != 0xFF)
			{
				erase = 1;	// ��Ҫ����
				break;
			}
		}
		if (erase == 1)	// ��Ҫ����
		{
			FLASH_ErasePage(sector_addr * FLASH_SECTOR_SIZE + FLASH_BASE);	// ��������
			for(int i = 0; i < sector_bytes; i++)					// ��������
			{
				FLASH_BUFFER[i + sector_offset] = buffer[i];	  
			}
			for(int i = 0; i < FLASH_SECTOR_SIZE; i += 2)	// д����������
			{
				FLASH_ProgramHalfWord(sector_addr * FLASH_SECTOR_SIZE + FLASH_BASE + i, *(uint16_t*)(FLASH_BUFFER + i));
			}
		}
		else						// ����Ҫ����
		{
			for(int i = 0; i < sector_bytes; i += 2)			// д������
			{
				FLASH_ProgramHalfWord(address + i, *(uint16_t*)(buffer + i));
			}
		}
		if (sector_bytes == length)  // 	���д��
		{
			break;
		}
		sector_addr++;								// ���������1
		sector_offset = 0;            // ������ƫ����0 
		buffer += sector_bytes;       // ����ָ��ƫ��
		address += sector_bytes;  		// д��ַƫ��	 
		length -= sector_bytes;       // ����ʣ���ֽ���
		if (length > FLASH_SECTOR_SIZE)
		{
			sector_bytes = FLASH_SECTOR_SIZE;	// ��һ������д����
		}
		else
		{
			sector_bytes = length;						// ��һ��������д��
		}
	}
	// ����
	FLASH_Lock();
}



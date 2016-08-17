#define EEPROMaddress 0x57
#include <Arduino.h>
#include <Wire.h>
#include "eeprom_mio.h"
#ifndef EEPROM_ESP
/*
//#include <I2C_eeprom.h>
uint8_t eeprom_read_byte(unsigned char * pos)
//{return EE.readByte((int)pos);}
// Read one byte from the EEPROM
//  uint8_t readEEPROM(uint8_t EEPROMaddress, uint8_t page, uint8_t entry)
//    uint16_t pageEntryAddress = (uint16_t)((uint16_t)page << 5) | entry;
//    uint8_t  highAddressByte = (uint8_t)(pageEntryAddress >> 8);  // byte with the four MSBits of the address
//    uint8_t  lowAddressByte = (uint8_t)(pageEntryAddress - ((uint16_t)highAddressByte << 8)); // byte with the eight LSbits of the address
{
	uint16_t highAddressByte = (int)pos >> 8;
	uint16_t  lowAddressByte = (int)pos & 0xFF;
	uint8_t data = 0xFF;                                    // `data` will store the register data   
	Wire.beginTransmission(EEPROMaddress);           // Initialize the Tx buffer
	Wire.write(highAddressByte);                     // Put slave register address 1 in Tx buffer
	Wire.write(lowAddressByte);                      // Put slave register address 2 in Tx buffer
	Wire.endTransmission(false);                     // Send the Tx buffer, but send a restart to keep connection alive
	Wire.requestFrom(EEPROMaddress, 1);              // Read one byte from slave register address 
	delay(10);                                       // maximum write cycle time per data sheet
	if (Wire.available()) data = Wire.read();                              // Fill Rx buffer with result
	delay(10);
	return data;                                     // Return data read from slave register
}


uint16_t eeprom_read_word(const uint16_t * pos) // Read one word from the EEPROM uint8_t readEEPROM(uint8_t EEPROMaddress, uint8_t page, uint8_t entry)
{
	//    uint16_t pageEntryAddress = (uint16_t)((uint16_t)page << 5) | entry;
	//    uint8_t  highAddressByte = (uint8_t)(pageEntryAddress >> 8);  // byte with the four MSBits of the address
	//    uint8_t  lowAddressByte = (uint8_t)(pageEntryAddress - ((uint16_t)highAddressByte << 8)); // byte with the eight LSbits of the address
	uint16_t highAddressByte = (int)*pos >> 8;
	uint16_t  lowAddressByte = (int)*pos & 0xFF;
	uint8_t data = 0xFF;                                    // `data` will store the register data   
	Wire.beginTransmission(EEPROMaddress);           // Initialize the Tx buffer
	Wire.write(highAddressByte);                     // Put slave register address 1 in Tx buffer
	Wire.write(lowAddressByte);                      // Put slave register address 2 in Tx buffer
	Wire.endTransmission(false);                     // Send the Tx buffer, but send a restart to keep connection alive
	Wire.requestFrom(EEPROMaddress, 1);             // Read one byte from slave register address 
	delay(10);                                       // maximum write cycle time per data sheet
	if (Wire.available()) data = Wire.read();                              // Fill Rx buffer with result
	delay(10);
	return data;                                     // Return data read from slave register
}

void eeprom_write_byte(uint8_t * pos, uint8_t data) { // Write one byte to the EEPROM
													  // {EE.writeByte((int)pos ,data);}
													  //  void writeEEPROM(uint8_t EEPROMaddress, uint8_t page, uint8_t entry, uint8_t data){
													  // Construct EEPROM address from page and entry input
													  // There are 128 pages and 32 entries (bytes) per page
													  // EEPROM address are 16-bit (2 byte) address where the MS four bits are zero (or don't care)
													  // the next seven MS bits are the page and the last five LS bits are the entry location on the page
													  //uint16_t pageEntryAddress = (uint16_t)((uint16_t)page << 5) | entry;
													  //uint8_t  highAddressByte = (uint8_t)(pageEntryAddress >> 8);  // byte with the four MSBits of the address
													  //uint8_t  lowAddressByte = (uint8_t)(pageEntryAddress - ((uint16_t)highAddressByte << 8)); // byte with the eight LSbits of the address

	uint16_t highAddressByte = (int)pos >> 8;
	uint16_t  lowAddressByte = (int)pos & 0xFF;

	Wire.beginTransmission(EEPROMaddress);    // Initialize the Tx buffer
	Wire.write(highAddressByte);              // Put slave register address 1 in Tx buffer
	Wire.write(lowAddressByte);               // Put slave register address 2 in Tx buffer
	Wire.write(data);                         // Put data in Tx buffer
	delay(10);                                // maximum write cycle time per data sheet
	Wire.endTransmission();                   // Send the Tx buffer
	delay(10);

}
void eeprom_read_block(
	void * __dst,
	const void * __src,
	size_t __n) //Read a block of __n bytes from EEPROM address __src to SRAM __dst.
{
	//	  EE.readBlock((int)__src, (uint8_t*)__dst, (uint16_t)__n);
	//  }

	// char  bufp[] = (char *)__dst;

	for (int i = 0; i < __n; i++) {

		*((char *)__dst + i) = eeprom_read_byte((uint8_t *)__src + i);

	}
}
void eeprom_write_block(
	const void * src,
	void * dst,
	size_t num) //Write a block of __n bytes to EEPROM address __dst from __src.
{
	//   EE.writeBlock((int)dst, (const uint8_t*)src,( const uint16_t) num);


	
	for (int i = 0; i < (int)num; i++) {

		


		eeprom_write_byte((uint8_t *)dst+i, *((char *)src+i));
		//Serial.println(eeprom_read_byte(EEpos));
		
	}

}*/
#define EEPROMaddress 0x57
//#include <I2C_eeprom.h>
uint8_t eeprom_read_byte(unsigned char * pos)
{//return EE.readByte((int)pos);}
 // Read one byte from the EEPROM
 //  uint8_t readEEPROM(uint8_t EEPROMaddress, uint8_t page, uint8_t entry)
	byte page = (int)pos / 32; byte entry = (int)pos % 32;
	int pageEntryAddress = (int)((int)page << 5) | entry;
	byte  highAddressByte = (byte)(pageEntryAddress >> 8);  // byte with the four MSBits of the address
	byte lowAddressByte = (byte)(pageEntryAddress - ((int)highAddressByte << 8)); // byte with the eight LSbits of the address

																							  //   uint16_t highAddressByte = (int)pos>>8;
																							  //    uint16_t  lowAddressByte = (int)pos & 0xFF;
	unsigned char data = 0xFF;                                    // `data` will store the register data   
	Wire.beginTransmission(EEPROMaddress);           // Initialize the Tx buffer
	Wire.write(highAddressByte);                     // Put slave register address 1 in Tx buffer
	Wire.write(lowAddressByte);                      // Put slave register address 2 in Tx buffer
	Wire.endTransmission(false);                     // Send the Tx buffer, but send a restart to keep connection alive
	Wire.requestFrom(EEPROMaddress, 1);              // Read one byte from slave register address 
	delay(10);                                       // maximum write cycle time per data sheet
	if (Wire.available()) data = Wire.read();                              // Fill Rx buffer with result
	delay(10);
	return data;                                     // Return data read from slave register
}


uint16_t eeprom_read_word(const int * pos) // Read one word from the EEPROM uint8_t readEEPROM(uint8_t EEPROMaddress, uint8_t page, uint8_t entry)
{
	//    uint16_t pageEntryAddress = (uint16_t)((uint16_t)page << 5) | entry;
	//    uint8_t  highAddressByte = (uint8_t)(pageEntryAddress >> 8);  // byte with the four MSBits of the address
	//    uint8_t  lowAddressByte = (uint8_t)(pageEntryAddress - ((uint16_t)highAddressByte << 8)); // byte with the eight LSbits of the address
	int highAddressByte = (int)*pos >> 8;
	int  lowAddressByte = (int)*pos & 0xFF;
	byte data = 0xFF;                                    // `data` will store the register data   
	Wire.beginTransmission(EEPROMaddress);           // Initialize the Tx buffer
	Wire.write(highAddressByte);                     // Put slave register address 1 in Tx buffer
	Wire.write(lowAddressByte);                      // Put slave register address 2 in Tx buffer
	Wire.endTransmission(false);                     // Send the Tx buffer, but send a restart to keep connection alive
	Wire.requestFrom(EEPROMaddress, 1);             // Read one byte from slave register address 
	delay(10);                                       // maximum write cycle time per data sheet
	if (Wire.available()) data = Wire.read();                              // Fill Rx buffer with result
	delay(10);
	return data;                                     // Return data read from slave register
}

void eeprom_write_byte(unsigned char * pos, unsigned char data) { // Write one byte to the EEPROM
													  //EE.writeByte((int)pos ,data);}
													  //  void writeEEPROM(uint8_t EEPROMaddress, uint8_t page, uint8_t entry, uint8_t data){
													  // Construct EEPROM address from page and entry input
													  // There are 128 pages and 32 entries (bytes) per page
													  // EEPROM address are 16-bit (2 byte) address where the MS four bits are zero (or don't care)
													  // the next seven MS bits are the page and the last five LS bits are the entry location on the page
	byte page = (int)pos / 32; byte entry = (int)pos % 32;
	int pageEntryAddress = (int)((int)page << 5) | entry;
	byte  highAddressByte = (byte)(pageEntryAddress >> 8);  // byte with the four MSBits of the address
	byte  lowAddressByte = (byte)(pageEntryAddress - ((int)highAddressByte << 8)); // byte with the eight LSbits of the address

																							  //uint16_t highAddressByte = (int)pos >> 8;
																							  //uint16_t  lowAddressByte = (int)pos & 0xFF;

	Wire.beginTransmission(EEPROMaddress);    // Initialize the Tx buffer
	delay(5);
	Wire.write(highAddressByte);              // Put slave register address 1 in Tx buffer
	Wire.write(lowAddressByte);               // Put slave register address 2 in Tx buffer
	Wire.write(data);                         // Put data in Tx buffer
	delay(20);                                // maximum write cycle time per data sheet
	Wire.endTransmission();                   // Send the Tx buffer
	delay(10);

}

void eeprom_read_block(
	void * __dst,
	const void * __src,
	unsigned int __n) //Read a block of __n bytes from EEPROM address __src to SRAM __dst.
{
	//	  EE.readBlock((int)__src, (uint8_t*)__dst, (uint16_t)__n);
	//  }

	// char  bufp[] = (char *)__dst;

	for (int i = 0; i < __n; i++) {

		*((char *)__dst + i) = eeprom_read_byte((byte *)__src + i);

	}
}
void eeprom_write_address(int page,int entry){
	int pageEntryAddress = (int)((int)page << 5) | entry;
	byte  highAddressByte = (byte)(pageEntryAddress >> 8);  // byte with the four MSBits of the address
	byte  lowAddressByte = (byte)(pageEntryAddress - ((int)highAddressByte << 8)); // byte with the eight LSbits of the address

	Wire.beginTransmission(EEPROMaddress);    // Initialize the Tx buffer
	delay(5);
	Wire.write(highAddressByte);              // Put slave register address 1 in Tx buffer
	Wire.write(lowAddressByte);               // Put slave register address 2 in Tx buffer
}
void eeprom_write_block(
	const void * src,
	void * dst,
	unsigned int num) //Write a block of __n bytes to EEPROM address __dst from __src.
{
	//   EE.writeBlock((int)dst, (const uint8_t*)src,( const uint16_t) num);


	//char * bufp = (char *)src;
	//uint8_t * EEpos = (uint8_t *)dst;
	int lastp; unsigned char c;
	  int i = 0;
	  int ip = 0;
	  while(i < (int)num) {
		
		int pos = (int)dst + i;
	//	unsigned char data = *((unsigned char *)src + i);
	    byte page = (int)pos / 32; byte entry = (int)pos % 32;
		eeprom_write_address(page, entry);
		int j = entry;
		while (j < 32 &&  i < int(num))
		{   j++;
			c = *((unsigned char *)src +i++  );
			Wire.write(c);
			if (i > 28 + ip) {  // wire buffer limit to 32 bytes
				ip = i;
				delay(20);
				Wire.endTransmission();
				delay(10);
				int pos = (int)dst + i;
				byte page = (int)pos / 32; byte entry = (int)pos % 32;
				eeprom_write_address(page, entry);
			}
		}
			
		                       // Put data in Tx buffer
		                                         // Put data in Tx buffer
		delay(20);                                // maximum write cycle time per data sheet
		Wire.endTransmission();                   // Send the Tx buffer
		
		delay(10);
	//	if (eeprom_read_byte((unsigned char*)((int)dst + lastp)) != c) Serial.print(c);

//		eeprom_write_byte((unsigned char *)dst+i, c);
	//	Serial.print(int(EEpos)); Serial.print('  ');Serial.println(eeprom_read_byte(EEpos));
		
	}

}
#endif
#pragma once

//NOTE:请确保靠前的是4字节长的，中间的是2字节，最后是1字节
typedef enum Register
{
	eax=0x00, ebx, ecx, edx, esp, ebp, esi, edi, 
	ax=0x10, bx, cx, dx, sp, bp, si, di,
	al=0x20, bl, cl, dl, 
	ah=0x30, bh, ch, dh,
} Register;

extern const unsigned int REG4_LEN;
extern Register REG4[];


extern const unsigned int REG2_LEN;
extern Register REG2[];

extern const unsigned int REG1_LEN;
extern Register REG1[];

extern const unsigned int REG_ALL_LEN;
extern Register REG_ALL[];

unsigned int registerLength(Register reg);

unsigned int registerOffset(Register reg);//将32位寄存器依次排列，返回某一寄存器的偏移量，比如说ax就返回0, ah就返回1， ebx返回4, 必须从0开始，紧密排列

Register stringToRegister(const char * str, unsigned int length = 0);
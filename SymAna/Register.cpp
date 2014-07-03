#include "Common.h"
#include "Register.h"

const unsigned int REG4_LEN = 8;
Register REG4[REG4_LEN] = { eax, ebx, ecx, edx, esp, ebp, esi, edi };

const unsigned int REG2_LEN = 8;
Register REG2[REG2_LEN] = { ax, bx, cx, dx, sp, bp, si, di };

const unsigned int REG1_LEN = 8;
Register REG1[REG1_LEN] = { al, bl, cl, dl, ah, bh, ch, dh };

const unsigned int REG_ALL_LEN = REG4_LEN + REG2_LEN + REG1_LEN;
Register REG_ALL[REG_ALL_LEN] = {
	eax, ebx, ecx, edx, esp, ebp, esi, edi,
	ax, bx, cx, dx, sp, bp, si, di,
	al, bl, cl, dl, ah, bh, ch, dh,
};

unsigned int registerLength(Register reg)
{
	switch (char(reg) & 0xf0)
	{
	case 0x00:
		return 4;
	case 0x10:
		return 2;
	case 0x20: case 0x30:
		return 1;
	default:
		printf("%d", reg);
		throw Error("registerLength Error");
	}
}

unsigned int registerOffset(Register reg)//将32位寄存器依次排列，返回某一寄存器的偏移量，比如说ax就返回0, ah就返回1， ebx返回4, 必须从0开始，紧密排列
{
	switch (reg & 0xf0)
	{
	case 0x00: case 0x10: case 0x20:
		return (reg & 0x0f) * 4;
	case 0x30:
		return (reg & 0x0f) * 4 + 1;
	default:
		throw Error("registerOffset Error");
	}
}

Register stringToRegister(const char * str, unsigned int length)
{
	if (length == 0) length = strlen(str);
	switch (length)
	{
	case 2:
		switch (str[1])
		{
		case 'x':
			switch (str[0]){
			case 'a':	return ax;
			case 'b':	return bx;
			case 'c':	return cx;
			case 'd':	return dx;
			}break;
		case 'h':
			switch (str[0]){
			case 'a':	return ah;
			case 'b':	return bh;
			case 'c':	return ch;
			case 'd':	return dh;
			}break;
		case 'l':
			switch (str[0]){
			case 'a':	return al;
			case 'b':	return bl;
			case 'c':	return cl;
			case 'd':	return dl;
			}break;
		case 'p':
			switch (str[0]){
			case 'b':	return bp;
			case 's':	return sp;
			}break;
		case 'i':
			switch (str[0]){
			case 'd':	return di;
			case 's':	return si;
			}break;
		}
		break;
	case 3:
		if (str[0] != 'e') break;
		switch (str[2])
		{
		case 'x':
			switch (str[1]){
			case 'a':return eax;
			case 'b':return ebx;
			case 'c':return ecx;
			case 'd':return edx;
			}break;
		case 'p':
			switch (str[1]){
			case 'b':return ebp;
			case 's':return esp;
			}break;
		case 'i':
			switch (str[1]){
			case 'd':return edi;
			case 's':return esi;
			}break;
		}
		break;
	}
	throw Error("stringToRegister: not string match a register");
}
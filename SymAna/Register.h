#pragma once

//NOTE:��ȷ����ǰ����4�ֽڳ��ģ��м����2�ֽڣ������1�ֽ�
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

unsigned int registerOffset(Register reg);//��32λ�Ĵ����������У�����ĳһ�Ĵ�����ƫ����������˵ax�ͷ���0, ah�ͷ���1�� ebx����4, �����0��ʼ����������

Register stringToRegister(const char * str, unsigned int length = 0);
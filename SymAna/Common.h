#pragma once
#include <string>
#define DEBUG             //����=���������Ϣ
//#define RELEASE           //����=����һЩ������ı߽��飬���ٿ���
#define IMM_AS_NONE
//#define MEMLEAK_CHECK
class Error
{
public:
	Error(const char * message) :message(message){
		printf(message);
	};
	std::string message;
};
#define MAX_NUM_OPERANDS (4)
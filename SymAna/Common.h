#pragma once
#include <string>
#define DEBUG             //开启=输出调试信息
//#define RELEASE           //开启=避免一些无意义的边界检查，减少开销
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
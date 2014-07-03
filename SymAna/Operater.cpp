#include "Operater.h"
#include "Common.h"
void operater2operaterStr(char * operaterStr, const Operater& operater)
{
	switch (operater)
	{
	case ADD:	strcpy(operaterStr, "add");		break;
	case SUB:	strcpy(operaterStr, "sub");		break;
	case DEC:	strcpy(operaterStr, "dec");		break;
	case INC:	strcpy(operaterStr, "inc");		break;

	case AND:	strcpy(operaterStr, "and");		break;
	case OR:	strcpy(operaterStr, "or");		break;
	case XOR:	strcpy(operaterStr, "xor");		break;
	case MUL:	strcpy(operaterStr, "mul");		break;
	case DIV:	strcpy(operaterStr, "div");		break;
	case MOD:	strcpy(operaterStr, "mod");		break;
	case SHL:	strcpy(operaterStr, "shl");		break;
	case SHR:	strcpy(operaterStr, "shr");		break;
	case ROR:	strcpy(operaterStr, "ror");		break;
	case ROL:	strcpy(operaterStr, "rol");		break;
	case NEG:	strcpy(operaterStr, "neg");		break;
	case NOT:	strcpy(operaterStr, "net");		break;
	case DEREF:	strcpy(operaterStr, "DEREF");		break;

	case CONN:	strcpy(operaterStr, "CONN");		break;
	case SLICE_0:	strcpy(operaterStr, "SLICE_0");		break;
	case SLICE_1:	strcpy(operaterStr, "SLICE_1");		break;
	case SLICE_2:	strcpy(operaterStr, "SLICE_2");		break;
	case SLICE_3:	strcpy(operaterStr, "SLICE_3");		break;
	case SLICE_01:	strcpy(operaterStr, "SLICE_01");		break;
	case SLICE_23:	strcpy(operaterStr, "SLICE_23");		break;
#ifndef RELEASE:
	default:
		throw Error("operater2operaterStr Error");
	}
#endif
}
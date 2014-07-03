#include "Common.h"
#include "Expression.h"
#include "Operand.h"
#include "Operater.h"
#include "Register.h"
#include <stdio.h>
#include <stdarg.h>

void (*Expression::onConstruct)(Expression *) = nullptr;
void (*Expression::onUnconstruct)(Expression *) = nullptr;
void (*Expression::onDecRefCount)(Expression *) = nullptr;
Expression Expression::noneExpression;
void Expression::init()
{
	noneExpression.addRefCount();
	noneExpression.addRefCount();
	noneExpression.addRefCount();
	onConstruct(&noneExpression);
}

Expression * Expression::create(const Operater& operater, unsigned int num_operands, ...)
{
	va_list ap;
	va_start(ap, num_operands);

	Expression * operands[MAX_NUM_OPERANDS];
	int i = 0;
	while (i < num_operands)
	{
		operands[i++] = va_arg(ap, Expression *);
	}
	va_end(ap);

	unsigned int num = num_operands;
	for (i = 0; i < num_operands; i++)
	{
		if (operands[i]->type != ExpressionType::NONE) break;
	}
	if (i == num_operands) return Expression::create(nullptr);
	return new Expression(operater, num_operands, (const Expression **)operands);
}
Expression * Expression::create(const Operand& operand)
{
	switch (operand.type)
	{
	case OperandType::IMM:
#ifdef IMM_AS_NONE
		return create(nullptr);
#endif
	case OperandType::DIRECT_MEM: case OperandType::REG:
		return new Expression(operand);
#ifndef RELEASE
	default:
		throw Error("Expression::create Error");
#endif
	}
}
Expression* Expression::create(Expression * expression)
{
	return (expression ? expression : &noneExpression);
}

Expression::Expression(const Operater& operater, unsigned int num_operands, const Expression ** operands)
{
	this->operater = operater;
	this->num_operands = num_operands;
	memcpy(this->operands, operands, sizeof(Expression *)* num_operands);
	for (unsigned int i = 0; i < num_operands; i++)
	{
		this->operands[i]->addRefCount();
		this->operands[i]->parents.insert(this);
	}
	this->type = ExpressionType::EXPRESSION;

	onConstruct(this);
}
const char * Expression::getStr()
{
	char retv[10240000];
	char * buf = retv;
	char tmp[1024];
	buf[0] = '\0';
	std::list<Expression *> list;
	list.push_back(this);
	unsigned int len = 0;
	while (!list.empty())
	{
		Expression * p = list.front();
		list.pop_front();
		if (p->type == ExpressionType::VALUE)
		{
			switch (p->operand.type)
			{
			case IMM:
				sprintf(tmp, " IMM-%d", p->operand.value);
				break;
			case DIRECT_MEM:
				sprintf(tmp, " [%08x %d]", p->operand.value, p->operand.length);
				break;
			default:
				throw Error("Unfinished");
			}
			strcat(buf, tmp);
		}
		else if (p->type == ExpressionType::NONE)
		{
			strcat(buf, " NONE");
		}
		else
		{
			char buffer[20];
			operater2operaterStr(buffer, p->operater);
			sprintf(tmp, " <%s>", buffer);
			strcat(buf, tmp);
			for (int i = p->num_operands - 1; i >= 0; i--)
			{
				list.push_front(p->operands[i]);
			}
		}
		len += strlen(buf);
		buf += strlen(buf);
		if (len > 10240000) throw Error("Expression::getStr buf is not enough");
	}
	return buf;
}
Expression::Expression(const Operand& operand)
{
	this->num_operands = 0;
	this->operand = operand;
	this->type = ExpressionType::VALUE;

	if (onConstruct)
		onConstruct(this);
}
Expression::Expression()
{
	this->num_operands = 0;
	this->type = ExpressionType::NONE;

	if (onConstruct)
		onConstruct(this);
}
Expression::~Expression()
{
	if (onUnconstruct) onUnconstruct(this);
	if (this->type == ExpressionType::EXPRESSION)
	for (unsigned int i = 0; i < num_operands; i++)
	{
		auto it = operands[i]->parents.find(this);
		if (it != operands[i]->parents.end())
			operands[i]->parents.erase(it);
		operands[i]->decRefCount();
	}
}
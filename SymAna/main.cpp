#include "SymbolicExecute.h"
#include "SymbolicMemory.h"
#include "ITraceConcreteMemory.h"
#include "Expression.h"
#include "stdio.h"
#include <string>
#include <unordered_set>
#include <iostream>
#include "ItraceParse.h"
#include "Expression.h"

std::unordered_set<int> addressHasBeenWritten;
void beforeGet(SymbolicMemory * symbolicMemory, unsigned int address, unsigned int memoryLength)
{
	if (address < 100) return;
	bool hasNotBeenWritten[4] = { false, false, false, false };
	unsigned int i;
	for (i = 0; i < memoryLength; i++)
	{
		if (addressHasBeenWritten.find(address + i) == addressHasBeenWritten.end())
			hasNotBeenWritten[i] = true;
	}
	if (memoryLength >= 4 && hasNotBeenWritten[0] && hasNotBeenWritten[1] && hasNotBeenWritten[2] && hasNotBeenWritten[3])
	{
		symbolicMemory->put(address, 4, Expression::create(Operand(DIRECT_MEM, address, 4)));
		return;
	}
	if (memoryLength >= 2 && hasNotBeenWritten[0] && hasNotBeenWritten[1])
	{
		symbolicMemory->put(address, 2, Expression::create(Operand(DIRECT_MEM, address, 2)));
		hasNotBeenWritten[0] = false;
		hasNotBeenWritten[1] = false;
	}
	if (memoryLength >= 4 && hasNotBeenWritten[2] && hasNotBeenWritten[3])
	{
		symbolicMemory->put(address + 2, 2, Expression::create(Operand(DIRECT_MEM, address + 2, 2)));
		hasNotBeenWritten[2] = false;
		hasNotBeenWritten[3] = false;
	}
	for (unsigned int i = 0; i < memoryLength; i++)
	{
		if (hasNotBeenWritten[i])
		{
			symbolicMemory->put(address + i, 1, Expression::create(Operand(DIRECT_MEM, address + i, 1)));
		}
	}
}
void beforePut(SymbolicMemory * symbolicMemory, unsigned int address, unsigned int memoryLength, Expression * expreesion)
{
	for (unsigned int i = address; i < address + memoryLength; i++)
		addressHasBeenWritten.insert(i);
}
std::unordered_set<Expression *> expression_set;
std::unordered_set<Expression *> new_expression_set;
void onConstruct(Expression * expression)
{
	new_expression_set.insert(expression);
	if (expression->type == Expression::ExpressionType::VALUE && expression->operand.type == DIRECT_MEM)
	{
		expression->deep = 0;
	}
	else if (expression->type == Expression::ExpressionType::VALUE)
	{
		expression->deep = -1;
	}
	else
	{
		for (int i = 0; i < expression->num_operands; i++)
		{
			if (expression->operands[i]->deep >= 0)
				expression->deep = (expression->operands[i]->deep + 1 > expression->deep ?
				expression->operands[i]->deep + 1 : expression->deep);
		}
	}
}
void onUnconstruct(Expression * expression)
{
	if (expression == &Expression::noneExpression) return;
	if (expression_set.find(expression) != expression_set.end())
		expression_set.erase(expression);
	if (new_expression_set.find(expression) != new_expression_set.end())
		new_expression_set.erase(expression);
}
void onDecRefCount(Expression * expression)
{
}
unsigned int relatedMemSize(Expression * expression)
{
	std::list<Expression *> list;
	std::unordered_set<Expression *> answer_set, set;
	list.push_back(expression);
	set.insert(expression);
	while (!list.empty())
	{
		Expression * exp = list.front();
		list.pop_front();

		switch (exp->type)
		{
		case Expression::ExpressionType::VALUE:
			if (exp->operand.type == DIRECT_MEM)
				answer_set.insert(exp);
			set.insert(exp);
			break;
		case Expression::ExpressionType::EXPRESSION:
			for (int i = 0; i < exp->num_operands; i++)
			{
				if (set.find(exp->operands[i]) == set.end())
				{
					set.insert(exp->operands[i]);
					list.push_back(exp->operands[i]);
				}
			}
			break;
		case Expression::ExpressionType::NONE:
			set.insert(exp);
			break;
		default:
			throw Error("relatedSize error");
		}
	}
	unsigned retv = 0;
	for (auto i = answer_set.begin(); i != answer_set.end(); i++)
	{
		retv += (*i)->operand.length;
	}
	return retv;
}
std::vector<Operand> relatedMemSet(Expression * expression)
{
	std::list<Expression *> list;
	std::unordered_set<Expression *> answer_set, set;
	list.push_back(expression);
	set.insert(expression);
	while (!list.empty())
	{
		Expression * exp = list.front();
		list.pop_front();

		switch (exp->type)
		{
		case Expression::ExpressionType::VALUE:
			if (exp->operand.type == DIRECT_MEM)
				answer_set.insert(exp);
			set.insert(exp);
			break;
		case Expression::ExpressionType::EXPRESSION:
			for (int i = 0; i < exp->num_operands; i++)
			{
				if (set.find(exp->operands[i]) == set.end())
				{
					set.insert(exp->operands[i]);
					list.push_back(exp->operands[i]);
				}
			}
			break;
		case Expression::ExpressionType::NONE:
			set.insert(exp);
			break;
		default:
			throw Error("relatedMemSet error");
		}
	}
	std::vector<Operand> retv;
	for (auto i = answer_set.begin(); i != answer_set.end(); i++)
	{
		retv.push_back((*i)->operand);
	}
	return retv;
}
int main(int argc, char ** argv)
{
	if (argc != 2 && argc != 3)
	{
		printf("Usage: %s itracePath.log [outPath.log]");
		return 0;
	}
	char inPath[1024];
	char outPath[1024];

	strcpy(inPath, argv[1]);
	if (argc == 2)
	{
		strcpy(outPath, inPath);
		strcat(outPath, ".output.txt");
	}
	else
	{
		strcpy(outPath, argv[2]);
	}
	SymbolicMemory::beforeGet = beforeGet;
	SymbolicMemory::beforePut = beforePut;
	//Expression::onDecRefCount = onDecRefCount;
	Expression::onConstruct = onConstruct;
	Expression::onUnconstruct = onUnconstruct;

	SymbolicMemory * symbolicMemory = new SymbolicMemory();
	ConcreteMemory * concreteMemory = new ITraceConcreteMemory();
	FILE * fp = fopen(inPath, "r");

	int lineNo = 0;
	char lineBuffer[1024];

	Expression::init();
	while (fgets(lineBuffer, 1023, fp))
	{
		Instruction inst;
		lineNo += 1;

		itraceParse(&inst, lineBuffer, concreteMemory);
		//SymbolicExecute!!

		SymbolicExecute(inst, symbolicMemory, concreteMemory);
		//将新生成的expression 中没有被使用的清理掉
		std::unordered_set<Expression *> temp_set;
		for (auto i = new_expression_set.begin(); i != new_expression_set.end(); i++)
		{
			if ((*i)->refCount <= 0)
			{
				temp_set.insert(*i);
			}
			else
			{
				expression_set.insert(*i);
			}
		}
		for (auto i = temp_set.begin(); i != temp_set.end(); i++)
		{
			(*i)->addRefCount();
			(*i)->decRefCount();
		}
		new_expression_set.clear();

		if (lineNo % 1000 == 0)
		{
			printf("%d\n", lineNo);
			std::unordered_set<Expression *>temp_set;
#ifdef MEMLEAK_CHECK
			printf("MEMLEAK_CHECK\n");
			std::list<Expression *> list;
			std::unordered_set<Expression * > set;
			for (int i = 0; i <= 2; i++)
			for (auto j = symbolicMemory->memory[i].begin(); j != symbolicMemory->memory[i].end(); j++)
			{
				list.push_back(j->second);
				set.insert(j->second);
			}
			while (list.size())
			{
				Expression * exp = list.back();
				list.pop_back();
				for (int i = 0; i < exp->num_operands; i++)
				{
					if (set.find(exp->operands[i]) == set.end())
					{
						set.insert(exp->operands[i]);
						list.push_back(exp->operands[i]);
					}
				}
			}
			printf("%d, %d\n", set.size(), expression_set.size());
#endif
			//char buf[2]; gets(buf);
		}
	}
	FILE * fp2 = fopen(outPath, "w+");
	for (int i = 0; i <= 2; i++)
	{
		int s = 0;
		for (auto j = symbolicMemory->memory[i].begin(); j != symbolicMemory->memory[i].end(); j++)
		{
			s += 1;
			if (j->second->deep >= 2)
			{
				int relatedSize = relatedMemSize(j->second);
				fprintf(fp2, "[%08x:2^%d] %d %d %lf\n", j->first, i, j->second->deep, relatedSize,
					(double)relatedSize / addressHasBeenWritten.size());
				printf("[%08x:2^%d] %d %d %lf\n", j->first, i, j->second->deep, relatedSize,
					(double)relatedSize / addressHasBeenWritten.size());
				auto v = relatedMemSet(j->second);
				for (int i = 0; i < v.size(); i++)
				{
					for (int j = i + 1; j < v.size(); j++)
					{
						if (v[i].value > v[j].value)
						{
							auto t = v[i];
							v[i] = v[j];
							v[j] = t;
						}
					}
				}
				for (int i = 0; i < v.size(); i++)
					fprintf(fp2, "[%08x:%d] ", v[i].value, v[i].length);
				fflush(fp2);

				fprintf(fp2, "\n");
			}
			if (s % 1000 == 0)
				printf("%d/%d/%d\n", s, symbolicMemory->memory[i].size(), i);
			//((std::unordered_set<Expression *>*)j->second->data)->size());
		}
	}
	int totalUsedByte = 0;
	{//统计总使用的内存数量
		std::list<Expression *> list;
		std::unordered_set<Expression * > set;
		std::unordered_set<unsigned int> memAddress_set;
		for (int i = 0; i <= 2; i++)
		for (auto j = symbolicMemory->memory[i].begin(); j != symbolicMemory->memory[i].end(); j++)
		{
			if (j->second->deep > 1)
			{
				list.push_back(j->second);
				set.insert(j->second);
			}
		}
		while (list.size())
		{
			Expression * exp = list.back();
			list.pop_back();
			if (exp->type == Expression::ExpressionType::VALUE && exp->operand.type == DIRECT_MEM)
			{
				for (int i = 0; i < exp->operand.length; i++)
				{
					memAddress_set.insert(exp->operand.value + i);
				}
			}
			for (int i = 0; i < exp->num_operands; i++)
			{
				if (set.find(exp->operands[i]) == set.end())
				{
					set.insert(exp->operands[i]);
					list.push_back(exp->operands[i]);
				}
			}
		}
		totalUsedByte = memAddress_set.size();
	}
	fprintf(fp2, "TOTAL_INPUT_BYTE:%d\n", addressHasBeenWritten.size());
	fprintf(fp2, "USED_INPUT_BYTE:%d\n", totalUsedByte);
	fprintf(fp2, "USED_RATE:%lf\n", (double)totalUsedByte / addressHasBeenWritten.size());

	fclose(fp2);
	printf("FIN\n");
	return 0;
}
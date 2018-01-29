#include "ARM_engine.h" 
#include "vm.h"
#include <stdlib.h>
#include <stdio.h>

#define MOV	13
#define ADD	4
#define SUB	2
#define MUL	0
#define CMP	10
/*
union{
	DataProcessing dataproc;
	Multiply multi;//union의 필요성
}oper;*/


void mov(VCPU* v){//mov의 경우 I,R을 분류x
printf("mov 명령어\n");
DataProcessing* instr = (DataProcessing*)((v->instr_addr));
printf("instruction is %llx\n",*instr);
//unsigned int result = shift(v,instr->operand2,instr->I);
unsigned int result = shift(v,(unsigned int)instr->operand2,instr->I);
v->regs[instr->Rd] = result;
v->regs[15]-=4;
printf("v->regs[instr->Rd]: %llu\n",v->regs[instr->Rd]);
}

void add(VCPU* v){
printf("add 명령어\n");
DataProcessing* instr;
instr=(DataProcessing*)((unsigned long)(v->instr_addr));
printf("instruction is %llx\n",*instr);
unsigned long long temp = (unsigned long long)shift(v,instr->operand2,instr->I);
//doodu
//if((instr->Rn == 11) || (instr->Rn == 13)) temp*=2;
v->regs[instr->Rd] = v->regs[instr->Rn] + temp;
if(instr->S==1)
        cpsrUpdate(v,v->regs[instr->Rn],temp,v->regs[instr->Rd]);
v->regs[15]-=4;
printf("regs[instr->Rd] = %llx\n",v->regs[instr->Rd]);
}

void sub(VCPU* v){
printf("sub 명령어\n");
DataProcessing* instr;
instr = (DataProcessing*)((unsigned long)(v->instr_addr));
printf("instruction is %llx\n",*instr);
int temp = ~shift(v,instr->operand2,instr->I)+1;
//if((instr->Rn == 11) || (instr->Rn == 13)) temp*=2;
v->regs[instr->Rd] = v->regs[instr->Rn]+temp;
if(instr->S==1)
        cpsrUpdate(v,v->regs[instr->Rn],temp,v->regs[instr->Rd]);
printf("regs[instr->Rd] = %llx\n",v->regs[instr->Rd]);
v->regs[15]-=4;
}

void cmp(VCPU *v){
printf("cmp 명령어\n");
DataProcessing* instr;
instr = (DataProcessing*)((unsigned long)(v->instr_addr));
printf("instruction is %llx\n",*instr);
long long result = v->regs[instr->Rn];
unsigned long long temp = ~shift(v,instr->operand2,instr->I)+1;//-가 아니라 ~를 쓰는 이유는 signed,unsigned의 범위 문제에 있다. -로 하면 안된다
result += temp;
printf(" cpsr.N : %hd\n", v->cpsr.N);
printf(" cpsr.Z : %hd\n", v->cpsr.Z);
printf(" cpsr.C : %hd\n", v->cpsr.C);
printf(" cpsr.V : %hd\n\n", v->cpsr.V);
cpsrUpdate(v,v->regs[instr->Rd],temp,result);
printf(" cpsr.N : %hd\n", v->cpsr.N);
printf(" cpsr.Z : %hd\n", v->cpsr.Z);
printf(" cpsr.C : %hd\n", v->cpsr.C);
printf(" cpsr.V : %hd\n", v->cpsr.V);
v->regs[15]-=4;
}

void mul(VCPU* v){
printf("mul 명령어\n");
Multiply* instr = (Multiply*)((unsigned long)(v->instr_addr));
        printf("instruction is %llx\n",*instr);
long long tmp = v->regs[instr->Rn];
tmp *= v->regs[instr->Rm];
v->regs[instr->Rd] = (unsigned long long)tmp;
printf(" v->regs[instr->Rd] = %llu\n");
v->regs[15]-=4;
}


void BX(VCPU* v){
        long long temp;
        BXWritePC* instr = (BXWritePC*)((v->instr_addr));
	if(!conditionPassed(v,instr->cond)){
		printf("couldn't passsed condition!!! \n");
		v->regs[15]-=4;
		//if(v->flag==1) insert_brkPoint(v,v->regs[15]);
		//insert_brkPoint(v,v->regs[15]);
		return;
	}
	printf("passed condition!!! \n");
        printf("instruction is %llx\n",*instr);
	v->regs[15] = v->regs[instr->Rm];
	//if(v->flag==1) insert_brkPoint(v,v->regs[15]);
	//if(v->flag==1) insert_brkPoint(v,v->regs[15]);
return;
}
 


void DataProcess(VCPU *v){
DataProcessing* instr;
instr = (DataProcessing*)((v->instr_addr));
/*
switch(instr->opcode){
	case MOV: mov(v); break;
	default: break;
}*/
if(instr->opcode == MOV) mov(v);
if(instr->opcode == ADD) add(v);
if(instr->opcode == SUB) sub(v);
if(instr->opcode == MUL) mul(v);
if(instr->opcode == CMP) cmp(v);
if((instr->opcode == 9) && (instr->S == 0) && (instr->Rn == 15) && (instr->Rd == 15) /*&& (instr->operand2 & 0xff0 == 0xf10)*/) BX(v);


}




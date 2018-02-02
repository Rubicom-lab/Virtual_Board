#include "VirtualMachine.h"
#include "DataProcessing.h"
#include "BitsHandler.h"
#include <stdio.h>

#define MOV	13
#define ADD	4
#define SUB	2
#define MUL	0
#define CMP	10

// mov의 경우 I,R을 분류x
void mov(struct VCPU* v){
    printf("mov 명령어\n");
    struct DataProcessing* instr = (struct DataProcessing*)((v->instr_addr));

    printf("instruction is %llx\n",*instr);
    unsigned int result = shift(v,(unsigned int)instr->operand2,instr->I);

    v->regs[instr->Rd] = result;
    v->regs[15]-=4;

    printf("v->regs[instr->Rd]: %llu\n",v->regs[instr->Rd]);
}

void add(struct VCPU* v){
    printf("add 명령어\n");
    struct DataProcessing* instr;
    instr = (struct DataProcessing*)((unsigned long)(v->instr_addr));
    printf("instruction is %llx\n",*instr);
    unsigned long long temp = (unsigned long long)shift(v,instr->operand2,instr->I);
    v->regs[instr->Rd] = v->regs[instr->Rn] + temp;

    if(instr->S==1)
        cpsrUpdate(v,v->regs[instr->Rn],temp,v->regs[instr->Rd]);

    v->regs[15]-=4;
    printf("regs[instr->Rd] = %llx\n",v->regs[instr->Rd]);
}

void sub(struct VCPU* v){
    printf("sub 명령어\n");
    struct DataProcessing* instr;
    instr = (struct DataProcessing*)((unsigned long)(v->instr_addr));
    printf("instruction is %llx\n",*instr);
    int temp = ~shift(v,instr->operand2,instr->I)+1;
    v->regs[instr->Rd] = v->regs[instr->Rn]+temp;

    if(instr->S==1)
        cpsrUpdate(v,v->regs[instr->Rn],temp,v->regs[instr->Rd]);

    printf("regs[instr->Rd] = %llx\n",v->regs[instr->Rd]);
    v->regs[15]-=4;
}

void cmp(struct VCPU *v){
    printf("cmp 명령어\n");
    struct DataProcessing* instr;
    instr = (struct DataProcessing*)((unsigned long)(v->instr_addr));
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

void mul(struct VCPU* v){
    printf("mul 명령어\n");
    struct Multiply* instr = (struct Multiply*)((unsigned long)(v->instr_addr));
    printf("instruction is %llx\n",*instr);

    long long tmp = v->regs[instr->Rn];
    tmp *= v->regs[instr->Rm];
    v->regs[instr->Rd] = (unsigned long long)tmp;
    printf(" v->regs[instr->Rd] = %llu\n");
    v->regs[15]-=4;
}


void BX(struct VCPU* v){
    long long temp;
    struct BXWritePC* instr = (struct BXWritePC*)((v->instr_addr));

    if(!conditionPassed(v,instr->cond)){
        printf("couldn't passsed condition!!! \n");
        v->regs[15]-=4;
        return;
    }

    printf("passed condition!!! \n");
    printf("instruction is %llx\n",*instr);
    v->regs[15] = v->regs[instr->Rm];
    return;
}



void dataProcess(struct VCPU *v){
    struct DataProcessing* instr;
    instr = (struct DataProcessing*)((v->instr_addr));

    if(instr->opcode == MOV) mov(v);
    if(instr->opcode == ADD) add(v);
    if(instr->opcode == SUB) sub(v);
    if(instr->opcode == MUL) mul(v);
    if(instr->opcode == CMP) cmp(v);
    if((instr->opcode == 9) && (instr->S == 0) && (instr->Rn == 15) && (instr->Rd == 15) /*&& (instr->operand2 & 0xff0 == 0xf10)*/) BX(v);
}




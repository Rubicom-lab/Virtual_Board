#include "ARM_engine.h"
#include "vm.h"
#include <stdlib.h>
#include <stdio.h>

void B(VCPU* v){
        long long temp;
        Branch* instr;
        instr=(Branch*)((v->instr_addr));
        printf("branch 명령어\n");
	printf("instruction is %llx\n",*instr);

	if(!conditionPassed(v,instr->cond)){
		printf("couldn't passsed condition!!! \n");
		v->regs[15]-=4;
		if(v->flag==1) insert_brkPoint(v,v->regs[15]);
		return;
	}

	printf("passed condition!!! \n");
        if (instr->L == 1) v->regs[14] = v->regs[15]-4;
        temp = instr->offset <<8;
        v->regs[15] += (temp >>6);
	
	if(v->flag==1) insert_brkPoint(v,v->regs[15]);
return;
}

/*
//BX()함수 정의는 Dataprocessing.c에 존재
void BX(VCPU* v){
        long long temp;
        BXWritePC* instr = (BXWritePC*)((v->instr_addr));
        printf("here is BXWritePC\n");
printf("v->regs[0] = %llx\n",v->regs[0]);
	if(!conditionPassed(v,instr->cond)){
		printf("couldn't passsed condition!!! \n");
		v->regs[15]-=4;
		return;
	}
	printf("passed condition!!! \n");
	printf("v->regs[14],(lr) = %llx\n",v->regs[14]);
        printf("instruction is %llx\n",*instr);
	printf("instr->Rm = %u\n",instr->Rm);
	printf("v->regs[instr->Rm] = %u\n",v->regs[instr->Rm]);
	v->regs[15] = v->regs[instr->Rm];
return;
}
*/ 

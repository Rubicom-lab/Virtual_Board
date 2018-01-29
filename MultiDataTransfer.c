#include "vm.h"
#include "ARM_engine.h"
#include <stdio.h>
#include <stdlib.h>
#define depth 4

char *stackJmpr;
unsigned long long int To64stack(VCPU* v){
	long long int initial_sp = INITIAL_SP;
	printf("in To64stack, 0x%llx\n",(unsigned long long int)((-initial_sp + (v->regs[13]))+ (char *)(v->stackBase) + REQUESTED_STACKSZ));
	printf("v->regs[13] = %llx\n",v->regs[13]);
	printf("v->stackBase = %llx\n",v->stackBase);
	return (unsigned long long int)((-initial_sp + (v->regs[13]))+ (char *)(v->stackBase) + REQUESTED_STACKSZ);//INITIAL_SP vm.h
}

void To32stack(VCPU* v){


}

void execute_LDSTR(VCPU* v){//load store multiple instruction
 unsigned int which=0;
 which = (*(v->instr_addr)&(PUSH_POP_WHICH));
 switch(which){
 case is_POP:
 execute_LDSTR_POP(v);
 break;
 case is_PUSH: 
 execute_LDSTR_PUSH(v);
 break;
 default: 
 (v->regs[15])-=4;
 break;
 }
}

void execute_LDSTR_PUSH(VCPU* v){
 printf("push 명령어 \n");
 int cnt = 0;
 unsigned int tmp_reglist=0;
 tmp_reglist = (*v->instr_addr)&(REG_LIST_MASK);
 //v->stackptr = v->regs[13];
 while(tmp_reglist!=0){//계속 shift하다보면 0됨

  if((tmp_reglist&1)==1){
	(v->regs[13])= (v->regs[13])-depth;//full descending 방식
 	//v->stackptr = v->regs[13];
 	v->stackptr = To64stack(v);
  	*(v->stackptr) = v->regs[cnt];
  }

  tmp_reglist=tmp_reglist>>1;
  cnt++;
 }
 //debugging
 stackJmpr = (char*)v->stackptr;

 
 v->regs[15]-=4;
 return;
}

void execute_LDSTR_POP(VCPU* v){
 int is_pc_write = 0;
 int cnt=15;

 printf("pop 명령어\n");
 unsigned int tmp_reglist=0;
 tmp_reglist = (*v->instr_addr)&(REG_LIST_MASK);
 tmp_reglist = tmp_reglist<<16;

 while(tmp_reglist!=0){
	if(cnt==15) is_pc_write = 1;
	if((tmp_reglist & 0x80000000)==0x80000000){//1000 0000 0000 0000 000..
		//v->stackptr = v->regs[13];
		v->stackptr = To64stack(v);
		v->regs[cnt] = *(v->stackptr);
		(v->regs[13]) = (v->regs[13]) + depth;
	}
	tmp_reglist = tmp_reglist<<1;
	cnt--;
}
//debugging
stackJmpr = (char*)v->stackptr;

 
 if(is_pc_write!=1) {v->regs[15]-=4; printf("no pc write\n"); }
 return;
}

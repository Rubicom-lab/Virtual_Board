#include "vm.h"
#include "ARM_engine.h"
#include "parse.h"
#include <stdio.h>
#include <stdlib.h>
/*
unsigned int to64stack(VCPU* v){
	return (v->regs[13])-INITIAL_SP + v->stackBase + REQUESTED_STACKSZ -1;
}
*/
extern unsigned long long int to64stack(unsigned long long executeReg,VCPU* v){
	long long int executeReg_tmp = (long long int)executeReg;
	long long int initial_sp = INITIAL_SP;
	
	return (unsigned long long int)((-initial_sp + executeReg_tmp) + (char *)(v->stackBase) + REQUESTED_STACKSZ -4);//stack : 4byte단위
}

/* for facto
#define VADDR_DATA_SEG 0x000097458         //exe_h.vm_ph[data_idx].p_vaddr //0x000097458
#define FILESZ_DATA_SEG 0x00000f7c        //exe_h.vm_ph[data_idx].p_filesz //0x00000f7c
#define MEMSZ_DATA_SEG 0x000020bc         //exe_h.vm_ph[data_idx].p_memsz //0x000020bc
#define FILE_OFFSET_OF_DATA_SEG 0x077458 //exe_h.vm_ph[data_idx].p_offset //0x077458
*/
//for bubbleSort
/*#define VADDR_DATA_SEG 0x000094f6c         //exe_h.vm_ph[data_idx].p_vaddr 
#define FILESZ_DATA_SEG 0x00000f7c        //exe_h.vm_ph[data_idx].p_filesz 
#define MEMSZ_DATA_SEG 0x00001b24         //exe_h.vm_ph[data_idx].p_memsz 
#define FILE_OFFSET_OF_DATA_SEG 0x074f6c //exe_h.vm_ph[data_idx].p_offset 
*/

void str_I(VCPU* v){//stackptr은 조작하되, reg[13]은 수정해서는 안된다.
printf("str명령어\n");
SingleDataTransfer* instr = (SingleDataTransfer*)((v->instr_addr));
printf("instruction is %llx\n",*instr);
printf("v->regs[instr->Rd] = 0x%llx\n",v->regs[instr->Rd]);
unsigned long long executeReg = v->regs[instr->Rn];//executeReg는64bit 주소를 담는 변수일 뿐. jmpr과 함께 쓰인다

	if(( (VADDR_DATA_SEG <= v->regs[instr->Rn]) && (VADDR_DATA_SEG+MEMSZ_DATA_SEG > v->regs[instr->Rn]) ))
	{
		str_Global_Data(v);
		return;
	}

/*
printf("VADDR_DATA_SEG = %llx\t%llx\n",VADDR_DATA_SEG,exe_h.vm_ph[data_idx].p_vaddr);
printf("FILESZ_DATA_SEG= %x\t%x\n",FILESZ_DATA_SEG, exe_h.vm_ph[data_idx].p_filesz);
printf("MEMSZ_DATA_SEG= %x\t%x\n",MEMSZ_DATA_SEG, exe_h.vm_ph[data_idx].p_memsz);
printf("FILE_OFFSET_OF_DATA_SEG= %x\t%x\n",FILE_OFFSET_OF_DATA_SEG, exe_h.vm_ph[data_idx].p_offset);
*/

long long temp = (long long)instr->offset;//temp는 더 커질 수 있어서 long long으로
//temp*=2;
//temp = shift(temp,0); //register인 경우
if(instr->U == 0) temp *=-1;
if(instr->P == 1) (executeReg)+= temp;//항상 일어남 arm특성
//if(oper.SDT_I.B == 1)  //거의 항상0.  1->BYTE 0->WORD

//v->jmpr= executeReg;
if(instr->Rn == 15) v->jmpr = executeReg-ARM_START_OF_CODESEG+(v->base);//rn이 pc인 경우.. v->jmpr은 64bit주소로 접근, str에서는 이런 경우가 거의 없지 않나?
else v->jmpr = to64stack(executeReg,v);//stack

*(v->jmpr) = v->regs[instr->Rd];//
if(instr->W == 1 || instr->P ==0) v->regs[instr->Rn] += temp;//post인 경우와 w==1인 경우인데 !이며, ldr,str에서는 거의 일어나지 않음
(v->regs[15])-=4;
}

void str_R(VCPU* v){
printf("this is str_R(), you have to define it \n");
exit(1);
}

void str_Global_Data(VCPU* v){
SingleDataTransfer* instr = (SingleDataTransfer*)((v->instr_addr));
unsigned long long executeReg = v->regs[instr->Rn];
long long temp = (long long)instr->offset;//temp는 더 커질 수 있어서 long long으로
//temp*=2;
if(instr->U == 0) temp *=-1;
if(instr->P == 1) (executeReg)+= temp;//항상 일어남 arm특성

*(unsigned int*)((v->base)+FILE_OFFSET_OF_DATA_SEG+(executeReg-VADDR_DATA_SEG)) = v->regs[instr->Rd];
if(instr->W == 1 || instr->P ==0) v->regs[instr->Rn] += temp;//post인 경우와 w==1인 경우인데 !이며, ldr,str에서는 거의 일어나지 않음
(v->regs[15])-=4;
return;
}

void ldr_I(VCPU* v){//stackptr은 조작하되, reg[13]은 수정해서는 안된다.
printf("ldr 명령어\n");
SingleDataTransfer* instr = (SingleDataTransfer*)((v->instr_addr));
printf("instruction is %llx\n",*instr);
printf("v->regs[instr->Rd] = 0x%llx\n",v->regs[instr->Rd]);
unsigned long long executeReg = v->regs[instr->Rn];


	if(( (VADDR_DATA_SEG <= v->regs[instr->Rn]) && (VADDR_DATA_SEG+MEMSZ_DATA_SEG > v->regs[instr->Rn]) ))
	{
		ldr_Global_Data(v);
		return;
	}

long long temp = (long long)instr->offset;//temp는 더 커질 수 있어서 long long으로
//temp*=2;
if(instr->U == 0) temp *=-1;
if(instr->P == 1) (executeReg)+= temp;//항상 일어남 arm특성

if(instr->Rn == 15) v->jmpr = executeReg-ARM_START_OF_CODESEG+(v->base);//v->jmpr은 64bit주소로 접근
else v->jmpr = to64stack(executeReg,v);

v->regs[instr->Rd] = *(v->jmpr);  
if(instr->W == 1 || instr->P ==0) v->regs[instr->Rn] += temp;//postindex인 경우와 w==1인 경우인데 !이며, ldr,str에서는 거의 일어나지 않음
(v->regs[15])-=4;
return;
}


void ldr_Global_Data(VCPU* v){//stackptr은 조작하되, reg[13]은 수정해서는 안된다.
SingleDataTransfer* instr = (SingleDataTransfer*)((v->instr_addr));
unsigned long long executeReg = v->regs[instr->Rn];
long long temp = (long long)instr->offset;//temp는 더 커질 수 있어서 long long으로
//temp*=2;
if(instr->U == 0) temp *=-1;
if(instr->P == 1) (executeReg)+= temp;//항상 일어남 arm특성

//v->jmpr = v->regs[instr->Rn];
//v->regs[instr->Rd] = *(v->jmpr);  
v->regs[instr->Rd]=*(unsigned int*)((v->base)+FILE_OFFSET_OF_DATA_SEG+(executeReg-VADDR_DATA_SEG));

if(instr->W == 1 || instr->P ==0) v->regs[instr->Rn] += temp;
//post인 경우와 w==1인 경우인데 !이며, ldr,str에서는 거의 일어나지 않음
(v->regs[15])-=4;
return;
}

void ldr_R(VCPU* v){
printf("this is ldr_R(), you have to define it \n");
exit(1);
}

void SDTransfer_I(VCPU* v){
SingleDataTransfer* instr = (SingleDataTransfer*)((v->instr_addr));
if(instr->L == 0){ 
str_I(v); 
}
else ldr_I(v);
}

void SDTransfer_R(VCPU* v){

}

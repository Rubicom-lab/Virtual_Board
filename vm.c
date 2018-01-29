#include "vm.h"
#include "ARM_engine.h"
#include "parse.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>

#define FLAG_I 1
#define FLAG_J 2
#define COPYINCR (1024*1024*1024) /*1GB*/
struct stat sbuf;//나중에 따로 빼야 함
int flag_local;

void insert_brkPoint(VCPU* v,unsigned long long brkPoint){ //param : v,32bit address
    //unsigned int *addr;
    v->orgData_ptr = brkPoint;
    unsigned int data_with_trap;

    if(mprotect(v->base,sbuf.st_size,PROT_READ|PROT_WRITE)<0) {printf("hey!\n"); quit("mprotect in insert_brkPoint");}
    unsigned long long temp = (unsigned long long)v->base;
    v->orgData_ptr = (char*)(v->orgData_ptr) + temp - ARM_START_OF_CODESEG;//

    v->orgData_temp = *(unsigned int*)(v->orgData_ptr);//값 바꾸기 전, backup. brkPoint_handler()에서 사용
    data_with_trap = (*(unsigned int*)(v->orgData_ptr) & 0xFFFFFF00) | 0xCC;//trap 설치
    *(unsigned int*)(v->orgData_ptr) = data_with_trap;
	

    /*permission 되돌리기*/
    if(mprotect(v->base,sbuf.st_size,PROT_READ)<0) quit("mprotect in insert_brkPoint");
    if(mprotect(v->base+0x074000,4096*3,PROT_READ|PROT_WRITE)<0) quit("mprotect in insert_brkPoint");


    return;
}

void brkPoint_handler(VCPU *v){//fetch()단계에서 불려야 함


    printf("1 : 한 단계 다음으로 진행 \n2 : break point 풀기 \n");
    scanf("%d",&v->flag);//잠깐 기다림
	
    if(v->flag == 2){// break point풀기 == 다음 instrcution에 trap안깔기
	if(mprotect(v->base,sbuf.st_size,PROT_READ|PROT_WRITE)<0) quit("mprotect in insert_brkPoint");

	/*POKETEXT  instruction 복구*/
	*(unsigned int*)(v->orgData_ptr) = v->orgData_temp;

	/*permission 되돌리기*/
	if(mprotect(v->base,sbuf.st_size,PROT_READ)<0) quit("mprotect in insert_brkPoint");
	if(mprotect(v->base+0x074000,4096*3,PROT_READ|PROT_WRITE)<0) quit("mprotect int insert_brkPoint");

	/*roll back*/
	//v->regs[15]-=4;
	//fetch(v);
	return;
    }
    else if(v->flag == 1){
	if(mprotect(v->base,sbuf.st_size,PROT_READ|PROT_WRITE)<0) quit("mprotect in insert_brkPoint");

	/*POKETEXT  instruction 복구*/
	*(unsigned int*)(v->orgData_ptr) = v->orgData_temp;

	/*permission 되돌리기*/
	if(mprotect(v->base,sbuf.st_size,PROT_READ)<0) quit("mprotect in insert_brkPoint");
	if(mprotect(v->base+0x074000,4096*3,PROT_READ|PROT_WRITE)<0) quit("mprotect in insert_brkPoint");

	int which = (*(v->instr_addr) & 0x0e000000);
	if(which != is_B)
	    { 
		insert_brkPoint(v,(unsigned long long)((v->regs[15])+4)); //그 다음 명령어가 B이나 BX류일때는 해서는 안된다
	    }
	else;
	/*roll back*/
	//v->regs[15]-=4;
	//fetch(v);
	return;
		
    }
}

void changeValue(VCPU* v){
	
    *(int*)( (v->base)+FILE_OFFSET_OF_DATA_SEG + 0x95088-VADDR_DATA_SEG + ((selectNum-1)*4) ) = num;
}

int quit(char *a){
    printf("err %s\n",a);
    printf("errno: %d, %s\n",errno,strerror(errno));
    exit(1);
}

VCPU*	vcpu_alloc(void)
{
    VCPU*	retval = (VCPU*)malloc(sizeof(VCPU));
    if(NULL != retval)
	memset(retval, 0, sizeof(VCPU));
    return retval;
}

void	vcpu_delete(VCPU** v)
{
    if(NULL != v && NULL != *v)
	{
	    if(NULL != (*v)->stackBase)
		free((*v)->stackBase);
	    if(NULL != (*v)->base)
		free((*v)->base);
	    free(*v);
	    *v = NULL;
	}
}

void	vcpu_set_pc(VCPU* v)//call only once
{
    setReg(v);//위치를 나중에 바꿔야 함. stack을 mmap으로 하려면
    if(NULL == v)
	return;
    //v->regs[15] = v->base+0xd48;//later change to exe_h.vm_eh->e_entry;	
    v->regs[15] = ARM_START_OF_CODESEG+0x5dc;//later change to exe_h.vm_eh->e_entry;	
}

int		vcpu_load(VCPU* v, char* vm_file)
{
    int fdin;//dusol
    unsigned int *src;//32bit instruction
    size_t copysz;
    off_t fsz = 0;
    //struct stat sbuf;

    VM_HEADER		vmh;
	
    if(NULL == v || NULL == vm_file) return -1;
    if((fdin = open(vm_file,O_RDONLY))<0)  quit("open ");
    if(fstat(fdin,&sbuf)<0) quit("fstat "); 
	 
    if((v->base = mmap(0,sbuf.st_size,PROT_READ,MAP_PRIVATE,fdin,0)) == MAP_FAILED) quit("mmap ");//0x3e8

    get_Headers(v->base);
    set_Headers(v->base);

    if(mprotect(v->base+0x074000,4096*3,PROT_READ|PROT_WRITE)<0) quit("mprotect ");//16^3 = 4096, 74000:boundary,memsize(0x020bc) optimized for pagesize,prot
    int i=0;
    src = (unsigned int*)(v->base);
    //for(i=0; i<10; i++) printf("%x\n", *((unsigned int *)exe_h.vm_eh->e_entry+i));

    printf("\n");
	
    if(research == 'y') changeValue(v);//숫자 change

    /*
      printf("%llx\n",((v->base)+FILE_OFFSET_OF_DATA_SEG+(0x00097568-0x00097458)));
      printf("%llx\n",(unsigned int*)((v->base)+FILE_OFFSET_OF_DATA_SEG+(0x00097568-0x00097458)));

      printf("%x\n",*(unsigned int*)((v->base)+FILE_OFFSET_OF_DATA_SEG+(0x00097568-0x00097458)));//a
      *(unsigned int*)((v->base)+(FILE_OFFSET_OF_DATA_SEG+(0x00097568-0x00097458))) =0xb;//change data
      printf("%x\n",*(unsigned int*)((v->base)+FILE_OFFSET_OF_DATA_SEG+(0x00097568-0x00097458)));//b
      *(unsigned int*)((v->base)+(FILE_OFFSET_OF_DATA_SEG+(0x00097568-0x00097458))) =0xa;//restore data
      */

    void *p_mmap;	
    p_mmap = mmap(NULL,REQUESTED_STACKSZ,PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);
    stackBase_mmap =  (unsigned int*)p_mmap;//vm.h
	
    //v->stack_Base = stackBase_mmap;
    //printf("p_mmap = 0x%llx \n", p_mmap);
    printf("stackBase_mmap = 0x%llx \n", stackBase_mmap);

    //*(stackBase_mmap+1) = 1;
    //printf("*(stackBase_mmap+1) = 0x%llx\n",*(stackBase_mmap+1));
    v->stackBase = stackBase_mmap;
    /*
      if(0 < REQUESTED_STACKSZ)
      {
      v->stackBase = (unsigned int*)malloc(sizeof(unsigned long long) * REQUESTED_STACKSZ);
      //printf("stack address : %llx\n",v->stackBase);
      //v->regs[13]= v->stackBase + REQUESTED_STACKSZ- 1;//stackptr(regs[13])가 스택의 시작이라고 생각해야함. 아래로 자라남
      //printf("stack created.: %llx\n",v->regs[13]);
      }
      else
      {
      printf("stack creation error\n");
      close(fdin);
      return -1;
      }
    */
    close(fdin);
	
    return 0;
}


void fetch(VCPU* v){
    if(v->regs[15] == 0x88888888)
	{
	    printf("프로그램 실행 완료\n");
	    exit(0);
	}	

    (v->instr_addr) = v->regs[15]-ARM_START_OF_CODESEG+(v->base);//
    if((*(v->instr_addr) & 0x000000CC) == 0xCC) brkPoint_handler(v);//trap 감지
    v->regs[15]+=4;
}

void decode(VCPU* v){

    unsigned int tmp_instr= *(v->instr_addr);
    v->which_mold = ( tmp_instr & FI_MASK ); 
    v->regs[15]+=4;
}

void setReg(VCPU* v){//초기값 설정
    //v->regs[0] = 1;
    //v->regs[1] = 0;
    //v->regs[2] = 0;
    //v->regs[3] = 0;
    //v->regs[4] = 0;
    v->regs[11]= 0;//fp
    v->regs[12]= 0;//
    //v->regs[11]=0x77777777;
    //v->regs[14]=0x10f64;//lr

    v->regs[13]=0x7effef40;//sp
    //v->regs[13]= v->stackBase + REQUESTED_STACKSZ- 1;//stackptr(regs[13])가 스택의 시작이라고 생각해야함. 아래로 자라남
    v->regs[14]=0x88888888;//lr
    v->cpsr.Z = 1;
    v->cpsr.C = 1;
}


int		vcpu_run(VCPU* v)
{
    //unsigned int *which_mold;//decode(), 
    int counter = 0;
    int __volatile__ 	do_run = 1;
    //unsigned int tmp_instr_addr = *(v->instr_addr);
    if(NULL == v || NULL == v->base)
	return 0;

    //setReg(v);

    while(1 == do_run){//fetch,decode,execute
	fetch(v);
	decode(v);
	switch(v->which_mold){
	case is_LDSTR_MUL:
	    execute_LDSTR(v); 
	    printAll(v);
	    printf("-----------------------------------------------------------------------------------------------------\n");
	    break;
	case is_LDSTR_SINGLE_I:
	    SDTransfer_I(v); 
	    printAll(v);
	    printf("-----------------------------------------------------------------------------------------------------\n");
	    break;
	case is_LDSTR_SINGLE_R:
	    printf("you have to make function for SDTranfer_R\n");
	    break;
	case is_DP_I:
	    DataProcess(v); //아래든 이거든  한번에 처리
	    printAll(v);
	    printf("-----------------------------------------------------------------------------------------------------\n");
	    break;
	case is_DP_R:
	    DataProcess(v); 
	    printAll(v);
	    printf("-----------------------------------------------------------------------------------------------------\n");
	    break;
	case is_B:
	    B(v);
	    printAll(v);
	    printf("-----------------------------------------------------------------------------------------------------\n");
	    break;
	default:
	    v->regs[15]-=4;//test용:
	    printf("i can't decode this instruction\n"); break; 
	}
	
	
    }
    return counter;
}
void printAll(VCPU *v){
    if(v->regs[15]-4 == 0x104e4) flag_local++;
    if(v->regs[15]-4 == 0x104fc) flag_local++;

    printf("\n\n");
    //unsigned int* stackPtr;
    unsigned int* stackPtr_arr[20];
    //unsigned long long stackPtr_32 =(unsigned long long)(v->regs[13]);
    unsigned long long int stackPtr_64 =(unsigned long long int)(v->stackBase);
    printf("stackPtr_64 = 0x%llx\n",stackPtr_64);
    printf("현재 stack pointer = %x\n",v->regs[13]);
    printf("현재 frame pointer = %x\n",v->regs[11]);
    printf("\n");
    //stackPtr = to64stack(stackPtr_32,v);
    //for(int i=1; i<16; i++){
    //	stackPtr_arr[i]	= to64stack();
    //}
    for(int i=1; i<17; i++){
	stackPtr_arr[i]	= stackPtr_64 + REQUESTED_STACKSZ - i*4;
    }

    for(int i=1; i<17; i++){
	printf("stackPtr_arr[%d]	= %llx\n",i,stackPtr_arr[i]);
    }

    for(int i=1; i<17; i++){
	printf("stackPtr_arr[%d]= %llx\n",i,*stackPtr_arr[i]);
    }
    //
    //
    for(int i=0; i<4; i++) v->arr[i] = *(int*)((v->base)+FILE_OFFSET_OF_DATA_SEG + 0x95088-VADDR_DATA_SEG + (4*i));

    if(v->regs[0] == v->regs_before[0])
	printf("    reg[0]  : 0x%08x ",v->regs[0]);
    else printf("    \033[1;32mreg[0]  : 0x%08x\033[0m",v->regs[0]); 
    if(v->arr[0] == v->arr_before[0])
	printf("	0x95088(arr[0]):%d \n",*(int*)((v->base)+FILE_OFFSET_OF_DATA_SEG + 0x95088-VADDR_DATA_SEG));
    else printf("	\033[1;31m95088(arr[0]):%d \033[0m\n",*(int*)((v->base)+FILE_OFFSET_OF_DATA_SEG + 0x95088-VADDR_DATA_SEG));

    if(v->regs[1] == v->regs_before[1])
	printf("    reg[1]  : 0x%08x ",v->regs[1]);
    else printf("    \033[1;32mreg[1]  : 0x%08x\033[0m",v->regs[1]); 
    if(v->arr[1] == v->arr_before[1])
	printf("	0x9508c(arr[1]):%d \n",*(int*)((v->base)+FILE_OFFSET_OF_DATA_SEG + 0x95088-VADDR_DATA_SEG+4));
    else printf("	\033[1;31m(0x9508c)arr[1]:%d \033[0m\n",*(int*)((v->base)+FILE_OFFSET_OF_DATA_SEG + 0x95088-VADDR_DATA_SEG+4));

    if(v->regs[2] == v->regs_before[2])
	printf("    reg[2]  : 0x%08x ",v->regs[2]);
    else printf("    \033[1;32mreg[2]  : 0x%08x\033[0m",v->regs[2]); 
    if(v->arr[2] == v->arr_before[2])
	printf("	0x95090(arr[2]):%d \n",*(int*)((v->base)+FILE_OFFSET_OF_DATA_SEG + 0x95088-VADDR_DATA_SEG+8));
    else printf("	\033[1;31m0x95090(arr[2]):%d \033[0m\n",*(int*)((v->base)+FILE_OFFSET_OF_DATA_SEG + 0x95088-VADDR_DATA_SEG+8));

    if(v->regs[3] == v->regs_before[3])
	printf("    reg[3]  : 0x%08x ",v->regs[3]);
    else printf("    \033[1;32mreg[3]  : 0x%08x\033[0m",v->regs[3]); 
    if(v->arr[3] == v->arr_before[3])
	printf("	0x95094(arr[3]):%d \n",*(int*)((v->base)+FILE_OFFSET_OF_DATA_SEG + 0x95088-VADDR_DATA_SEG+12));
    else printf("	\033[1;31m0x95094(arr[3]):%d \033[0m\n",*(int*)((v->base)+FILE_OFFSET_OF_DATA_SEG + 0x95088-VADDR_DATA_SEG+12));

    if(v->regs[4] == v->regs_before[4])
	printf("    reg[4]  : 0x%08x \n",v->regs[4]);
    else printf("    \033[1;32mreg[4]  : 0x%08x\n\033[0m",v->regs[4]);
    if(v->regs[5] == v->regs_before[5])
	printf("    reg[5]  : 0x%08x ",v->regs[5]);
    else printf("    \033[1;32mreg[5]  : 0x%08x\033[0m",v->regs[5]); printf("			Stack segment		\n");
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    if(v->regs[6] == v->regs_before[6]){
	printf("    reg[6]  : 0x%08x \n",v->regs[6]);
    }
    else {
	printf("    \033[1;32mreg[6]  : 0x%08x\033[0m \n",v->regs[6]);
    } 
    /*
      if(v->regs[13]== 0x7effef10 && (flag_local > 1)){
      printf("	 0x%x : 	0x%08x 0x%08x 0x%08x 0x%08x \033[1;36m0x%08x\033[0m \n",v->regs[13],*stackPtr_arr[0],*stackPtr_arr[1],*stackPtr_arr[2],*stackPtr_arr[3],*stackPtr_arr[4]);
      }
      else
      printf("	 0x%x : 	0x%08x 0x%08x 0x%08x 0x%08x 0x%08x \n",v->regs[13],*stackPtr_arr[0],*stackPtr_arr[1],*stackPtr_arr[2],*stackPtr_arr[3],*stackPtr_arr[4]);
    */

    if(v->regs[7] == v->regs_before[7])
	printf("    reg[7]  : 0x%08x \n",v->regs[7]);
    else printf("    \033[1;32mreg[7]  : 0x%08x\033[0m \n",v->regs[7]); 

    /*
      if(v->regs[13]== 0x7effef10 && (flag_local >=1))//bubbleSort loop stack frame의 sp
      printf("	 0x%x : 	\033[1;33m0x%08x\033[0m 0x%08x\033[0m 0x%08x 0x%08x 0x%08x \n",v->regs[13]+20,*stackPtr_arr[5],*stackPtr_arr[6],*stackPtr_arr[7],*stackPtr_arr[8],*stackPtr_arr[9]);
      else
      printf("	 0x%x : 	0x%08x 0x%08x 0x%08x 0x%08x 0x%08x \n",v->regs[13]+20,*stackPtr_arr[5],*stackPtr_arr[6],*stackPtr_arr[7],*stackPtr_arr[8],*stackPtr_arr[9]);
    */
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    if(v->regs[8] == v->regs_before[8])
	printf("    reg[8]  : 0x%08x \n",v->regs[8]);
    else printf("    \033[1;32mreg[8]  : 0x%08x\033[0m \n",v->regs[8]); 

    //printf("	 0x%x : 	0x%08x 0x%08x 0x%08x 0x%08x 0x%08x 0x%08x \n",v->regs[13]+40,*stackPtr_arr[10],*stackPtr_arr[11],*stackPtr_arr[12],*stackPtr_arr[13],*stackPtr_arr[14],*stackPtr_arr[15]);

    if(v->regs[9] == v->regs_before[9])
	printf("    reg[9]  : 0x%08x \n",v->regs[9]);
    else printf("    \033[1;32mreg[9]  : 0x%08x\033[0m \n",v->regs[9]); 

    //printf("	 0x%x : 	0x%08x 0x%08x 0x%08x 0x%08x 0x%08x \n",v->regs[13]+60,*stackPtr_arr[15],*stackPtr_arr[16],*stackPtr_arr[17],*stackPtr_arr[18],*stackPtr_arr[19]);


    if(v->regs[10] == v->regs_before[10])
	printf("    reg[10] : 0x%08x\n",v->regs[10]);
    else printf("    \033[1;32mreg[10] : 0x%08x\n\033[0m",v->regs[10]); 
    if(v->regs[11] == v->regs_before[11])
	printf("(fp)reg[11] : 0x%08x\n",v->regs[11]);
    else printf("\033[1;32m(fp)reg[11] : 0x%08x\n\033[0m",v->regs[11]); 
    if(v->regs[12] == v->regs_before[12])
	printf("(ip)reg[12] : 0x%08x\n",v->regs[12]);
    else printf("\033[1;32m(ip)reg[12] : 0x%08x\n\033[0m",v->regs[12]); 
    if(v->regs[13] == v->regs_before[13])
	printf("(sp)reg[13] : 0x%08x\n",v->regs[13]);
    else printf("\033[1;32m(sp)reg[13] : 0x%08x\033[0m\n",v->regs[13]); 
    if(v->regs[14] == v->regs_before[14])
	printf("(lr)reg[14] : 0x%08x\n",v->regs[14]);
    else printf("\033[1;32m(lr)reg[14] : 0x%08x\033[0m\n",v->regs[14]); 
    if(v->regs[15] == v->regs_before[15])
	printf("(pc)reg[15] : 0x%08x\n",v->regs[15]-4);
    else printf("\033[1;32m(pc)reg[15] : 0x%08x\033[0m\n",v->regs[15]-4); 
	

    if(v->regs[13] == 0x7effef10 && flag_local == 1 ) //i만 표시
	printf("						\033[1;33m지역변수 i\033[0m\n");
    if(v->regs[13] == 0x7effef10 && flag_local > 1) //i,j 둘다표시
	printf("						\033[1;33m지역변수 i\033[0m	\033[1;36m지역변수 j\033[0m\n");
    //	
    //	if(v->regs[13] == 0x7effef10 && (v->regs[15]-4 > 0x104e0) && (v->regs[15]-4<0x104fc)) //i만 표시
    //		printf("						\033[1;33m지역변수 i\033[0m\n");
    //	if(v->regs[13] == 0x7effef10 && (v->regs[15]-4 >= 0x104fc)) //i,j 둘다표시
    //		printf("						\033[1;33m지역변수 i\033[0m	\033[1;36m지역변수 j\033[0m\n");


    //	int frameSize = (v->regs[11] - v->regs[13])/4;
    //	if(v->regs[11]!= 0 && v->regs[11]!=88888888){
    //for(int z=0; z<frameSize; z++) printf("0x%08x \n",*(stackPtr+z));
    //	for(int z=frameSize-1; z>=0; z--) printf("0x%08x \n",*(stackPtr+z));

    //	}

    //for(int z=0; z<count; z++);
    printf("\n");
    for(int i=0; i<16; i++) v->regs_before[i] = v->regs[i];
    for(int j=0; j<4; j++) v->arr_before[j] = v->arr[j];
    //for(int i=0; i<3; i++) arr[0]:%d \n",*(int*)((v->base)+FILE_OFFSET_OF_DATA_SEG + 0x95088-VADDR_DATA_SEG));
    printf("\n");
}
/*
  void printAll(VCPU *v){
  if(v->regs[15]-4 == 0x104e4) flag_local++;
  if(v->regs[15]-4 == 0x104fc) flag_local++;

  printf("\n\n");
  //unsigned int* stackPtr;
  unsigned int* stackPtr_arr[20];
  //unsigned long long stackPtr_32 =(unsigned long long)(v->regs[13]);
  unsigned long long int stackPtr_64 =(unsigned long long int)(v->stackBase);
  printf("현재 stack pointer = %x\n",v->regs[13]);
  printf("현재 frame pointer = %x\n",v->regs[11]);
  printf("\n");
  //stackPtr = to64stack(stackPtr_32,v);
  //for(int i=0; i<16; i++){
  //	stackPtr_arr[i]	= to64stack();
  //}
  for(int i=1; i<17; i++){
  stackPtr_arr[i]	= stackPtr_64 + REQUESTED_STACKSZ - i*4;
  }
  for(int i=0; i<4; i++) v->arr[i] = *(int*)((v->base)+FILE_OFFSET_OF_DATA_SEG + 0x95088-VADDR_DATA_SEG + (4*i));

  if(v->regs[0] == v->regs_before[0])
  printf("    reg[0]  : 0x%08x ",v->regs[0]);
  else printf("    \033[1;32mreg[0]  : 0x%08x\033[0m",v->regs[0]); 
  if(v->arr[0] == v->arr_before[0])
  printf("	0x95088(arr[0]):%d \n",*(int*)((v->base)+FILE_OFFSET_OF_DATA_SEG + 0x95088-VADDR_DATA_SEG));
  else printf("	\033[1;31m95088(arr[0]):%d \033[0m\n",*(int*)((v->base)+FILE_OFFSET_OF_DATA_SEG + 0x95088-VADDR_DATA_SEG));

  if(v->regs[1] == v->regs_before[1])
  printf("    reg[1]  : 0x%08x ",v->regs[1]);
  else printf("    \033[1;32mreg[1]  : 0x%08x\033[0m",v->regs[1]); 
  if(v->arr[1] == v->arr_before[1])
  printf("	0x9508c(arr[1]):%d \n",*(int*)((v->base)+FILE_OFFSET_OF_DATA_SEG + 0x95088-VADDR_DATA_SEG+4));
  else printf("	\033[1;31m(0x9508c)arr[1]:%d \033[0m\n",*(int*)((v->base)+FILE_OFFSET_OF_DATA_SEG + 0x95088-VADDR_DATA_SEG+4));

  if(v->regs[2] == v->regs_before[2])
  printf("    reg[2]  : 0x%08x ",v->regs[2]);
  else printf("    \033[1;32mreg[2]  : 0x%08x\033[0m",v->regs[2]); 
  if(v->arr[2] == v->arr_before[2])
  printf("	0x95090(arr[2]):%d \n",*(int*)((v->base)+FILE_OFFSET_OF_DATA_SEG + 0x95088-VADDR_DATA_SEG+8));
  else printf("	\033[1;31m0x95090(arr[2]):%d \033[0m\n",*(int*)((v->base)+FILE_OFFSET_OF_DATA_SEG + 0x95088-VADDR_DATA_SEG+8));

  if(v->regs[3] == v->regs_before[3])
  printf("    reg[3]  : 0x%08x ",v->regs[3]);
  else printf("    \033[1;32mreg[3]  : 0x%08x\033[0m",v->regs[3]); 
  if(v->arr[3] == v->arr_before[3])
  printf("	0x95094(arr[3]):%d \n",*(int*)((v->base)+FILE_OFFSET_OF_DATA_SEG + 0x95088-VADDR_DATA_SEG+12));
  else printf("	\033[1;31m0x95094(arr[3]):%d \033[0m\n",*(int*)((v->base)+FILE_OFFSET_OF_DATA_SEG + 0x95088-VADDR_DATA_SEG+12));

  if(v->regs[4] == v->regs_before[4])
  printf("    reg[4]  : 0x%08x \n",v->regs[4]);
  else printf("    \033[1;32mreg[4]  : 0x%08x\n\033[0m",v->regs[4]);
  if(v->regs[5] == v->regs_before[5])
  printf("    reg[5]  : 0x%08x ",v->regs[5]);
  else printf("    \033[1;32mreg[5]  : 0x%08x\033[0m",v->regs[5]); printf("			Stack segment		\n");
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  if(v->regs[6] == v->regs_before[6]){
  printf("    reg[6]  : 0x%08x \n",v->regs[6]);
  }
  else {
  printf("    \033[1;32mreg[6]  : 0x%08x\033[0m \n",v->regs[6]);
  } 

  if(v->regs[13]== 0x7effef10 && (flag_local > 1)){
  printf("	 0x%x : 	0x%08x 0x%08x 0x%08x 0x%08x \033[1;36m0x%08x\033[0m \n",v->regs[13],*stackPtr_arr[0],*stackPtr_arr[1],*stackPtr_arr[2],*stackPtr_arr[3],*stackPtr_arr[4]);
  }
  else
  printf("	 0x%x : 	0x%08x 0x%08x 0x%08x 0x%08x 0x%08x \n",v->regs[13],*stackPtr_arr[0],*stackPtr_arr[1],*stackPtr_arr[2],*stackPtr_arr[3],*stackPtr_arr[4]);


  if(v->regs[7] == v->regs_before[7])
  printf("    reg[7]  : 0x%08x \n",v->regs[7]);
  else printf("    \033[1;32mreg[7]  : 0x%08x\033[0m \n",v->regs[7]); 

  if(v->regs[13]== 0x7effef10 && (flag_local >=1))//bubbleSort loop stack frame의 sp
  printf("	 0x%x : 	\033[1;33m0x%08x\033[0m 0x%08x\033[0m 0x%08x 0x%08x 0x%08x \n",v->regs[13]+20,*stackPtr_arr[5],*stackPtr_arr[6],*stackPtr_arr[7],*stackPtr_arr[8],*stackPtr_arr[9]);
  else
  printf("	 0x%x : 	0x%08x 0x%08x 0x%08x 0x%08x 0x%08x \n",v->regs[13]+20,*stackPtr_arr[5],*stackPtr_arr[6],*stackPtr_arr[7],*stackPtr_arr[8],*stackPtr_arr[9]);

  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  if(v->regs[8] == v->regs_before[8])
  printf("    reg[8]  : 0x%08x \n",v->regs[8]);
  else printf("    \033[1;32mreg[8]  : 0x%08x\033[0m \n",v->regs[8]); 

  printf("	 0x%x : 	0x%08x 0x%08x 0x%08x 0x%08x 0x%08x 0x%08x \n",v->regs[13]+40,*stackPtr_arr[10],*stackPtr_arr[11],*stackPtr_arr[12],*stackPtr_arr[13],*stackPtr_arr[14],*stackPtr_arr[15]);

  if(v->regs[9] == v->regs_before[9])
  printf("    reg[9]  : 0x%08x \n",v->regs[9]);
  else printf("    \033[1;32mreg[9]  : 0x%08x\033[0m \n",v->regs[9]); 

  //printf("	 0x%x : 	0x%08x 0x%08x 0x%08x 0x%08x 0x%08x \n",v->regs[13]+60,*stackPtr_arr[15],*stackPtr_arr[16],*stackPtr_arr[17],*stackPtr_arr[18],*stackPtr_arr[19]);


  if(v->regs[10] == v->regs_before[10])
  printf("    reg[10] : 0x%08x\n",v->regs[10]);
  else printf("    \033[1;32mreg[10] : 0x%08x\n\033[0m",v->regs[10]); 
  if(v->regs[11] == v->regs_before[11])
  printf("(fp)reg[11] : 0x%08x\n",v->regs[11]);
  else printf("\033[1;32m(fp)reg[11] : 0x%08x\n\033[0m",v->regs[11]); 
  if(v->regs[12] == v->regs_before[12])
  printf("(ip)reg[12] : 0x%08x\n",v->regs[12]);
  else printf("\033[1;32m(ip)reg[12] : 0x%08x\n\033[0m",v->regs[12]); 
  if(v->regs[13] == v->regs_before[13])
  printf("(sp)reg[13] : 0x%08x\n",v->regs[13]);
  else printf("\033[1;32m(sp)reg[13] : 0x%08x\033[0m\n",v->regs[13]); 
  if(v->regs[14] == v->regs_before[14])
  printf("(lr)reg[14] : 0x%08x\n",v->regs[14]);
  else printf("\033[1;32m(lr)reg[14] : 0x%08x\033[0m\n",v->regs[14]); 
  if(v->regs[15] == v->regs_before[15])
  printf("(pc)reg[15] : 0x%08x\n",v->regs[15]-4);
  else printf("\033[1;32m(pc)reg[15] : 0x%08x\033[0m\n",v->regs[15]-4); 
	

  if(v->regs[13] == 0x7effef10 && flag_local == 1 ) //i만 표시
  printf("						\033[1;33m지역변수 i\033[0m\n");
  if(v->regs[13] == 0x7effef10 && flag_local > 1) //i,j 둘다표시
  printf("						\033[1;33m지역변수 i\033[0m	\033[1;36m지역변수 j\033[0m\n");
  //	
  //	if(v->regs[13] == 0x7effef10 && (v->regs[15]-4 > 0x104e0) && (v->regs[15]-4<0x104fc)) //i만 표시
  //		printf("						\033[1;33m지역변수 i\033[0m\n");
  //	if(v->regs[13] == 0x7effef10 && (v->regs[15]-4 >= 0x104fc)) //i,j 둘다표시
  //		printf("						\033[1;33m지역변수 i\033[0m	\033[1;36m지역변수 j\033[0m\n");


  //	int frameSize = (v->regs[11] - v->regs[13])/4;
  //	if(v->regs[11]!= 0 && v->regs[11]!=88888888){
  //for(int z=0; z<frameSize; z++) printf("0x%08x \n",*(stackPtr+z));
  //	for(int z=frameSize-1; z>=0; z--) printf("0x%08x \n",*(stackPtr+z));

  //	}

  //for(int z=0; z<count; z++);
  printf("\n");
  for(int i=0; i<16; i++) v->regs_before[i] = v->regs[i];
  for(int j=0; j<4; j++) v->arr_before[j] = v->arr[j];
  //for(int i=0; i<3; i++) arr[0]:%d \n",*(int*)((v->base)+FILE_OFFSET_OF_DATA_SEG + 0x95088-VADDR_DATA_SEG));
  printf("\n");
  }
*/
/*
  unsigned long long to64stack(unsigned long long executeReg,VCPU* v){
  return INITIAL_SP-executeReg + v->stackBase + REQUESTED_STACKSZ -1;
  }*/

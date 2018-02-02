#include "VirtualMachine.h"
#include "Parse.h"
#include "SingleDataTransfer.h"
#include "DataProcessing.h"
#include "MultiDataTransfer.h"
#include "Branch.h"
#include <errno.h>

struct stat sbuf; //나중에 따로 빼야 함
int    flag_local;

// param : v,32bit address
void insertBrkPoint(struct VCPU* v, unsigned long long brk_point) {

    v->org_data_ptr = brk_point;
    unsigned int data_with_trap;

    if(mprotect(v->base, sbuf.st_size, PROT_READ|PROT_WRITE) < 0) {
        printf("hey!\n");
        quit("mprotect in insertBrkPoint");
    }

    unsigned long long temp = (unsigned long long)v->base;
    v->org_data_ptr = (char*)(v->org_data_ptr) + temp - ARM_START_OF_CODESEG;

    // 값 바꾸기 전, backup. brkPoint_handler()에서 사용
    v->org_data_temp = *(unsigned int*)(v->org_data_ptr);

    // trap 설치
    data_with_trap = (*(unsigned int*)(v->org_data_ptr) & 0xFFFFFF00) | 0xCC;
    *(unsigned int*)(v->org_data_ptr) = data_with_trap;


    // permission 되돌리기
    if(mprotect(v->base, sbuf.st_size, PROT_READ) < 0)
        quit("mprotect in insert_brkPoint");
    if(mprotect(v->base+0x074000, 4096*3, PROT_READ | PROT_WRITE) < 0)
        quit("mprotect in insert_brkPoint");

    return;
}

// fetch()단계에서 불려야 함
void brkPointHandler(struct VCPU* v){

    printf("1 : 한 단계 다음으로 진행 \n2 : break point 풀기 \n");

    // 잠깐 기다림
    scanf("%d",&v->flag);

    // break point풀기 == 다음 instrcution에 trap안깔기
    if(v->flag == 2){
        if(mprotect(v->base, sbuf.st_size, PROT_READ | PROT_WRITE) < 0)
            quit("mprotect in insert_brkPoint");

        // POKETEXT  instruction 복구
        *(unsigned int*)(v->org_data_ptr) = v->org_data_temp;

        // permission 되돌리기
        if(mprotect(v->base, sbuf.st_size, PROT_READ) < 0)
            quit("mprotect in insert_brkPoint");
        if(mprotect(v->base+0x074000, 4096*3, PROT_READ | PROT_WRITE) < 0)
            quit("mprotect int insert_brkPoint");

        // roll back
        //v->regs[15]-=4;
        //fetch(v);
        return;
    } else if(v->flag == 1) {
        if(mprotect(v->base, sbuf.st_size, PROT_READ | PROT_WRITE) < 0)
            quit("mprotect in insert_brkPoint");

        // POKETEXT  instruction 복구
        *(unsigned int*)(v->org_data_ptr) = v->org_data_temp;

        // permission 되돌리기
        if(mprotect(v->base, sbuf.st_size, PROT_READ) < 0)
            quit("mprotect in insert_brkPoint");
        if(mprotect(v->base+0x074000, 4096*3, PROT_READ | PROT_WRITE) < 0)
            quit("mprotect in insert_brkPoint");

        int which = (*(v->instr_addr) & 0x0e000000);

        //그 다음 명령어가 B이나 BX류일때는 해서는 안된다
        if(which != is_B)
            insertBrkPoint(v, (unsigned long long)((v->regs[15])+4));

        // roll back
        //v->regs[15]-=4;
        //fetch(v);
        return;
    }
}

void changeValue(struct VCPU* v){
    *(int*)( (v->base)+FILE_OFFSET_OF_DATA_SEG + 0x95088-VADDR_DATA_SEG + ((selectNum-1)*4) ) = num;
}

int quit(char* a){
    printf("err %s\n",a);
    printf("errno: %d, %s\n",errno,strerror(errno));
    exit(1);
}

struct VCPU* vcpuAlloc(void) {
    struct VCPU* ret_val = (struct VCPU*)malloc(sizeof(struct VCPU));
    if(NULL != ret_val)
        memset(ret_val, 0, sizeof(struct VCPU));
    return ret_val;
}

void vcpuDelete(struct VCPU** v) {
    if(NULL != v && NULL != *v) {
        if(NULL != (*v)->stack_base)
            free((*v)->stack_base);
        if(NULL != (*v)->base)
            free((*v)->base);
        free(*v);
        *v = NULL;
    }
}

// call only once
void vcpuSetPc(struct VCPU* v) {

    // 위치를 나중에 바꿔야 함. stack을 mmap으로 하려면
    setReg(v);
    if(NULL == v)
        return;

    // later change to exe_h.vm_eh->e_entry;
    //v->regs[15] = v->base+0xd48;

    //later change to exe_h.vm_eh->e_entry;
    v->regs[15] = ARM_START_OF_CODESEG+0x5dc;
}

int	vcpuLoad(struct VCPU* v, char* vm_file) {
    int fdin;
    unsigned int* src; //32bit instruction
    size_t copysz;
    off_t fsz = 0;

    struct VmHeader	vmh;

    if(NULL == v || NULL == vm_file) return -1;
    if((fdin = open(vm_file, O_RDONLY)) < 0)  quit("open ");
    if(fstat(fdin, &sbuf) < 0) quit("fstat ");

    if((v->base = mmap(0, sbuf.st_size, PROT_READ,MAP_PRIVATE, fdin, 0)) == MAP_FAILED)
        // 0x3e8
        quit("mmap ");

    getHeaders(v->base);
    setHeaders(v->base);

    if(mprotect(v->base+0x074000,4096*3,PROT_READ|PROT_WRITE)<0)
        // 16^3 = 4096, 74000:boundary,memsize(0x020bc) optimized for pagesize,prot
        quit("mprotect ");

    int i = 0;
    src = (unsigned int*)(v->base);

    printf("\n");

    // 숫자 change
    if(research == 'y') changeValue(v);

    void* p_mmap;
    p_mmap = mmap(NULL,REQUESTED_STACKSZ,PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);
    stack_base_mmap =  (unsigned int*)p_mmap; // vm.h

    printf("stackBase_mmap = 0x%llx \n", stack_base_mmap);

    v->stack_base = stack_base_mmap;

    close(fdin);

    return 0;
}


void fetch(struct VCPU* v){
    if(v->regs[15] == 0x88888888) {
        printf("프로그램 실행 완료\n");
        exit(0);
    }

    (v->instr_addr) = v->regs[15] - ARM_START_OF_CODESEG + (v->base);

    // trap 감지
    if((*(v->instr_addr) & 0x000000CC) == 0xCC) brkPointHandler(v);
    v->regs[15] += 4;
}

void decode(struct VCPU* v){

    unsigned int tmp_instr= *(v->instr_addr);
    v->which_mold = (tmp_instr & FI_MASK);
    v->regs[15] += 4;
}

void setReg(struct VCPU* v){//초기값 설정
    //v->regs[0] = 1;
    //v->regs[1] = 0;
    //v->regs[2] = 0;
    //v->regs[3] = 0;
    //v->regs[4] = 0;
    v->regs[11]= 0;            //fp
    v->regs[12]= 0;
    //v->regs[11]=0x77777777;
    //v->regs[14]=0x10f64;     //lr

    v->regs[13]=0x7effef40;    //sp

    // stackptr(regs[13])가 스택의 시작이라고 생각해야함. 아래로 자라남
    //v->regs[13]= v->stackBase + REQUESTED_STACKSZ- 1;
    v->regs[14]=0x88888888;    //lr
    v->cpsr.Z = 1;
    v->cpsr.C = 1;
}


int	vcpuRun(struct VCPU* v) {
    int               counter = 0;
    int __volatile__  do_run = 1;
    if(NULL == v || NULL == v->base)
        return 0;

    // fetch,decode,execute
    while(1 == do_run){
        fetch(v);
        decode(v);
        switch(v->which_mold){
            case is_LDSTR_MUL:
                exeLoadStoreRegister(v);
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
                dataProcess(v); //아래든 이거든  한번에 처리
                printAll(v);
                printf("-----------------------------------------------------------------------------------------------------\n");
                break;
            case is_DP_R:
                dataProcess(v);
                printAll(v);
                printf("-----------------------------------------------------------------------------------------------------\n");
                break;
            case is_B:
                branch(v);
                printAll(v);
                printf("-----------------------------------------------------------------------------------------------------\n");
                break;
            default:
                v->regs[15]-=4;//test용:
                printf("i can't decode this instruction\n");
                break;
        }
    }
    return counter;
}
void printAll(struct VCPU* v){
    if(v->regs[15]-4 == 0x104e4) flag_local++;
    if(v->regs[15]-4 == 0x104fc) flag_local++;

    printf("\n\n");
    unsigned int* stack_ptr_arr[20];
    unsigned long long int stack_ptr_64 =(unsigned long long int)(v->stack_base);
    printf("stackPtr_64 = 0x%llx\n",stack_ptr_64);
    printf("현재 stack pointer = %x\n",v->regs[13]);
    printf("현재 frame pointer = %x\n",v->regs[11]);
    printf("\n");

    for(int i=1; i<17; i++)
        stack_ptr_arr[i] = stack_ptr_64 + REQUESTED_STACKSZ - (i * 4);

    for(int i=1; i<17; i++)
        printf("stackPtr_arr[%d] = %llx\n", i , stack_ptr_arr[i]);

    for(int i=1; i<17; i++)
        printf("stackPtr_arr[%d] = %llx\n", i, *stack_ptr_arr[i]);

    for(int i=0; i<4; i++)
        v->arr[i] = *(int*)((v->base) + FILE_OFFSET_OF_DATA_SEG
                            + 0x95088 - VADDR_DATA_SEG + (i * 4));

    if(v->regs[0] == v->regs_before[0])
        printf("    reg[0]  : 0x%08x ",v->regs[0]);

    else
        printf("    \033[1;32mreg[0]  : 0x%08x\033[0m",v->regs[0]);

    if(v->arr[0] == v->arr_before[0])
        printf("	0x95088(arr[0]):%d \n",
               *(int*)((v->base) + FILE_OFFSET_OF_DATA_SEG + 0x95088-VADDR_DATA_SEG));
    else
        printf("	\033[1;31m95088(arr[0]):%d \033[0m\n",
               *(int*)((v->base)+FILE_OFFSET_OF_DATA_SEG + 0x95088-VADDR_DATA_SEG));

    if(v->regs[1] == v->regs_before[1])
        printf("    reg[1]  : 0x%08x ",v->regs[1]);
    else
        printf("    \033[1;32mreg[1]  : 0x%08x\033[0m",v->regs[1]);

    if(v->arr[1] == v->arr_before[1])
        printf("	0x9508c(arr[1]):%d \n",
               *(int*)((v->base) + FILE_OFFSET_OF_DATA_SEG + 0x95088-VADDR_DATA_SEG + 4));
    else
        printf("	\033[1;31m(0x9508c)arr[1]:%d \033[0m\n",
               *(int*)((v->base) + FILE_OFFSET_OF_DATA_SEG + 0x95088-VADDR_DATA_SEG + 4));

    if(v->regs[2] == v->regs_before[2])
        printf("    reg[2]  : 0x%08x ",v->regs[2]);
    else
        printf("    \033[1;32mreg[2]  : 0x%08x\033[0m", v->regs[2]);

    if(v->arr[2] == v->arr_before[2])
        printf("	0x95090(arr[2]):%d \n",
               *(int*)((v->base) + FILE_OFFSET_OF_DATA_SEG + 0x95088 - VADDR_DATA_SEG + 8));
    else
        printf("	\033[1;31m0x95090(arr[2]):%d \033[0m\n",
               *(int*)((v->base) + FILE_OFFSET_OF_DATA_SEG + 0x95088 - VADDR_DATA_SEG + 8));

    if(v->regs[3] == v->regs_before[3])
        printf("    reg[3]  : 0x%08x ",v->regs[3]);
    else
        printf("    \033[1;32mreg[3]  : 0x%08x\033[0m",v->regs[3]);

    if(v->arr[3] == v->arr_before[3])
        printf("	0x95094(arr[3]):%d \n",
               *(int*)((v->base) + FILE_OFFSET_OF_DATA_SEG + 0x95088 - VADDR_DATA_SEG + 12));
    else
        printf("	\033[1;31m0x95094(arr[3]):%d \033[0m\n",*(int*)((v->base)+FILE_OFFSET_OF_DATA_SEG + 0x95088-VADDR_DATA_SEG+12));

    if(v->regs[4] == v->regs_before[4])
        printf("    reg[4]  : 0x%08x \n",v->regs[4]);
    else
        printf("    \033[1;32mreg[4]  : 0x%08x\n\033[0m",v->regs[4]);

    if(v->regs[5] == v->regs_before[5])
        printf("    reg[5]  : 0x%08x ",v->regs[5]);
    else
        printf("    \033[1;32mreg[5]  : 0x%08x\033[0m",v->regs[5]);

    printf("			Stack segment		\n");

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    if(v->regs[6] == v->regs_before[6])
        printf("    reg[6]  : 0x%08x \n",v->regs[6]);
    else
        printf("    \033[1;32mreg[6]  : 0x%08x\033[0m \n",v->regs[6]);

    if(v->regs[7] == v->regs_before[7])
        printf("    reg[7]  : 0x%08x \n",v->regs[7]);
    else
        printf("    \033[1;32mreg[7]  : 0x%08x\033[0m \n",v->regs[7]);
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    if(v->regs[8] == v->regs_before[8])
        printf("    reg[8]  : 0x%08x \n",v->regs[8]);
    else
        printf("    \033[1;32mreg[8]  : 0x%08x\033[0m \n",v->regs[8]);

    if(v->regs[9] == v->regs_before[9])
        printf("    reg[9]  : 0x%08x \n",v->regs[9]);
    else
        printf("    \033[1;32mreg[9]  : 0x%08x\033[0m \n",v->regs[9]);

    if(v->regs[10] == v->regs_before[10])
        printf("    reg[10] : 0x%08x\n",v->regs[10]);
    else
        printf("    \033[1;32mreg[10] : 0x%08x\n\033[0m",v->regs[10]);

    if(v->regs[11] == v->regs_before[11])
        printf("(fp)reg[11] : 0x%08x\n",v->regs[11]);
    else
        printf("\033[1;32m(fp)reg[11] : 0x%08x\n\033[0m",v->regs[11]);

    if(v->regs[12] == v->regs_before[12])
        printf("(ip)reg[12] : 0x%08x\n",v->regs[12]);
    else
        printf("\033[1;32m(ip)reg[12] : 0x%08x\n\033[0m",v->regs[12]);

    if(v->regs[13] == v->regs_before[13])
        printf("(sp)reg[13] : 0x%08x\n",v->regs[13]);
    else
        printf("\033[1;32m(sp)reg[13] : 0x%08x\033[0m\n",v->regs[13]);

    if(v->regs[14] == v->regs_before[14])
        printf("(lr)reg[14] : 0x%08x\n",v->regs[14]);
    else
        printf("\033[1;32m(lr)reg[14] : 0x%08x\033[0m\n",v->regs[14]);

    if(v->regs[15] == v->regs_before[15])
        printf("(pc)reg[15] : 0x%08x\n",v->regs[15]-4);
    else
        printf("\033[1;32m(pc)reg[15] : 0x%08x\033[0m\n",v->regs[15]-4);

    // i만 표시
    if(v->regs[13] == 0x7effef10 && flag_local == 1 )
        printf("						\033[1;33m지역변수 i\033[0m\n");

    // i,j 둘다표시
    if(v->regs[13] == 0x7effef10 && flag_local > 1)
        printf("						\033[1;33m지역변수 i\033[0m	\033[1;36m지역변수 j\033[0m\n");

    printf("\n");

    for(int i=0; i<16; i++) v->regs_before[i] = v->regs[i];
    for(int j=0; j<4; j++) v->arr_before[j] = v->arr[j];

    printf("\n");
}
#include "VirtualMachine.h"
#include "MultiDataTransfer.h"
#include <stdio.h>

#define depth 4

char *stackJmpr;

unsigned long long int to64StackMultiData(struct VCPU* v){
    long long int initial_sp = INITIAL_SP;
    printf("in To64stack, 0x%llx\n",(unsigned long long int)((-initial_sp + (v->regs[13]))+ (char *)(v->stack_base) + REQUESTED_STACKSZ));
    printf("v->regs[13] = %llx\n",v->regs[13]);
    printf("v->stackBase = %llx\n",v->stack_base);
    return (unsigned long long int)((-initial_sp + (v->regs[13]))+ (char *)(v->stack_base) + REQUESTED_STACKSZ);//INITIAL_SP vm.h
}

void exeLoadStoreRegister(struct VCPU* v){//load store multiple instruction
    unsigned int which = 0;
    which = (*(v->instr_addr)&(PUSH_POP_WHICH));
    switch(which){
        case is_POP:
            exeLoadStoreRegisterPop(v);
            break;
        case is_PUSH:
            exeLoadStoreRegisterPush(v);
            break;
        default:
            (v->regs[15])-=4;
            break;
    }
}

void exeLoadStoreRegisterPush(struct VCPU* v){
    printf("push 명령어 \n");
    int cnt = 0;
    unsigned int tmp_reglist=0;
    tmp_reglist = (*v->instr_addr)&(REG_LIST_MASK);

    // 계속 shift하다보면 0됨
    while(tmp_reglist!=0){

        // full descending 방식
        if((tmp_reglist&1)==1){
            (v->regs[13])= (v->regs[13])-depth;
            v->stack_ptr = to64StackMultiData(v);
            *(v->stack_ptr) = v->regs[cnt];
        }

        tmp_reglist=tmp_reglist>>1;
        cnt++;
    }

    // debugging
    stackJmpr = (char*)v->stack_ptr;

    v->regs[15]-=4;
    return;
}

void exeLoadStoreRegisterPop(struct VCPU* v){
    int is_pc_write = 0;
    int cnt=15;

    printf("pop 명령어\n");
    unsigned int tmp_reglist=0;
    tmp_reglist = (*v->instr_addr)&(REG_LIST_MASK);
    tmp_reglist = tmp_reglist<<16;

    while(tmp_reglist!=0){
        if(cnt==15) is_pc_write = 1;

        // 1000 0000 0000 0000 000..
        if((tmp_reglist & 0x80000000)==0x80000000){
            v->stack_ptr = to64StackMultiData(v);
            v->regs[cnt] = *(v->stack_ptr);
            (v->regs[13]) = (v->regs[13]) + depth;
        }
        tmp_reglist = tmp_reglist<<1;
        cnt--;
    }
    // debugging
    stackJmpr = (char*)v->stack_ptr;

    if(is_pc_write!=1) {v->regs[15]-=4; printf("no pc write\n"); }
    return;
}

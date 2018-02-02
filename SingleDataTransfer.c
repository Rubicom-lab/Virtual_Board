#include "VirtualMachine.h"
#include "SingleDataTransfer.h"
#include "Parse.h"

extern unsigned long long int to64StackSingleData(unsigned long long execute_reg,struct VCPU* v){
    long long int execute_reg_tmp = (long long int)execute_reg;
    long long int initial_sp = INITIAL_SP;
	
    return (unsigned long long int)((-initial_sp + execute_reg_tmp) + (char *)(v->stack_base) + REQUESTED_STACKSZ -4);//stack : 4byte단위
}

// stackptr은 조작하되, reg[13]은 수정해서는 안된다.
void strImmediate(struct VCPU* v){
    printf("str명령어\n");
    struct SingleDataTransfer* instr = (struct SingleDataTransfer*)((v->instr_addr));
    printf("instruction is %llx\n",*instr);
    printf("v->regs[instr->Rd] = 0x%llx\n",v->regs[instr->Rd]);
    unsigned long long execute_reg = v->regs[instr->Rn]; // executeReg는64bit 주소를 담는 변수일 뿐. jmpr과 함께 쓰인다

    if(( (VADDR_DATA_SEG <= v->regs[instr->Rn]) && (VADDR_DATA_SEG+MEMSZ_DATA_SEG > v->regs[instr->Rn]) ))
	{
	    strGlobalData(v);
	    return;
	}

    long long temp = (long long)instr->offset;  // temp는 더 커질 수 있어서 long long으로
    if(instr->U == 0) temp *=-1;
    if(instr->P == 1) (execute_reg)+= temp;     // 항상 일어남 arm특성

    // rn이 pc인 경우.. v->jmpr은 64bit주소로 접근, str에서는 이런 경우가 거의 없지 않나?
    if(instr->Rn == 15) v->jmpr = execute_reg-ARM_START_OF_CODESEG+(v->base);
    else v->jmpr = to64StackSingleData(execute_reg,v);

    *(v->jmpr) = v->regs[instr->Rd];

    // post인 경우와 w==1인 경우인데 !이며, ldr,str에서는 거의 일어나지 않음
    if(instr->W == 1 || instr->P ==0) v->regs[instr->Rn] += temp;

    (v->regs[15])-=4;
}

void strRegister(struct VCPU* v){
    printf("this is str_R(), you have to define it \n");
    exit(1);
}

void strGlobalData(struct VCPU* v){
    struct SingleDataTransfer* instr = (struct SingleDataTransfer*)((v->instr_addr));
    unsigned long long execute_reg = v->regs[instr->Rn];
    long long temp = (long long)instr->offset;// temp는 더 커질 수 있어서 long long으로
    //temp*=2;
    if(instr->U == 0) temp *=-1;

    // 항상 일어남 arm특성
    if(instr->P == 1) (execute_reg)+= temp;

    *(unsigned int*)((v->base) + FILE_OFFSET_OF_DATA_SEG + (execute_reg-VADDR_DATA_SEG)) = v->regs[instr->Rd];

    // post인 경우와 w==1인 경우인데 !이며, ldr,str에서는 거의 일어나지 않음
    if(instr->W == 1 || instr->P ==0) v->regs[instr->Rn] += temp;

    (v->regs[15])-=4;
    return;
}

// stackptr은 조작하되, reg[13]은 수정해서는 안된다.
void ldrImmediate(struct VCPU* v){
    printf("ldr 명령어\n");
    struct SingleDataTransfer* instr = (struct SingleDataTransfer*)((v->instr_addr));
    printf("instruction is %llx\n",*instr);
    printf("v->regs[instr->Rd] = 0x%llx\n",v->regs[instr->Rd]);
    unsigned long long execute_reg = v->regs[instr->Rn];


    if((VADDR_DATA_SEG <= v->regs[instr->Rn])
        && (VADDR_DATA_SEG + MEMSZ_DATA_SEG > v->regs[instr->Rn])) {
	    ldrGlobalData(v);
	    return;
	}

    long long temp = (long long)instr->offset; // temp는 더 커질 수 있어서 long long으로
    if(instr->U == 0) temp *=-1;

    // 항상 일어남 arm특성
    if(instr->P == 1) (execute_reg)+= temp;

    // v->jmpr은 64bit주소로 접근
    if(instr->Rn == 15) v->jmpr = execute_reg-ARM_START_OF_CODESEG+(v->base);
    else v->jmpr = to64StackSingleData(execute_reg,v);

    v->regs[instr->Rd] = *(v->jmpr);

    // postindex인 경우와 w==1인 경우인데 !이며, ldr,str에서는 거의 일어나지 않음
    if(instr->W == 1 || instr->P ==0) v->regs[instr->Rn] += temp;

    (v->regs[15])-=4;
    return;
}

// stackptr은 조작하되, reg[13]은 수정해서는 안된다.
void ldrGlobalData(struct VCPU* v){
    struct SingleDataTransfer* instr = (struct SingleDataTransfer*)((v->instr_addr));
    unsigned long long execute_reg = v->regs[instr->Rn];
    long long temp = (long long)instr->offset;  // temp는 더 커질 수 있어서 long long으로

    if(instr->U == 0) temp *= -1;
    if(instr->P == 1) (execute_reg) += temp;    // 항상 일어남 arm특성

    v->regs[instr->Rd]=*(unsigned int*)((v->base)+FILE_OFFSET_OF_DATA_SEG+(execute_reg-VADDR_DATA_SEG));

    // post인 경우와 w==1인 경우인데 !이며, ldr,str에서는 거의 일어나지 않음
    if(instr->W == 1 || instr->P ==0) v->regs[instr->Rn] += temp;

    (v->regs[15])-=4;
    return;
}

void ldrRegister(struct VCPU* v){
    printf("this is ldr_R(), you have to define it \n");
    exit(1);
}

void SDTransfer_I(struct VCPU* v){
    struct SingleDataTransfer* instr = (struct SingleDataTransfer*)(v->instr_addr);
    if(instr->L == 0) strImmediate(v);
    else ldrImmediate(v);
}
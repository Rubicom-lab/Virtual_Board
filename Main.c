#include "VirtualMachine.h"
#include <stdio.h>
#include <stdlib.h>

int main(int argc,char *argv[] )
{
    struct VCPU*        cpu = vcpuAlloc();
    unsigned long long	brk_point;

    printf("배열값을 바꾸겠습니까?  y/n \n");
    scanf("%c", &research);

    if(research == 'y'){
        printf("현재 arr[] = {1,5,4,3}\n 바꾸고자 하는 숫자를 선택해주세요. \n 1번 : 1 \n 2번 : 5 \n 3번 : 4 \n 4번 : 3 \n");
        scanf("%d", &selectNum);
        printf("원하는 값을 넣어주세요 \n");
        scanf("%d", &num);
    }

    printf("break Point를 설정하시요  예)0x105e0 \n");
    scanf("%llx", &brk_point);


    vcpuLoad(cpu, argv[1]);
    printf("cpu->base : %llx\n", cpu->base);

    insertBrkPoint(cpu, brk_point);

    // entry point로pc 설정
    vcpuSetPc(cpu);
    printf("pc : %llx\n", cpu->regs[15]);
    vcpuRun(cpu);

    free(cpu);
    return 0;
}

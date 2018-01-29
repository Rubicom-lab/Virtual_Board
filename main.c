#include "vm.h"
#include <stdio.h>
#include <string.h>
//#define MAIN_OFFSET 0xcfc 

int main(int argc,char *argv[] )
{
    VCPU*			cpu = vcpu_alloc();
    //unsigned long long	main;
    unsigned long long	brkPoint;
    int 			i;
	
    printf("배열값을 바꾸겠습니까?  y/n \n");
    scanf("%c", &research);

    if(research == 'y'){
	printf("현재 arr[] = {1,5,4,3}\n 바꾸고자 하는 숫자를 선택해주세요. \n 1번 : 1 \n 2번 : 5 \n 3번 : 4 \n 4번 : 3 \n");	
	scanf("%d", &selectNum);
	printf("원하는 값을 넣어주세요 \n");
	scanf("%d", &num);
    }
    else;

    printf("break Point를 설정하시요  예)0x105e0 \n");
    scanf("%llx", &brkPoint);


    vcpu_load(cpu, argv[1]);
    printf("cpu->base : %llx\n", cpu->base);

    insert_brkPoint(cpu,brkPoint);

    vcpu_set_pc(cpu);//entry point로pc 설정
    printf("pc : %llx\n", cpu->regs[15]);
    vcpu_run(cpu);
	
    free(cpu);
    return 0;
}

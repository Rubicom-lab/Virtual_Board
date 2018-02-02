#include "VirtualMachine.h"
#include "BitsHandler.h"


/*
 * 비트열의 수정, 가공하는 기능의 함수들 모음
 *
 * src : 원본 비트열
 * start : 시작 비트 위치
 * len : 가져올 비트 개수
 *
 * start부터 오른쪽으로 len개 가져온다.
 */
unsigned int getBins(int src, int start, int len) {
    int tmp = 0, mask = 1;

    mask = ((1 << len) - 1) << (start - len);
    tmp = (mask & src) >> (start - len);

    return tmp;
}

/* 비트열의 shift 담당
 *
 *	C에서 기본적으로 제공하는 <<과 >>는 lsl과 asr이다.
 *	lsr의 경우는 앞쪽을 전부 0으로 채워야 되기때문에
 *	bit의 32번째 값이 1일경우 32번째 비트값을 1빼주면 된다.
 */
unsigned int lsl(unsigned int bits, unsigned int count) {
    return bits <<= count;
}

unsigned int lsr(unsigned int bits, unsigned int count) {
    if (count > 0) {
        bits >>= 1;
        if (((bits >> 31) & 1) == 1) bits -= 1 << 31;
        if (count > 1) bits >>= count - 1;
    }

    return bits;
}

unsigned int asr(unsigned int bits, unsigned int count) {
    return bits >>= count;
}

unsigned int ror(unsigned int bits, unsigned int count) {
    if (count > 0) {
        unsigned int tmp = getBins(bits, count, count);

        bits = lsr(bits, count);
        bits += tmp << (32 - count);
    }

    return bits;
}

/*
* operand : 12비트의 operand2
* imme : 1이면 상수 0이면 레지스터
*
* operand 분석, 쉬프트 처리 후 알맞은 값 반환.
*/
unsigned int shift(struct VCPU* v,unsigned int operand, unsigned int imme) {
    unsigned int shift_count = 0;
    unsigned int shifted_val = 0;

    // I비트==1, MOV(immediate)경우
    if (imme == 1) {
        shift_count = getBins(operand, 12, 4) << 1;//shift 몇만큼 해줄건지 비트8부터 비트 11까지. 그리고 곱하기2
        shifted_val = ror(getBins(operand, 8, 8), shift_count);
    }

    // I비트==0
    else {
        unsigned int shift_oper = getBins(operand, 7, 2);
        unsigned int rn = v->regs[getBins(operand, 4, 4)];  // =Rm
        shift_count = getBins(operand, 12, 5);

        if (((operand >> 4) & 1) == 1) {
            shift_count = v->regs[lsr(shift_count, 1)];
        }

        switch (shift_oper) {
            case 0:shifted_val = lsl(rn, shift_count); break;
            case 1:shifted_val = lsr(rn, shift_count); break;
            case 2:shifted_val = asr(rn, shift_count); break;
            case 3:shifted_val = ror(rn, shift_count); break;
        }
    }

    return shifted_val;
}

void cpsrUpdate(struct VCPU* v,unsigned int num1, unsigned int num2, long long result){//return cpsr 배열
    v->cpsr.N= ((result >> 31) & 1ll) == 1 ? 1 : 0;
    v->cpsr.Z= (unsigned int) result == 0 ? 1 : 0;
    v->cpsr.C= result > 0xffffffff ? 1 : 0;
    v->cpsr.V= (((num1 ^ num2) >> 31) & 1) == 0 && (((num1 ^ result) >> 31) & 1ll) == 1 ? 1 : 0;
}

int conditionPassed(struct VCPU* v, unsigned int cond) {//1 리턴-> condition pass
    switch (cond) {
        case 0: return v->cpsr.Z;
        case 1: return !v->cpsr.Z;
        case 2: return v->cpsr.C;
        case 3: return !v->cpsr.C;
        case 4: return v->cpsr.N;
        case 5: return !v->cpsr.N;
        case 6: return v->cpsr.V;
        case 7: return !v->cpsr.V;
        case 8: return v->cpsr.C & !v->cpsr.Z;
        case 9: return !v->cpsr.C | v->cpsr.Z;
        case 10: return !(v->cpsr.N ^ v->cpsr.V);//^ : 같은면 0
        case 11: return v->cpsr.N ^ v->cpsr.V;
        case 12: return !v->cpsr.Z & !(v->cpsr.N ^ v->cpsr.V);
        case 13: return v->cpsr.Z | (v->cpsr.N ^ v->cpsr.V);
        case 14: return 1;
        case 15: return 0;
    }
}
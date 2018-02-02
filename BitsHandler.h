#ifndef VIRTUAL_BOARD_BITSHANDLER_H
#define VIRTUAL_BOARD_BITSHANDLER_H

unsigned int getBins(int src, int start, int len);
unsigned int lsl(unsigned int bits, unsigned int count);
unsigned int lsr(unsigned int bits, unsigned int count);
unsigned int asr(unsigned int bits, unsigned int count);
unsigned int ror(unsigned int bits, unsigned int count);
unsigned int shift(struct VCPU* v,unsigned int operand, unsigned int imme);
void cpsrUpdate(struct VCPU* v,unsigned int num1, unsigned int num2, long long result);
int conditionPassed(struct VCPU* v, unsigned int cond);

#endif //VIRTUAL_BOARD_BITSHANDLER_H

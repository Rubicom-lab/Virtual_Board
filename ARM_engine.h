//#ifndef ARM_ENGINE_H
//#define ARM_ENGINE_H
#include "vm.h"


//void str_I(VCPU* v);
//void str_R(VCPU* v);
//void ldr_I(VCPU* v);
//void ldr_R(VCPU* v);
void SDTransfer_I(VCPU* v);
void SDTransfer_R(VCPU* v);
void execute_LDSTR(VCPU* v);
//void execute_LDSTR_PUSH(VCPU* v);
//void execute_LDSTR_POP(VCPU* v);
void DataProcess(VCPU *V);
void B(VCPU *v);

unsigned int getBins(int src, int start, int len);
unsigned int lsl(unsigned int bits, unsigned int count);
unsigned int lsr(unsigned int bits, unsigned int count);
unsigned int asr(unsigned int bits, unsigned int count);
unsigned int ror(unsigned int bits, unsigned int count);
unsigned int shift(VCPU* v,unsigned int operand, unsigned int imme);


//#endif

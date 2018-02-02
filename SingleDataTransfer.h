#ifndef VIRTUAL_BOARD_SINGLEDATATRANSFER_H
#define VIRTUAL_BOARD_SINGLEDATATRANSFER_H

extern unsigned long long int to64StackSingleData(unsigned long long executeReg,struct VCPU* v);

void strImmediate(struct VCPU* v);

void strRegister(struct VCPU* v);

void strGlobalData(struct VCPU* v);

void ldrImmediate(struct VCPU* v);

void ldrGlobalData(struct VCPU* v);

void ldrRegister(struct VCPU* v);

void SDTransfer_I(struct VCPU* v);

#endif //VIRTUAL_BOARD_SINGLEDATATRANSFER_H

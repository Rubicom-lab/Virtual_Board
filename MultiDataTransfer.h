#ifndef VIRTUAL_BOARD_MULTIDATATRANSFER_H
#define VIRTUAL_BOARD_MULTIDATATRANSFER_H

unsigned long long int to64StackMultiData(struct VCPU* v);

void exeLoadStoreRegister(struct VCPU* v);

void exeLoadStoreRegisterPush(struct VCPU* v);

void exeLoadStoreRegisterPop(struct VCPU* v);

#endif //VIRTUAL_BOARD_MULTIDATATRANSFER_H

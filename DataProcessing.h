
#ifndef VIRTUAL_BOARD_DATAPROCESSING_H
#define VIRTUAL_BOARD_DATAPROCESSING_H

void mov(struct VCPU* v);

void add(struct VCPU* v);

void sub(struct VCPU* v);

void cmp(struct VCPU *v);

void mul(struct VCPU* v);

void BX(struct VCPU* v);

void dataProcess(struct VCPU* v);

#endif //VIRTUAL_BOARD_DATAPROCESSING_H

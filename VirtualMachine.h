#ifndef _VM_
#define _VM_
#define REQUESTED_STACKSZ 0x00100000
#define INITIAL_SP 0x7effef40

// for debugging
int	  selectNum;
int   num;
char  research;

// for bubbleSort
#define VADDR_DATA_SEG 0x000094f6c         //exe_h.vm_ph[data_idx].p_vaddr //0x000097458
#define MEMSZ_DATA_SEG 0x00001b24          //exe_h.vm_ph[data_idx].p_memsz //0x000020bc
#define FILE_OFFSET_OF_DATA_SEG 0x074f6c   //exe_h.vm_ph[data_idx].p_offset //0x077458

// masks global
#define FI_MASK 0x0e000000                 //bit F,I mask. F,I: instruction type

// masks local
#define PUSH_POP_WHICH 0x01f00000          //push? pop?
#define ARM_START_OF_CODESEG 0x10000

// masks global result
#define is_LDSTR_MUL 0x08000000            //is load store multiple 0x08000000
#define is_LDSTR_SINGLE_I 0x04000000
#define is_LDSTR_SINGLE_R 0x06000000
#define is_DP_I 0x02000000
#define is_DP_R 0x00000000
#define is_B 0x0a000000

// masks local result
#define is_PUSH 0x01200000                 //it is push. 나중에 5bit를 더 세분화 필요.
#define is_POP 0x00b00000

#define REG_LIST_MASK 0x0000ffff           //used for push,pop

// Header
struct VmHeader {
    unsigned int version;
    unsigned int code_offset;
    unsigned int code_size;
    unsigned int data_offset;
    unsigned int data_size;
    unsigned int export_offset;
    unsigned int export_size;
    unsigned int requested_stack;
    unsigned int file_size;
};


// cpsr register
struct CPSR {
    unsigned short mode   :5;
    unsigned short t      :1;
    unsigned short f      :1;
    unsigned short i      :1;
    unsigned short empty1 :16;
    unsigned short j      :1;
    unsigned short empty  :2;
    unsigned short Q      :1;
    unsigned short V      :1;
    unsigned short C      :1;
    unsigned short Z      :1;
    unsigned short N      :1;

};

// Our virtual CPU
struct VCPU {
    unsigned int        regs[15];         // Registers
    unsigned int*       stack_base;       // Stack pointer and stack base
    unsigned char*      base;             // Base address of the image (where the pseudo executable is loaded to). Actually, the pointer to the buffer
    struct CPSR         cpsr;
    unsigned int*       instr_addr;       // pointer로는 64bit vm에서 메모리 접근용 + *(내용)으로는 32bit 명령어
    unsigned int*       stack_ptr;        // stack data조작을 위해서는 따로 필요
    unsigned int*       jmpr;             // 32bit 명령어 단위
    unsigned int        which_mold;       // decode()에서 사용

    // 디버깅 용
    unsigned long long* org_data_ptr;     // insert_brkPoint()에서 사용
    unsigned int        org_data_temp;    // brkPoint_handler()에서 사용
    int                 state;            // state machine
    int                 flag;             // for debugging
    unsigned int        regs_before[15];  // for debugging
    unsigned int        arr_before[4];    // for debugging
    unsigned int        arr[4];           // for debugging
};

/*
 * Instruction encoding
 * 하나의 instruction은(32비트)구성으로 수정해야 함
 */
struct Instruction0 {
    unsigned short operand2  :12;
    unsigned short Rd        :4;
    unsigned short Rn        :4;
    unsigned short S         :1;
    unsigned short opcode    :4;
    unsigned short I         :1;
    unsigned short F         :2;
    unsigned short cond      :4;
};

struct SingleDataTransfer {
    unsigned short offset  :12;
    unsigned short Rd      :4;
    unsigned short Rn      :4;
    unsigned short L       :1;
    unsigned short W       :1;
    unsigned short B       :1;
    unsigned short U       :1;
    unsigned short P       :1;
    unsigned short I       :1;
    unsigned short F       :2;
    unsigned short Cond    :4;
};

struct DataProcessing {
    unsigned short operand2 :12;
    unsigned short Rd       :4;
    unsigned short Rn       :4;
    unsigned short S        :1;
    unsigned short opcode   :4;
    unsigned short I        :1;
    unsigned short F        :2;
    unsigned short cond     :4;
};

struct Multiply {
    unsigned int Rn    :4;
    unsigned int nine  :4;
    unsigned int Rm    :4;
    unsigned int empty :4;
    unsigned int Rd    :4;
    unsigned int S     :1;
    unsigned int zero  :7;
    unsigned int cond  :4;
};


struct Branch {
    unsigned int offset :24;
    unsigned int L      :1;
    unsigned int F      :3;
    unsigned int cond   :4;
};

struct BXWritePC {
    unsigned int Rm       :4;
    unsigned int constant :24;
    unsigned int cond     :4;
};

unsigned int*  stack_base_mmap;

extern void             insertBrkPoint(struct VCPU* v, unsigned long long brk_point);

void                    brkPointHandler(struct VCPU* v);

void                    changeValue(struct VCPU* v);

int                     quit(char* a);

// Allocation of Virtual CPU
extern struct VCPU*	    vcpuAlloc(void);

// Deallocation of Virtual CPU and whatever it has pointers to
extern void	            vcpuDelete(struct VCPU** v);

// Sets instruction pointer (you pass actual pointer, not file offset!)
extern void	            vcpuSetPc(struct VCPU* v);

// Loading pseudo executable and stack allocation
extern int		        vcpuLoad(struct VCPU* v, char* vm_file);

extern void             fetch(struct VCPU* v);

extern void             decode(struct VCPU* v);

void                    setReg(struct VCPU* v);

// Runs VM starting from current IP
extern int              vcpuRun(struct VCPU* v);

void                    printAll(struct VCPU* v);

#endif

#ifndef _VM_
#define _VM_
#define REQUESTED_STACKSZ 0x00100000
//#define REQUESTED_STACKSZ 0x100000
//#define REQUESTED_STACKSZ 0x40
#define INITIAL_SP 0x7effef40

/* for facto
   #define VADDR_DATA_SEG 0x000097458         //exe_h.vm_ph[data_idx].p_vaddr //0x000097458
   #define FILESZ_DATA_SEG 0x00000f7c        //exe_h.vm_ph[data_idx].p_filesz //0x00000f7c
   #define MEMSZ_DATA_SEG 0x000020bc         //exe_h.vm_ph[data_idx].p_memsz //0x000020bc
   #define FILE_OFFSET_OF_DATA_SEG 0x077458 //exe_h.vm_ph[data_idx].p_offset //0x077458
*/

//for debugging
int			selectNum;
int			num;
char			research;

//for bubbleSort
#define VADDR_DATA_SEG 0x000094f6c         //exe_h.vm_ph[data_idx].p_vaddr //0x000097458
#define FILESZ_DATA_SEG 0x00000f7c        //exe_h.vm_ph[data_idx].p_filesz //0x00000f7c
#define MEMSZ_DATA_SEG 0x00001b24         //exe_h.vm_ph[data_idx].p_memsz //0x000020bc
#define FILE_OFFSET_OF_DATA_SEG 0x074f6c //exe_h.vm_ph[data_idx].p_offset //0x077458



//masks global
#define FI_MASK 0x0e000000//bit F,I mask. F,I: instruction type
#define TEST_MASK 0xffffffff//test용
//masks local
#define PUSH_POP_WHICH 0x01f00000//push? pop?
//#define LDR_STR_WHICH 0x00100000

#define ARM_START_OF_CODESEG 0x10000

//masks global result 
#define is_LDSTR_MUL 0x08000000//is load store multiple 0x08000000
#define is_LDSTR_SINGLE_I 0x04000000
#define is_LDSTR_SINGLE_R 0x06000000
#define is_DP_I 0x02000000
#define is_DP_R 0x00000000
#define is_B 0x0a000000

//masks local result 
#define is_PUSH 0x01200000//it is push. 나중에 5bit를 더 세분화 필요.
#define is_POP 0x00b00000
//#define is_LDR 0x00100000//L비트
//#define is_STR 0x00000000

#define REG_LIST_MASK 0x0000ffff//used for push,pop

/*	Header	*/
typedef struct _VM_HEADER
{
    unsigned int			version;
    unsigned int			codeOffset;
    unsigned int			codeSize;
    unsigned int			dataOffset;
    unsigned int			dataSize;
    unsigned int			exportOffset;
    unsigned int			exportSize;
    unsigned int			requestedStack;
    unsigned int			fileSize;
}VM_HEADER;


/*cpsr register*/
typedef struct _CPSR
{
    unsigned short mode:5;
    unsigned short t:1;
    unsigned short f:1;
    unsigned short i:1;
    unsigned short empty1:16;
    unsigned short j:1;
    unsigned short empty: 2;
    unsigned short Q:1;
    unsigned short V:1;
    unsigned short C:1;
    unsigned short Z:1;
    unsigned short N:1;

}CPSR;

/*	Our virtual CPU	*/
typedef struct _VCPU
{
    unsigned int regs[15];					/*	Registers	*/
    unsigned int *stackBase;		/*	Stack pointer and stack base	*/
    //unsigned long long	*stack_Base;		/*	Stack pointer and stack base	*/
    unsigned char		*base;		/*	Base address of the image (where the pseudo executable is loaded to). Actually, the pointer to the buffer	*/
    CPSR cpsr;
    unsigned int *instr_addr;//pointer로는 64bit vm에서 메모리 접근용 + *(내용)으로는 32bit 명령어
    unsigned int *stackptr;//stack data조작을 위해서는 따로 필요
    unsigned int *jmpr;//32bit 명령어 단위
    unsigned int which_mold;//decode()에서 사용
    /*디버깅 용*/
    unsigned long long *orgData_ptr;//insert_brkPoint()에서 사용
    unsigned int orgData_temp;//brkPoint_handler()에서 사용
    int state;//state machine
    int flag;//for debugging
    unsigned int regs_before[15];//for debugging
    unsigned int arr_before[4];//for debugging
    unsigned int arr[4];//for debugging
}VCPU;


/*	Registers	*/
#define	REG_A			0
#define	REG_B			1
#define REG_C			2
#define REG_D 			3


//struct *instr_choice;//구조체 포인터.

/*	Instruction encoding	*/
typedef struct _INSTRUCTION0//하나의 instruction은(32비트)구성으로 수정해야 함 
{
    unsigned short operand2:12;
    unsigned short Rd:4;
    unsigned short Rn:4;
    unsigned short S:1;
    unsigned short opcode:4;
    unsigned short I:1;
    unsigned short F:2;
    unsigned short cond:4;
}INSTRUCTION0;

typedef struct _SingleDataTransfer
{
    unsigned short offset : 12;
    unsigned short Rd : 4;
    unsigned short Rn : 4;
    unsigned short L : 1;
    unsigned short W : 1;
    unsigned short B : 1;
    unsigned short U : 1;
    unsigned short P : 1;
    unsigned short I : 1;
    unsigned short F : 2;
    unsigned short Cond : 4;
}SingleDataTransfer;

typedef struct _DataProcessing{
    unsigned short operand2:12;
    unsigned short Rd:4;
    unsigned short Rn:4;
    unsigned short S:1;
    unsigned short opcode:4;
    unsigned short I:1;
    unsigned short F:2;
    unsigned short cond:4;
}DataProcessing;

typedef struct _Multiply{

    unsigned int Rn:4;
    unsigned int _nine:4;
    unsigned int Rm:4;
    unsigned int empty:4;
    unsigned int Rd:4;
    unsigned int S:1;
    unsigned int _zero:7;
    unsigned int cond:4;
}Multiply;


typedef struct _Branch{
    unsigned int offset:24;
    unsigned int L:1;
    unsigned int F:3;
    unsigned int cond:4;
}Branch;

typedef struct _BXWritePC{
    unsigned int Rm:4;
    unsigned int _constant:24;
    unsigned int cond:4;
}BXWritePC;




/*	Operand types	*/
//#define OP_REG			0	/*	Register operand	*/
//#define OP_IMM			1	/*	Immediate operand	*/
//#define OP_MEM			2	/*	Memory reference	*/
//#define OP_NONE			3	/*	No operand (optional)	*/

/*	Operand sizes	*/
#define OP_SIZE_BYTE	0
#define OP_SIZE_WORD	1
#define OP_SIZE_DWORD	2
#define _BYTE			OP_SIZE_BYTE
#define _WORD			OP_SIZE_WORD
#define _DWORD			OP_SIZE_DWORD

/*	Operation direction	*/
#define DIR_LEFT		0
#define DIR_RIGHT		1


#define END_MARKER		0xDEADC0DE

/*	Macros	*/
//#define MPUSH(cpu, val)	(*(--(cpu)->stackPtr) = (val))
//#define MPOP(cpu, dest)	((dest) = *((cpu)->stackPtr++))

/*	Value		*/
//unsigned long long	*stackBase_mmap;		
unsigned int	*stackBase_mmap;		

/*	Functions	*/
extern	VCPU*	vcpu_alloc(void);										/* Allocation of Virtual CPU */
extern	void	vcpu_delete(VCPU** v);									/* Deallocation of Virtual CPU and whatever it has pointers to */
extern	int		vcpu_load(VCPU* v, char* vm_file);						/* Loading pseudo executable and stack allocation */
extern	unsigned long	vcpu_get_exported_entry(VCPU* v, char* name);	/* Returns a pointer to the exported entry or NULL */
extern 	void	vcpu_set_pc(VCPU* v);					/* Sets instruction pointer (you pass actual pointer, not file offset!)*/
extern	int		vcpu_run(VCPU* v);										/* Runs VM starting from current IP */
extern void fetch(VCPU* v);
//extern void decode(VCPU* v, INSTRUCTION* instruction);
//extern void execute(VCPU* v, INSTRUCTION* instruction);
extern void decode(VCPU* v);
extern void execute1(VCPU* v);
extern void get_Headers(unsigned long long *mmap_addr);
extern void set_Headers(unsigned long long *mmap_addr);
extern void free_ARM_Headers();
extern void get_Data_Info(int i);
extern void insert_brkPoint(VCPU* v, unsigned long long brkPoint);
//extern void print_VM_Headers(); not essential
//extern void print_ARM_Headers(); not essential
#endif	/*	_VM_	*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#define EI_INDENT 16

struct ArmElfHeader {
     unsigned char   e_indent[EI_INDENT];  // 파일의 내용을 해석,디코딩을 위해 사용
     unsigned short  e_type;               // 파일의 타입표시
     unsigned short  e_machine;            // architecture 정보표시
     unsigned int    e_version;            // 파일의 버전정보표시
     unsigned int    e_entry;              // entry point 저장
     unsigned int    e_phoff;              // 프로그램 헤더 테이블의 파일 offset을 byte 단위
     unsigned int    e_shoff;              // 섹션 헤더 테이블의 파일 offset
     unsigned int    e_eflags;             // 파일과 관련된 프로세서의 특수 flag
     unsigned short  e_ehsize;             // elf 헤더의 크기
     unsigned short  e_phentsize;          // 프로그램 헤더 테이블에 있는 엔트리의 크기
     unsigned short  e_phnum;              // 프로그램 헤더테이블의 엔트리 갯수
     unsigned short  e_shentsize;          // 섹션 헤더의 크기
     unsigned short  e_shnum;              // 섹션 헤더 테이블에 있는 엔트리 갯수
     unsigned short  e_shstrndx;           // 섹션의 이름을 나타내는 string 테이블, 섹션헤더테이                   블
};

struct VmElfHeader {
    unsigned long long  e_entry;
    unsigned int        e_phoff;
    unsigned int        e_shoff;
    unsigned int        e_ehsize;
    unsigned int        e_phnum;
    unsigned int        e_phentsize;
    unsigned int        e_shnum;
    unsigned int        e_shentsize;
};

struct ArmProgramHeader {
     unsigned int p_type;     //세그먼트 형식
     unsigned int p_offset;   //세그먼트 오프셋
     unsigned int p_vaddr;    //세그먼트 가상주소
     unsigned int p_paddr;    //세그먼트 물리주소
     unsigned int p_filesz;   //파일에서의 세그먼트 크기
     unsigned int p_memsz;    //메모리에서의 세그먼트 크기
     unsigned int p_flags;    //세그먼트 플래그 ex)Read,Write
     unsigned int p_align;    //메모리에서의 세그먼트 정렬
};

struct VmProgramHeader {
    char               p_name[10];
    unsigned int       p_offset;
    unsigned long long p_vaddr;
    unsigned int       p_filesz;
    unsigned int       p_memsz;
    unsigned int       p_flags;
    unsigned int       p_align;
};

struct ArmSectionHeader {
    unsigned int    sh_name;        //shdr 이름을 나타내는 shdr 문자열 테이블 오프셋
    unsigned int    sh_type;        //shdr 형식 ex) SHT_PROGBITS
    unsigned int    sh_flags;       //shdr 플래그 ex) SHT_WRITE | SHT_ALLOC
    unsigned int    sh_addr;        //섹션 시작 주소
    unsigned int    sh_offset;      //파일의 엔트리 포인트부터의 shdr 오프셋
    unsigned int    sh_size;        //디스크에 할당된 섹션 크기
    unsigned int    sh_link;        //다른 섹션으로의 포인터
    unsigned int    sh_info;        //섹션 형식
    unsigned int    sh_addralign;   //섹션 재배열 주소
    unsigned int    sh_entsize;     //섹션 엔트리 크기
};

struct ArmExeHeaders {
    struct ArmElfHeader*      e_h;
    struct ArmProgramHeader*  p_h;
    struct ArmSectionHeader*  s_h;
};

struct VmExeHeaders {
    struct VmElfHeader*     vm_eh;
    struct VmProgramHeader* vm_ph;
};

struct ArmExeHeaders  arm_exe_h;
struct VmExeHeaders   exe_h;
int                   data_idx;

void                    printArmHeaders();

void                    printVmHeaders();

extern void             getHeaders(unsigned long long *mmap_addr);

extern void             freeArmHeaders();

extern void             setHeaders(unsigned long long *mmap_addr);

extern void             getDataInfo(int i);


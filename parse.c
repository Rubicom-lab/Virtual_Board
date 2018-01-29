#include "parse.h"

void print_ARM_Headers() // print all headers info (same as readelf inst)
{
    int i=0;
    int ph_ent=0;
    int sh_ent=0;
    
    printf("ELF HEADER:\n");
    printf("\tMagic:\t");
    for(i=0 ; i<EI_INDENT;i++)
    {
         printf("%02x ", (char *)(arm_exe_h.e_h->e_indent[i]));
    }

    printf("\n\tEntry Point address : \t\t0x%x",arm_exe_h.e_h->e_entry);
    printf("\n\tStart of program headers : \t%d (0x%x) bytes into file",arm_exe_h.e_h->e_phoff, arm_exe_h.e_h->e_phoff);
    printf("\n\tStart of section headers : \t%d (0x%x) bytes into file",arm_exe_h.e_h->e_shoff, arm_exe_h.e_h->e_shoff);
    printf("\n\tSize of this header: \t\t%d (bytes)",arm_exe_h.e_h->e_ehsize);
    printf("\n\tNumber of program headers : \t%d",arm_exe_h.e_h->e_phnum);
    printf("\n\tSize of program header : \t%d (bytes)",arm_exe_h.e_h->e_phentsize);
    printf("\n\tNumber of section headers : \t%d",arm_exe_h.e_h->e_shnum);
    printf("\n\tSize of section header : \t%d (bytes)",arm_exe_h.e_h->e_shentsize);
    printf("\n\nProgram Headers:\n  Type\t\tOffset\t\tVirtAddr\tPhysAddr\tFilesiz\tMemSiz\tFlg\tAlign\n");

    ph_ent=arm_exe_h.e_h->e_phnum;
     
    for(i=0;i<ph_ent;i++)
    {
        char Type[20];
        char Flag[10];
        
        switch(arm_exe_h.p_h[i].p_type)
        {
  
            case 0x1: strcpy(Type,"LOAD");break;
            case 0x4: strcpy(Type,"NOTE");break;
            case 0x7: strcpy(Type,"TLS");break;
            case 0x70000001: strcpy(Type,"EXIDX");break;
            default :strcpy(Type,"GNU_STACK");break;
        }
        
        switch(arm_exe_h.p_h[i].p_flags & 0b111)
        {
            case 0b001: strcpy(Flag,"  E");break;
            case 0b010: strcpy(Flag," W ");break;
            case 0b100: strcpy(Flag,"R  ");break;
            case 0b011: strcpy(Flag," WE");break;
            case 0b101: strcpy(Flag,"R E");break;
            case 0b110: strcpy(Flag,"RW ");break;
            case 0b111: strcpy(Flag,"RWE");break;
            default : strcpy(Flag,"   ");break;
        }
    
        
        printf("  %-8s\t0x%06x\t0x%08x\t0x%08x\t0x%05x\t0x%05x\t%s\t0x%x\n",Type 
                 , arm_exe_h.p_h[i].p_offset, arm_exe_h.p_h[i].p_vaddr, arm_exe_h.p_h[i].p_paddr,
                 arm_exe_h.p_h[i].p_filesz, arm_exe_h.p_h[i].p_memsz, Flag, arm_exe_h.p_h[i].p_align);
    }
   
    sh_ent=arm_exe_h.e_h->e_shnum;
        
    printf("\nSection Headers:\n");
    printf("[Nr]  Name\t\tType\t\tAddr\t\tOff\tSize\tES   Flg  Lk Inf Al\n");
    
    unsigned int symtab_base;
    char Type[10];
    char Flag[10];
    char Name[20];
    i=0;
    while(1)
    {
     
        if(arm_exe_h.s_h[i].sh_type == 0x02)
        {
            symtab_base=arm_exe_h.s_h[i].sh_offset;
            i=0;
            break;
        }
        i++;
    }
    
    for(i=0;i<sh_ent;i++)
    {
        switch(arm_exe_h.s_h[i].sh_type)
        {
            case 0x00: strcpy(Type,"NULL");break;
            case 0x07: strcpy(Type,"NOTE");break;
            case 0x01: strcpy(Type,"PROGBITS");break;
            case 0x70000001: strcpy(Type,"ARM_EXIDX");break;
            case 0x08: strcpy(Type,"NOBITS");break;
            case 0x0e: strcpy(Type,"INIT_ARRAY");break;
            case 0x0f: strcpy(Type,"FINI_ARRAY");break;
            case 0x70000003: strcpy(Type,"ARM_ATTRIBUTE");break;
            case 0x03: strcpy(Type,"STRTAB");break;
            case 0x02: strcpy(Type,"SYMTAB");break;
        }
       
    
        switch(arm_exe_h.s_h[i].sh_flags)
        {
            case 0x2: strcpy(Flag,"A");break;
            case 0x6: strcpy(Flag,"AX");break;
            case 0x82: strcpy(Flag,"AL");break;
            case 0x403: strcpy(Flag,"WAT");break;
            case 0x3: strcpy(Flag,"WA");break;
            case 0x30: strcpy(Flag,"MS");break;
            case 0x00: strcpy(Flag,"   ");break;
        }
    
        

        printf("[%2d]  %-10x\t%-10s\t0x%08x  0x%06x  0x%06x  0x%02x  %3s  %d  %d  %d\n", i, 
               arm_exe_h.s_h[i].sh_name, Type, arm_exe_h.s_h[i].sh_addr, arm_exe_h.s_h[i].sh_offset, 
               arm_exe_h.s_h[i].sh_size, arm_exe_h.s_h[i].sh_entsize, Flag, arm_exe_h.s_h[i].sh_link, 
               arm_exe_h.s_h[i].sh_info, arm_exe_h.s_h[i].sh_addralign);
    }
}

 
void print_VM_Headers() // print all headers info (same as readelf inst)
{
    int i=0;
    int ph_ent=0;
    char Flag[10];

    printf("ELF HEADER:\n");
    printf("\n\tEntry Point address : \t\t0x%llx",exe_h.vm_eh->e_entry);
    printf("\n\tStart of program headers : \t%d (0x%x) bytes into file",exe_h.vm_eh->e_phoff, exe_h.vm_eh->e_phoff);
    printf("\n\tStart of section headers : \t%d (0x%x) bytes into file",exe_h.vm_eh->e_shoff, exe_h.vm_eh->e_shoff);
    printf("\n\tSize of this header: \t\t%d (bytes)",exe_h.vm_eh->e_ehsize);
    printf("\n\tNumber of program headers : \t%d",exe_h.vm_eh->e_phnum);
    printf("\n\tSize of program header : \t%d (bytes)",exe_h.vm_eh->e_phentsize);
    printf("\n\tNumber of section headers : \t%d",exe_h.vm_eh->e_shnum);
    printf("\n\tSize of section header : \t%d (bytes)",exe_h.vm_eh->e_shentsize);
    printf("\n\nProgram Headers:\n  Type\t\tOffset\t\tVirtAddr\tFilesiz\tFlg\tAlign\n");

    ph_ent=exe_h.vm_eh->e_phnum;
     
    for(i=0;i<ph_ent;i++)
    {
         switch(arm_exe_h.p_h[i].p_flags & 0b111)
        {
            case 0b001: strcpy(Flag,"  E");break;
            case 0b010: strcpy(Flag," W ");break;
            case 0b100: strcpy(Flag,"R  ");break;
            case 0b011: strcpy(Flag," WE");break;
            case 0b101: strcpy(Flag,"R E");break;
            case 0b110: strcpy(Flag,"RW ");break;
            case 0b111: strcpy(Flag,"RWE");break;
            default : strcpy(Flag,"   ");break;
        }

        printf("  %-8s\t0x%06x\t0x%08llx\t0x%05x\t%s\t0x%x\n",exe_h.vm_ph[i].p_name 
                 , exe_h.vm_ph[i].p_offset, exe_h.vm_ph[i].p_vaddr, 
                 exe_h.vm_ph[i].p_filesz, Flag,exe_h.vm_ph[i].p_align);
    } 
}

void get_Headers(unsigned long long *mmap_addr) // get headers from exe file
{
    int ph_ent=0;
    int sh_ent=0;

    arm_exe_h.e_h = malloc(sizeof(arm_Elf_Header));
    memcpy(arm_exe_h.e_h, mmap_addr, sizeof(arm_Elf_Header));
    
    ph_ent=arm_exe_h.e_h->e_phnum;
    arm_exe_h.p_h = malloc(sizeof(arm_Program_Header)*ph_ent);
    memcpy(arm_exe_h.p_h, (unsigned int*)mmap_addr+arm_exe_h.e_h->e_phoff/4, sizeof(arm_Program_Header)*ph_ent);

    sh_ent=arm_exe_h.e_h->e_shnum;
    arm_exe_h.s_h = (arm_Section_Header *)malloc(sizeof(arm_Section_Header)*sh_ent);
    memcpy(arm_exe_h.s_h, (unsigned int*)mmap_addr+arm_exe_h.e_h->e_shoff/4, sizeof(arm_Section_Header)*sh_ent);
}

void free_ARM_Headers()
{
    free(arm_exe_h.e_h);
    free(arm_exe_h.p_h);
    free(arm_exe_h.s_h);
}

void set_Headers(unsigned long long *mmap_addr)
{
    exe_h.vm_eh = malloc(sizeof(vm_Elf_Header));
    exe_h.vm_eh->e_phoff = arm_exe_h.e_h->e_phoff;
    exe_h.vm_eh->e_shoff = arm_exe_h.e_h->e_shoff;
    exe_h.vm_eh->e_ehsize = arm_exe_h.e_h->e_ehsize;
    exe_h.vm_eh->e_phnum = arm_exe_h.e_h->e_phnum;
    exe_h.vm_eh->e_phentsize = arm_exe_h.e_h->e_phentsize;
    exe_h.vm_eh->e_shnum = arm_exe_h.e_h->e_shnum;
    exe_h.vm_eh->e_shentsize = arm_exe_h.e_h->e_shentsize;
  
    int ph_ent=0;
    unsigned int i=0;
    unsigned int start_of_text;
    
    ph_ent = arm_exe_h.e_h->e_phentsize;
    exe_h.vm_ph = malloc(sizeof(vm_Program_Header)*ph_ent);
    
    for(i=0;i<ph_ent;i++)
    {
        
        switch(arm_exe_h.p_h[i].p_type)
        {
            case 0x1: 
                if(arm_exe_h.p_h[i].p_flags & 0b010) {data_idx=i;strcpy(exe_h.vm_ph[i].p_name,"DATA");break;}
                else {strcpy(exe_h.vm_ph[i].p_name,"TEXT");start_of_text= arm_exe_h.p_h[i].p_vaddr;break;}
            case 0x4: strcpy(exe_h.vm_ph[i].p_name,"NOTE");break;
            case 0x7: strcpy(exe_h.vm_ph[i].p_name,"TLS");break;
            case 0x70000001: strcpy(exe_h.vm_ph[i].p_name,"EXIDX");break;
            default: strcpy(exe_h.vm_ph[i].p_name,"GNU_STACK");break;
        }
        exe_h.vm_ph[i].p_offset = arm_exe_h.p_h[i].p_offset;
        exe_h.vm_ph[i].p_filesz = arm_exe_h.p_h[i].p_filesz;
        exe_h.vm_ph[i].p_memsz  = arm_exe_h.p_h[i].p_memsz;
        exe_h.vm_ph[i].p_flags = arm_exe_h.p_h[i].p_flags;
        exe_h.vm_ph[i].p_vaddr = (unsigned long long)((unsigned int*)mmap_addr+exe_h.vm_ph[i].p_offset/4);
        exe_h.vm_ph[i].p_align = arm_exe_h.p_h[i].p_align;
    }
    exe_h.vm_eh->e_entry = (unsigned long long)((unsigned int*)mmap_addr+(arm_exe_h.e_h->e_entry-start_of_text)/4);
/*    
    unsigned long long *tmp = exe_h.vm_eh->e_entry;
    printf("mmapaddr : %p\n",mmap_addr);
    printf("entry :    0x%llx\n",((unsigned int*)mmap_addr + 0xb94/4));
    printf("tmp :      0x%llx\n",tmp);
    for(i=0;i<10;i++)
    {
        printf("~~~~~~~~~~~%x~~~~~~~~\n",*((unsigned int *)mmap_addr+0xb94/4+i));
    }
    */
   free_ARM_Headers();
}

void get_Data_Info(int i)
{
    /* unsigned long long VADDR_DATA_SEG = exe_h.vm_ph[i].p_vaddr; */
    /* unsigned int FILESZ_DATA_SEG = exe_h.vm_ph[i].p_filesz; */
    /* unsigned int MEMSZ_DATA_SEG = exe_h.vm_ph[i].p_memsz; */
    /* unsigned int FILE_OFFSET_OF_DATA_SEG = exe_h.vm_ph[i].p_offset; */
}

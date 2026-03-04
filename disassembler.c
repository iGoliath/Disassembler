#include <stdio.h>
#include <stdlib.h>
#include <elf.h>
#include <string.h>

void printEhdrInfo(Elf64_Ehdr, char*[]);
void printPhdrInfo(Elf64_Phdr, char*[], char*[]);
void printShdrInfo(Elf64_Shdr);

int main() {

    FILE *file = fopen("add", "rb");

    Elf64_Ehdr header;

    char *EI_OSABIValues[] = {"No extensions or unspecified", "Hewlett-Packard HP-UX",
        "NetBSD", "GNU or Linux(historical-alias for ELFOSABI_GNU)", "None", "None", 
        "Sun Solaris", "AIX", "IRIX", "FreeBSD", "Compaq TRU64 UNIX", "Novell Modesto", 
        "Open BSD", "Open VMS", "Hewlett-Packard Non-Stop Kernel", "Amiga Research OS",
        "The FenixOS highly scalable multi-core OS", "Nuxi CloudABI", "Stratus Technologies OpenVOS"};

    char *p_flagValues[] = {"Error", "Execute", "Write", "Write/Execute", "Read", "Read/Execute", "Read/Write", "Read/Write/Execute"};

    char *p_typeValues[] = {"PT_NULL", "PT_LOAD", "PT_DYNAMIC", "PT_INTERP", "PT_NOTE", "PT_SHLIB", "PT_PHDR", "PT_TLS"};

    fread(&header, sizeof(Elf64_Ehdr), 1, file);

    printEhdrInfo(header, EI_OSABIValues);

    fseek(file, header.e_phoff, SEEK_SET);

    Elf64_Phdr phdr;

    for (int i = 0; i < header.e_phnum; i++) {
        fread(&phdr, sizeof(char), header.e_phentsize, file);
        printPhdrInfo(phdr, p_flagValues, p_typeValues);
    }

    fseek(file, header.e_shoff, SEEK_SET);

    Elf64_Shdr shdr;

    for (int i = 0; i < header.e_shnum; i++) {
        fread(&shdr, sizeof(char), header.e_shentsize, file);
        if (shdr.sh_type == 1 && shdr.sh_flags == 6) {
          printShdrInfo(shdr);
        }
       
    }

    long int stringTableHeaderOffset = header.e_shentsize * (header.e_shstrndx - 1) + header.e_shoff;

    fseek(file, stringTableHeaderOffset, SEEK_SET);
    fread(&shdr, sizeof(char), header.e_shentsize, file);
    printShdrInfo(shdr);

    fseek(file, shdr.sh_offset, SEEK_SET);

    char *string = malloc(shdr.sh_size * sizeof(char));

    fread(string, sizeof(char), shdr.sh_size, file);
    fwrite(string, sizeof(char), shdr.sh_size, stdout);
    printf("\n\n");

    free(string);

    fseek(file, 4476, SEEK_SET); 

    unsigned char *byteArray = malloc(0x0D * sizeof(char));
    fread(byteArray, sizeof(char), 0x0D, file);
    fclose(file);
    for (int i = 0; i < 0x0D; i++) {
        printf("%02X ", *(byteArray + i));
    }
    printf("\n\n");
    free(byteArray);


    return 0;
}

void printEhdrInfo(Elf64_Ehdr header, char* EI_OSABIValues[]) {

    printf("\n\n=====E_Ident Info=====\n\n");

    printf("Magic number: %02X%c%c%c\n", header.e_ident[0],header.e_ident[1], header.e_ident[2], header.e_ident[3]);

    if (header.e_ident[4] == 2) {
        printf("Architecture: %02X, [64-bit]\n", header.e_ident[4]);
    }
    else if (header.e_ident[4] == 1) {
        printf("Architecture: %02X, [32-bit]\n", header.e_ident[4]);
    }
    else{
        printf("Architecture: %02X, [Unknown]\n", header.e_ident[4]);
    }

    if (header.e_ident[5] == 2) {
        printf("Data Encoding: %02X, [MSB(Big Endian)]\n", header.e_ident[5]);
    }
    else if (header.e_ident[5] == 1) {
        printf("Data Encoding: %02X, [LSB(Little Endian)]\n", header.e_ident[5]);
    }
    else {
        printf("Data Encoding: %02X [Invalid]\n", header.e_ident[5]);
    }

    if (header.e_ident[6] == 1) {
        printf("File Version: %02X, [Current]\n", header.e_ident[6]);
    }
    else {
        printf("File Version: %02X, [Invalid]\n", header.e_ident[6]);
    }

    if (header.e_ident[7] < 19) {
        printf("OS_ABI Value: %02X, [%s]\n", header.e_ident[7], EI_OSABIValues[header.e_ident[7]]);
    }
    else {
        printf("OS_ABI Value: %02X, [Invalid/Reserved]\n", header.e_ident[7]);
    }

    printf("\n\n=====Additional Fields=====\n\n");

    char e_type[10];

    switch (header.e_type) {
        case 0:
            strcpy(e_type,"ET_NONE");
            break;
        case 1:
            strcpy(e_type,"ET_REL");
            break;
        case 2:
            strcpy(e_type,"ET_EXEC");
            break;
        case 3:
            strcpy(e_type,"ET_DYN");
            break;
        case 4:
            strcpy(e_type,"ET_CORE");
            break;
        case 0xFE00:
            strcpy(e_type,"ET_LOOS");
            break;
        case 0xFEFF:
            strcpy(e_type,"ET_HIOS");
            break;
        case 0xFF00:
            strcpy(e_type,"ET_LOPROC");
            break;
        case 0xFFFF:
            strcpy(e_type,"ET_HIPROC");
            break;
        default:
            strcpy(e_type,"ERROR");
            break;
    }

    printf("e_type: %04X [%s]\n", header.e_type, e_type);

    printf("e_machine: %04X [Reference Appendix]\n", header.e_machine);

    if (header.e_version == 1) {
        printf("e_version: %04X [Current]\n", header.e_version);
    }
    else {
        printf("e_version: %04X [Invalid]\n", header.e_version);
    }

    printf("e_entry: %08lX\n", header.e_entry);

    printf("e_phoff: %08lX\n", header.e_phoff);

    printf("e_shoff: %08lX\n", header.e_shoff);

    printf("e_flags: %04X\n", header.e_flags);

    printf("e_ehsize: %d Bytes\n", header.e_ehsize);

    printf("e_phentsize: %d Bytes\n", header.e_phentsize);

    printf("e_phnum: %d Entries\n", header.e_phnum);

    printf("e_shentsize: %d Bytes\n", header.e_shentsize);

    printf("e_shnum: %d Entries\n", header.e_shnum);

    printf("e_shstrndx: String table is the %dth entry in Section Header\n\n\n", header.e_shstrndx);

    
    return;
}

void printPhdrInfo(Elf64_Phdr phdr, char *p_flagValues[], char *p_typeValues[]) {

    printf("=====Phdr Info=====\n\n");

    if (phdr.p_type >= 0 && phdr.p_type <= 7) {
        printf("p_type: %08X [%s]\n", phdr.p_type, p_typeValues[phdr.p_type]);
    }
    else if (phdr.p_type >= 0x60000000 && phdr.p_type <= 0x6fffffff) {
        printf("p_type: %08X [OS-Specific]\n", phdr.p_type);
    }
    else if (phdr.p_type >= 0x70000000 && phdr.p_type <= 0x7fffffff) {
        printf("p_type: %08X [PROCESSOR-SPECIFIC]\n", phdr.p_type);
    }
    

    if (phdr.p_flags & 0x0ff00000) {
        printf("p_flags: %08X [PF_MASKOS/Unspecified]", phdr.p_flags);
    }
    if (phdr.p_flags & 0xf0000000) {
        printf(" [PF_MASKPROC/Unspecified]");
    }
    else {
        printf("p_flags: %08X [%s]", phdr.p_flags, p_flagValues[phdr.p_flags]);
    }

    printf("\np_offset: %016lX\n", phdr.p_offset);
    
    printf("p_vaddr: %016lX\n", phdr.p_vaddr);

    printf("p_paddr: %016lX\n", phdr.p_paddr);

    printf("p_filesz: %016lX\n", phdr.p_filesz);

    printf("p_memsz: %016lX\n", phdr.p_memsz);

    printf("p_align: %016lX\n\n\n", phdr.p_align);

    return;
}

void printShdrInfo(Elf64_Shdr shdr) {
    printf("Section Header Name: %d\n", shdr.sh_name);

    printf("Section Header Type: %d\n", shdr.sh_type);

    printf("Flags: %016lX\n", shdr.sh_flags);

    printf("Section Header Address: %016lX\n", shdr.sh_addr);

    printf("Section Offset: %016lX\n", shdr.sh_offset);

    printf("Section Size: %016lX\n", shdr.sh_size);

    printf("Section header table index link: %08X\n", shdr.sh_link);

    printf("Section info: %08X\n", shdr.sh_info);

    printf("Section alignment: %016lX\n", shdr.sh_addralign);

    printf("Section entry size (in bytes): %016lX\n\n", shdr.sh_entsize);

}




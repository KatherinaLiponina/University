#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>

////////////////////////////////////////////////////////
//  PE-file
////////////////////////////////////////////////////////
//  DOS_HEADER                  - not imprt            
//      e_lfnew                     - 0x3c
//  STUB                        - ???
//  PE\0\0 SIGNATURE            - 4
//  IMAGE_FILE_HEADER           - 20
//      NumberOfSection             - 3 word -> 6 byte
//  IMAGE_OPTIONAL_HEADER       - 224
//      SizeOfCode                  - 4
//      AddressOfEntryPoint         - 16
//      ImageBase                   - 28
//      FileAlignment               - 36
//      SizeOfHeaders               - 60
//  IMAGE_SECTION_HEADER * n    - 40 * n
//  Code
//  Other data 
///////////////////////////////////////////////////////
#define E_LFNEW_OFFSET 0x3c
#define SIGNATURE_SIZE 4
#define IMAGE_FILE_HEADER_SIZE 20
#define IMAGE_OPTIONAL_HEADER_SIZE (96 + 16 * 8)
#define IMAGE_SECTION_HEADER_SIZE 0x28

#define NUMBER_OF_SECTION_OFFSET 3
#define SIZE_OF_CODE_OFFSET 4
#define ADDRESS_OF_ENTRY_POINT_OFFSET 16
#define IMAGE_BASE_OFFSET 28
#define FILE_ALIGNMENT_OFFSET 36
#define SIZE_OF_HEADER_OFFSET 60

#define IMAGE_SIZEOF_SHORT_NAME 8

#define I2A(v) "0123456789abcdef"[v]
#define DUMP_CHAR(fd, c) dprintf(fd, "%c%c ", I2A((c>>4) & 0xF), I2A(c & 0XF))
#define DUMP_STR(fd, str, str_len)                      \
    do {                                                \
        int i;                                          \
        for(i=0; i < str_len; ++i)                      \
        {                                               \
            if (i % 8 == 0) dprintf(fd, "\n");          \
            DUMP_CHAR(fd, str[i]);                      \
        }                                               \
    } while(0)


int create_output_file(char *, const char *);

int main(int argc, char * argv[]) {
    if (argc <= 1) {
        printf("Using: ./pe_reader PE_FILE_NAME\n");
        return 0;
    } 
    char * pe_file_name = strdup(argv[1]);
    int fd = open(pe_file_name, O_RDONLY);
    if (fd == -1){
        perror("open");
        return 0;
    }
    size_t size;
    struct stat st;
    if ((fstat(fd, &st) != 0) || (!S_ISREG(st.st_mode))) {
			perror("fstat");
            return 0;
	}
    size = st.st_size;
    void * pe_ptr = mmap(NULL, size, PROT_READ, MAP_PRIVATE, fd, 0);

    char * byte_ptr = (char *)pe_ptr; //byte_ptr -> e_magic in IMAGE_DOS_HEADER
    if (*byte_ptr != 'M' || *(byte_ptr + 1) != 'Z') {
        printf("File hasn't \"MZ\" as first bytes\n");
        return 0;
    }

    byte_ptr +=  E_LFNEW_OFFSET; //byte_ptr -> e_lfnew in IMAGE_DOS_HEADER
    int * dword_ptr = (int *)byte_ptr;
    //printf("e_lfnew = %d\n", *dword_ptr);

    byte_ptr = (char *) pe_ptr + *dword_ptr; //byte_ptr -> pe_header
    if (*byte_ptr != 'P' || *(byte_ptr + 1) != 'E') {
        printf("File isn't PE-file\n");
        return 0;
    }



    int16_t NumberOfSections = 0;
    int16_t * word_ptr = (int16_t *)byte_ptr;
    word_ptr += NUMBER_OF_SECTION_OFFSET; //word_ptr -> NumberOfSections in _IMAGE_FILE_HEADER
    NumberOfSections = *word_ptr;

    int sec_fd = create_output_file(pe_file_name, "_info.txt");
    if (sec_fd == -1) {
        return 0;
    }

    dprintf(sec_fd, "Number of sections: %d\n", NumberOfSections);

    byte_ptr += SIGNATURE_SIZE + IMAGE_FILE_HEADER_SIZE; // byte_ptr -> _IMAGE_OPTIONAL_HEADER

    int ImageBase, AddressEP, SizeOfCode, SizeofHeader, FileAlignment;
    SizeOfCode = *((int *)(byte_ptr + SIZE_OF_CODE_OFFSET));
    ImageBase = *((int *)(byte_ptr + IMAGE_BASE_OFFSET)); // byte_ptr -> ImageBase
    AddressEP = *((int *)(byte_ptr + ADDRESS_OF_ENTRY_POINT_OFFSET)); // byte_ptr -> AddressOfEntryPoint
    SizeofHeader = *((int *)(byte_ptr + SIZE_OF_HEADER_OFFSET));
    FileAlignment = *((int *)(byte_ptr + FILE_ALIGNMENT_OFFSET));
    dprintf(sec_fd, "VA of entry point: %x\n", ImageBase + AddressEP);

    byte_ptr += IMAGE_OPTIONAL_HEADER_SIZE; //byte_ptr -> _IMAGE_SECTION_HEADER
    for (int j = 0; j < NumberOfSections; j++){
        dprintf(sec_fd, "Section Name: ");
        for (int i = 0; i < IMAGE_SIZEOF_SHORT_NAME; i++) {
            dprintf(sec_fd, "%c", *(byte_ptr + i));
        }
        dprintf(sec_fd, "\n");
        dword_ptr = (int *)(byte_ptr + 16);
        dprintf(sec_fd, "Size of section: %x\n", *dword_ptr);
        dword_ptr = (int *)(byte_ptr + 36);
        //printf("Characteristics: %x\n", *dword_ptr);
        if (*dword_ptr & 0x20) {
            dprintf(sec_fd, "The section contains executable code.\n");
        }
        else if (*dword_ptr & 0x40) {
            dprintf(sec_fd, "The section contains initialized data.\n");
        }
        else if (*dword_ptr & 0x80) {
            dprintf(sec_fd, "The section contains uninitialized data.\n");
        }
        if (*dword_ptr & 0x10000000) {
            dprintf(sec_fd, "The section can be shared in memory.\n");
        }
        if (*dword_ptr & 0x20000000) {
            dprintf(sec_fd, "The section can be executed as code.\n");
        }
        if (*dword_ptr & 0x40000000) {
            dprintf(sec_fd, "The section can be read.\n");
        }
        if (*dword_ptr & 0x80000000) {
            dprintf(sec_fd, "The section can be written to.\n");
        }
        dprintf(sec_fd, "\n");
        byte_ptr += IMAGE_SECTION_HEADER_SIZE;
    }

    byte_ptr = (char *)pe_ptr;
    byte_ptr += SizeofHeader; //byte_ptr -> code
    int code_fd = create_output_file(pe_file_name, "_code.txt");
    if (code_fd == -1) {
        return 0;
    }
    DUMP_STR(code_fd, byte_ptr, SizeOfCode);

    munmap(pe_ptr, size);
    free(pe_file_name);
    close(code_fd);
    close(sec_fd);
    return 0;
}

int create_output_file(char * path, const char * end) {
    int plen = strlen(path), elen = strlen(end);
    int point = -1;
    for (int i = 0; i < plen; i++) {
        if (path[i] == '.') {
            point = i;
            break;
        }
    }
    if (point != -1) {
        plen = point;
    }
    char * name = (char *)malloc(sizeof(char) * (plen + elen + 1));
    for (int i = 0; i < plen; i++) {
        name[i] = path[i];
    }
    for (int i = plen; i < plen + elen + 1; i++) {
        name[i] = end[i - plen];
    }
    int fd = open(name, O_RDWR | O_CREAT | O_TRUNC, S_IRWXU | S_IRWXO | S_IRWXG);
    if (fd == -1) {
        perror("open");
        return -1;
    }
    return fd;
}

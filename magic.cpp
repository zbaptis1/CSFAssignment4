#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cstdarg>
#include <cerrno>
#include <cstdint>
#include <elf.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/unistd.h>
#include <fcntl.h>

#include <iostream>
#include <fstream>
#include <iterator>
#include <string>
#include <vector>

#include "elf_names.h"

using std::ifstream;
using std::vector;
using std::cout;
using std::endl;
using std::string;

int main(int argc, char **argv) {
    // TODO: implement
    ifstream FILE(argv[1], std::ios::in | std::ios::binary);
    char isELF[17];
    FILE.read(isELF, 17);
    if (!(isELF[0] == 127 && isELF[1] == 'E' && isELF[2] == 'L' && isELF[3] == 'F')) {
        cout << "Not an ELF file" << endl; 
        FILE.close();
        return 0;
    }
    // For ELF files

    /** TODO: need to see if this works, use some of the test files */


    /** TODO: data is a pointer to the beginning of the file */
    unsigned char * data;
    Elf64_Ehdr *elf_header = (Elf64_Ehdr *) data;    
    for (int i = 0; i < 17; i++) { 
        elf_header->e_ident[i] = isELF[i]; 
        cout << "HEADER: " << elf_header->e_ident[i];
        cout << "\tISELF: " << isELF[i] << endl;
    }
    // focus on working on 64-bit ELF files first, as 32-bit isn't worth the amount of work for the grade
    elf_header->e_ident[4] = '2'; 


    /* Output */
    const char * objtype = get_type_name(elf_header->e_type);
    const char * machtype = get_machine_name(elf_header->e_machine);

    // Summary of ELF file
    cout << "Object file type: " << objtype << endl;
    cout << "Instruction set: " << machtype << endl;
    cout << "Endianness: ";
    if (elf_header->e_ident[5] == 1) { cout << "Little endian" << endl; }
    else { cout << "Big endian" << endl; }

    // Section info
    /* in the following format:
    Section header N: name=name, type=X offset=Y, size=Z
    N is a section index in the range 0 to e_shnum-1. 
    name is the section name, which will be a NUL-terminated string value in the .shstrtab section data. 
    X, Y, Z are the values of the section header’s sh_type, sh_offset, and sh_size values, respectively. 
    Each of these values should be printed using the %lx conversion using printf. 
    Note that the name may be an empty string. */
    
    for (uint16_t i = 0; i < elf_header->e_shnum; i++) {
        /** TODO: Figure out how to do name, X, Y, and Z; go to TA's */
        unsigned long int X, Y, Z;
        cout << "Symbol " << i << ": name=" << get_type_name(i) << ", ";
        printf("type=%lx offset=%lx, size=%lx", X, Y, Z);
    }

    // Symbol info
    /* in the following format:
    Symbol N: name=name, size=X, info=Y, other=Z
    N is the index of the symbol (0 for first symbol), 
    name is the name of the symbol based on the value of the symbol’s st_name value 
        (if non-zero, it specifies an offset in the .strtab section.) 
    X, Y, Z are the values of the symbol’s st_size, st_info, and st_other fields, respectively, 
        printed using printf with the %lx conversion. */

        // when will the loop end?
    for (uint16_t i = 0; i < elf_header->e_shnum; i++) {
        /** TODO: Figure out how to do name X Y and Z; go to TA's */
        unsigned long int X, Y, Z;

        cout << "Symbol " << i << ": name=" << get_type_name(i) << ", ";
        printf("size=%lx, info=%lx, other=%lx", X, Y, Z);
    }




}



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
#include <vector>

#include "elf_names.h"
#include "magic.h"

using std::ifstream;
using std::vector;
using std::cout;
using std::endl;

int main(int argc, char **argv) {
    // TODO: implemen
    ifstream FILE(argv[1], std::ios::in | std::ios::binary);
    char isELF[17];
    FILE.read(isELF, 17);
    if (!(isELF[0] == 127 && isELF[1] == 'E' && isELF[2] == 'L' && isELF[3] == 'F')) {
        cout << "Not an ELF file" << endl; 
        FILE.close();
        return 0;
    }
    // For ELF files

    //TODO: need to see if this works, use some of the test files
    ELFHeader header;
    for (int i = 0; i < 17; i++) { 
        header.e_ident[i] = isELF[i]; 
        cout << "HEADER: " << header.e_ident[i];
        cout << "\tISELF: " << isELF[i] << endl;
    }

}



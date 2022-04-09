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

using std::ifstream;
using std::vector;
using std::cout;
using std::endl;

int main(int argc, char **argv) {
    // TODO: implemen
    ifstream FILE(argv[1], std::ios::in | std::ios::binary);
    char isELF[4];
    FILE.read(bytes, 4);
    if (!(isELF[1] == 'E' && isELF[2] == 'L' && isELF[3] == 'F')) {
        cout << "Not an ELF file" << endl; 
        FILE.close();
        exit();
    }
    // For ELF files

}



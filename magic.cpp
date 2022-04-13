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


// plan to move to another file
// bring back magic.h ??
struct SectionInfo {
    bool isValid;
    const char * name;
    unsigned type, offset, size, entsize;

    SectionInfo() : isValid(false), name(""), type(0), offset(0), size(0), entsize(0) { }
};

struct ELFFile {
    int fileDescription;
    size_t fileSize;
    unsigned char * data;
    unsigned offset, shnum, shentsize;
    unsigned shstrndx;
    unsigned strtabIndex; 
    unsigned symtabIndex;
    SectionInfo * sectionInfo;

    ELFFile();
    ~ELFile(); // giving compiler warnings need to fix later

    int mapFile(const char * filename);
    void unmapFile();

    unsigned char * getData(unsigned offset, unsigned size);
    int isELF();

    Elf64_Ehdr * getELF();

    const char * getTypeName();
    const char * getMachineName();

    void printSummary();
    void printSections();
    void printSymbols();

    const char * findString(unsigned symbolIndex, unsigned symbolOffset);
};


/** TODO: Change up main from TA's help */
int main(int argc, char **argv) {
    if (argc != 2) { // Error check
        cout << "Invalid arguments" << endl;
        return 1;
    }

    const char * filename = argv[1];

    ELFFile * elf = new ELFFile;
    elf->mapFile(filename);

    if (elf->isELF() == 1) { 
        cout << "Not an ELF file" << endl; 
        return 1;
    }

    /* Output */
    // Summary
    elf->printSummary();
    // Section info
    elf->printSections();
    // Symbol info
    elf->printSymbols();


    elf->unmapFile();
}

/* Struct Header */
ELFFile::ELFFile()
    : fileDescription(-1)
    , fileSize(0)
    , data(nullptr)
    , offset(0)
    , shnum(0)
    , shentsize(0)
    , shstrndx(0)
    , strtabIndex(0)
    , symtabIndex(0)
   , sectionInfo(nullptr) {
}

ELFFile::~ELFFile() { delete sectionInfo; }

int ELFFile::mapFile(const char * filename) {
    int stat;
    struct stat statbuf;

    fileDescription = open(filename, O_RDONLY);
    if (fileDescription < 0) { return 0; }

    stat = fstat(fileDescription, &statbuf);
    if (stat != 0) {
        close(fileDescription);
        return 0;
    }

    fileSize = (size_t) statbuf.st_size;

    data = static_cast<unsigned char *> (mmap(nullptr, fileSize, PROT_READ, MAP_PRIVATE, fileDescription, 0));
    if (data == nullptr) {
        close(fileDescription);
        return 0;
    }

    return 1;
}

void ELFFile::unmapFile() {
    munmap(data, fileSize);
    close(fileDescription);
}

unsigned char * ELFFile::getData(unsigned offset, unsigned size) { 
    if (offset > size) { return nullptr; } // logically cant happen
    if ((offset + size) > size) { return nullptr; } // goes beyond EOF
    if ((offset + size) < offset) { return nullptr; } // overflow

    return data + offset; 
}
 
int ELFFile::isELF() {
    unsigned char magicNumber[4] = {0x7F, 'E', 'L', 'F'};
    return memcmp(data, magicNumber, 4) == 0;
}

Elf64_Ehdr * ELFFile::getELF() { return reinterpret_cast<Elf64_Ehdr * > (data); }

/** Printing Functions */
const char * ELFFile::getTypeName() {
    Elf64_Ehdr * file = getELF();
    Elf64_Half objtype = file->e_type;
    return get_type_name(objtype);
}

const char * ELFFile::getMachineName() {
    Elf64_Ehdr * file = getELF();
    Elf64_Half machtype = file->e_machine;
    return get_type_name(machtype);
}


void ELFFile::printSummary() {
    cout << "Object file type: " << getTypeName() << endl;
    cout << "Instruction set: " << getMachineName() << endl;
    cout << "Endianness: " << "Little Endiann" << endl;
 }

void ELFFile::printSections() { 
    for (unsigned i = 0; i < shnum; i++) {
        struct SectionInfo * curr = &sectionInfo[i];
        if (!(curr->isValid)) { printf("Section header %u: Invalid Section\n", i ); }
        else {
            printf("Section header %u: name=%s, type=%lx, offset=%lx, size=%lx\n", 
            i, curr->name, uint64_t(curr->type), uint64_t(curr->offset), uint64_t(curr->size));
        }
    }
}

void ELFFile::printSymbols() {
    struct SectionInfo * symbolInfo = &sectionInfo[symtabIndex];
    if (strcmp(".symtab", symbolInfo->name) != 0) { return; }

    unsigned symbolOffset = symbolInfo->offset;
    unsigned end = symbolOffset + symbolInfo->size;
    unsigned i = 0;

    while (symbolOffset < end) {
        // get current symbol
        unsigned char * curr = getData(symbolOffset, symbolInfo->size);

        if (curr != nullptr) {
            Elf64_Sym * elfSymbol = reinterpret_cast<Elf64_Sym *>(curr);
            unsigned st_name = elfSymbol->st_name;
            // match string to the ones in elf_names.cpp file
            const char * name = findString(strtabIndex, st_name);
            printf("Symbol %u: name=%s, size=%lx, info=%lx, other=%lx\n",
                    i, name, elfSymbol->st_size, uint64_t(elfSymbol->st_info), uint64_t(elfSymbol->st_other));
        }

        symbolOffset += symbolInfo->size;
        i++;
    }
}

const char * ELFFile::findString(unsigned symbolIndex, unsigned symbolOffset) {
    const SectionInfo &info = sectionInfo[symbolIndex]; 
    unsigned offset = info.offset;
    unsigned index = offset + symbolOffset;

    if (index < offset) { return nullptr; }
    
    unsigned i = index;
    while (i < fileSize) {
        unsigned char * curr = data + i;
        if (*curr == '\0') {
            return reinterpret_cast<const char *>(data + index);
        }
        i++;
    }
    return "";
}









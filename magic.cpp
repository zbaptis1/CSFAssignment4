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

#include "elf_names.h"
#include "magic.h"

using std::ifstream;
using std::cout;
using std::endl;
using std::cerr;


int main(int argc, char **argv) {    

    if (argc != 2) {
        cerr << "Invalid arguments" << endl;
        return 1;
    }

    const char * filename = argv[1];

    struct ELFFile elf;
    
    if(!elf.mapFile(filename)) { 
        cerr << "Couldn't map" << endl;
        return 2;
    }

    if (!(elf.isELF())) { 
        cout << "Not an ELF file" << endl; 
        return 0;
    }

    /* Output */

    elf.printSummary();

    // Section & Symbol info
    if (!elf.scanSections()) {
        printf("All invalid section headers");
    } else {
        elf.printSections();
        elf.printSymbols();
    }

    elf.unmapFile();
    return 0; 
}


/* Struct Header */
ELFFile::ELFFile()
    : fd(-1)
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
    struct stat statbuf;

    int fd = open(filename, O_RDONLY);
    if (fd < 0) {
        return 0;
     }

    int rc = fstat(fd, &statbuf);
    if (rc != 0) {
        close(fd);
        return 0;
    }

    fileSize = (size_t) statbuf.st_size;

    data = static_cast<unsigned char *> (mmap(nullptr, fileSize, PROT_READ, MAP_PRIVATE, fd, 0));
    if (data == nullptr) {
        close(fd);
        return 0;
    }

    return 1;
}

void ELFFile::unmapFile() {
    munmap(data, fileSize);
    close(fd);
}

unsigned char * ELFFile::getData(unsigned off, unsigned size) { 
    if (off > fileSize) { return nullptr; }
    if ((off + size) > fileSize) { return nullptr; }
    if ((off + size) < off) { return nullptr; }

    return data + off; 
}
 
int ELFFile::isELF() {
    if (fileSize < 56) { // not enough space
        return 0;
    }

    unsigned char magicNumber[4] = {0x7F, 'E', 'L', 'F'};
    return memcmp(data, magicNumber, 4) == 0;
}

Elf64_Ehdr * ELFFile::getELF() { 
    return reinterpret_cast<Elf64_Ehdr * > (data); 
}

/** Printing Functions */
const char * ELFFile::getTypeName() {
    Elf64_Ehdr * file = getELF();
    Elf64_Half objtype = file->e_type;
    return get_type_name(objtype);
}

const char * ELFFile::getMachineName() {
    Elf64_Ehdr * file = getELF();
    Elf64_Half machtype = file->e_machine;
    return get_machine_name(machtype);
}

int ELFFile::findSectionHeader() {
    Elf64_Ehdr * header = getELF();
    if (header != nullptr) {
        offset = header->e_shoff;
        shnum = header->e_shnum;
        shentsize = header->e_shentsize;
        shstrndx = header->e_shstrndx;
        return 1;
    } else {
        return 0;
    }
}

int ELFFile::scanSections() {
    if (!findSectionHeader()) { return 0; }

    sectionInfo = new SectionInfo[shnum];

    for (unsigned i = 0; i < shnum; i++) { 
        unsigned char * header = getData(offset + (i * shentsize), shentsize);
   
        Elf64_Shdr * curr = reinterpret_cast<Elf64_Shdr * >(header);
        
        sectionInfo[i].isValid = true;
        sectionInfo[i].secType = (unsigned) curr->sh_type;
        sectionInfo[i].secOffset = (unsigned) curr->sh_offset;
        sectionInfo[i].secSize = (unsigned) curr->sh_size;
        sectionInfo[i].secEntsize = (unsigned) curr->sh_entsize;
    }

    if (shstrndx >= shnum) { return 0; }

    for (unsigned i = 0; i < shnum; i++) {
        unsigned char * header = getData(offset + (i * shentsize), shentsize);

            Elf64_Shdr * curr = reinterpret_cast<Elf64_Shdr *>(header);
            sectionInfo[i].name = findString(shstrndx, curr->sh_name);

            if (strcmp(".strtab", sectionInfo[i].name) == 0) { strtabIndex = i; }
            if (strcmp(".symtab", sectionInfo[i].name) == 0) { symtabIndex = i; }
        
    }

    return 1;
}


void ELFFile::printSummary() {
    printf("Object file type: %s\n", getTypeName());
    printf("Instruction set: %s\n", getMachineName());
    printf("Endianness: Little endian\n");
 }

void ELFFile::printSections() { 
    for (unsigned i = 0; i < shnum; i++) {
        struct SectionInfo * curr = &sectionInfo[i];
        if (!(curr->isValid)) { printf("Section header %u: Invalid Section\n", i ); }
        else {
            printf("Section header %u: name=%s, type=%lx, offset=%lx, size=%lx\n", 
            i, curr->name, uint64_t(curr->secType), uint64_t(curr->secOffset), uint64_t(curr->secSize));
        }
    }
}

void ELFFile::printSymbols() {
    struct SectionInfo * symbolInfo = &sectionInfo[symtabIndex];

    if (strcmp(".symtab", symbolInfo->name) != 0) { return; }

    unsigned symbolOffset = symbolInfo->secOffset;
    unsigned end = symbolOffset + symbolInfo->secSize;
    unsigned i = 0;

    while (symbolOffset < end) {
        unsigned char * curr = getData(symbolOffset, symbolInfo->secEntsize);

        if (curr != nullptr) {
            Elf64_Sym * elfSymbol = reinterpret_cast<Elf64_Sym *>(curr);
            unsigned st_name = elfSymbol->st_name;
            const char * name;
            if (st_name != 0) {
                name = findString(strtabIndex, st_name);
            } else {
                name = "";
            }
            printf("Symbol %u: name=%s, size=%lx, info=%lx, other=%lx\n",
                    i, name, elfSymbol->st_size, uint64_t(elfSymbol->st_info), uint64_t(elfSymbol->st_other));
        }
        symbolOffset += symbolInfo->secEntsize;
        i++;
    }
}

const char * ELFFile::findString(unsigned symbolIndex, unsigned symbolOffset) {
    const SectionInfo &info = sectionInfo[symbolIndex]; 
    unsigned off = info.secOffset;
    unsigned index = off + symbolOffset;

    if (index < off) { return nullptr; }
    
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
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

using std::ifstream;
using std::cout;
using std::endl;
using std::cerr;


struct SectionInfo { // Struct for the Sections within the ELF File
    int isValid;
    const char * name;
    unsigned secType;
    unsigned secOffset;
    unsigned secSize;
    unsigned secEntsize;

    SectionInfo() : 
    isValid(0), 
    name(""), 
    secType(0), 
    secOffset(0), 
    secSize(0), 
    secEntsize(0) { }
};


// helper methods
unsigned char * ELFData(unsigned offset, unsigned size, size_t fileSize, unsigned char * data) {
    if (offset > fileSize) { return nullptr; }
    if ((offset + size) > fileSize) { return nullptr; }
    if ((offset + size) < offset) { return nullptr; }

    return data + offset; 
}
const char * findName(unsigned symbolOffset, unsigned symbolIndex, SectionInfo * sectionInfo, unsigned fileSize, unsigned char * data);


int main(int argc, char **argv) {  
    
    if (argc != 2) {
        cerr << "Invalid arguments" << endl;
        return 1;
    }

    const char * filename = argv[1];

    int fd = open(filename, O_RDONLY);
    if (fd < 0) { return 0; }

    struct stat statbuf;
    int rc = fstat(fd, &statbuf);
    if (rc != 0) {
        cerr << "Couldn't map" << endl;
        close(fd);
        return 2;
    }

    size_t fileSize = (size_t) statbuf.st_size;

    unsigned char * data = static_cast<unsigned char *> (mmap(nullptr, fileSize, PROT_READ, MAP_PRIVATE, fd, 0));
    if (data == nullptr) {
        cerr << "Couldn't map" << endl;
        close(fd);
        return 2;
    }

    unsigned char magicNumber[4] = {0x7F, 'E', 'L', 'F'};
    if (memcmp(data, magicNumber, 4) != 0) {
        cout << "Not an ELF file" << endl; 
        return 0;
    }

    Elf64_Ehdr * file = reinterpret_cast<Elf64_Ehdr * > (data);
    Elf64_Half objtype = file->e_type;
    Elf64_Half machtype = file->e_machine;

    printf("Object file type: %s\n", get_type_name(objtype));
    printf("Instruction set: %s\n", get_machine_name(machtype));
    printf("Endianness: Little endian\n");

    int allInvalidSections = 0; 

    Elf64_Ehdr * sectionHeader = reinterpret_cast<Elf64_Ehdr * > (data);
    if (sectionHeader == nullptr) { 
        allInvalidSections = 1; 
        return 0; 
    }
    Elf64_Ehdr * header = reinterpret_cast<Elf64_Ehdr * > (data); 
    unsigned offset = header->e_shoff;
    unsigned shnum = header->e_shnum;
    unsigned shentsize = header->e_shentsize;
    unsigned shstrndx = header->e_shstrndx;
    unsigned strtabIndex, symtabIndex;


    SectionInfo * sectionInfo = new SectionInfo[shnum];
    while (!allInvalidSections) {
        unsigned i = 0; 
        while (i < shnum) {
            unsigned char * header = ELFData(offset + (i * shentsize), shentsize, fileSize, data);
            Elf64_Shdr * curr = reinterpret_cast<Elf64_Shdr * >(header);
            
            sectionInfo[i].isValid = 1;
            sectionInfo[i].secType = (unsigned) curr->sh_type;
            sectionInfo[i].secOffset = (unsigned) curr->sh_offset;
            sectionInfo[i].secSize = (unsigned) curr->sh_size;
            sectionInfo[i].secEntsize = (unsigned) curr->sh_entsize;
            i++;
        }

        if (shstrndx >= shnum) { 
            allInvalidSections = true;
            break; 
        }

        i = 0;
        
        while (i < shnum) {
            unsigned char * header = ELFData(offset + (i * shentsize), shentsize, fileSize, data);

            Elf64_Shdr * curr = reinterpret_cast<Elf64_Shdr *>(header);
            sectionInfo[i].name = findName(curr->sh_name, shstrndx, sectionInfo, fileSize, data);

            if (strcmp(".symtab", sectionInfo[i].name) == 0) { symtabIndex = i; }
            if (strcmp(".strtab", sectionInfo[i].name) == 0) { strtabIndex = i; }

            i++;
        }
    }

    if(allInvalidSections) {
        printf("All invalid section headers");
    } else {
        // sections
        unsigned i = 0;
        while (i < shnum) {
            struct SectionInfo * curr = &sectionInfo[i];
            if ((curr->isValid)) { 
                printf("Section header %u: name=%s, type=%lx, offset=%lx, size=%lx\n", 
                i, curr->name, uint64_t(curr->secType), uint64_t(curr->secOffset), uint64_t(curr->secSize));
            } else {
                printf("Section header %u: Invalid Section\n", i );
            }
            i++;
        }

        // symbols
        struct SectionInfo * symbolInfo = &sectionInfo[symtabIndex];
        if (strcmp(".symtab", symbolInfo->name) != 0) { return 0; }

        unsigned symbolOffset = symbolInfo->secOffset;
        unsigned end = symbolOffset + symbolInfo->secSize;
        unsigned index = 0;

        while (symbolOffset < end) {
            unsigned char * curr = ELFData(symbolOffset, symbolInfo->secEntsize, fileSize, data);

            if (curr != nullptr) {
                Elf64_Sym * elfSymbol = reinterpret_cast<Elf64_Sym *>(curr);
                unsigned st_name = elfSymbol->st_name;
                const char * name;

                if (st_name != 0) {
                    name = findName(st_name, strtabIndex, symbolInfo, fileSize, data);
                } else {
                    name = "";
                }

                printf("Symbol %u: name=%s, size=%lx, info=%lx, other=%lx\n",
                        index, name, elfSymbol->st_size, uint64_t(elfSymbol->st_info), uint64_t(elfSymbol->st_other));
            }
            symbolOffset += symbolInfo->secEntsize;
            index++;
        }
    }

    munmap(data, fileSize);
    close(fd);
    return 0; 
}
/*
unsigned char * ELFdata(unsigned offset, unsigned size, size_t fileSize, unsigned char * data) { 
    if (offset > fileSize) { return nullptr; }
    if ((offset + size) > fileSize) { return nullptr; }
    if ((offset + size) < offset) { return nullptr; }

    return data + offset; 
}*/
 
const char * findName(unsigned symbolOffset, unsigned symbolIndex, SectionInfo * sectionInfo, unsigned fileSize, unsigned char * data) {
    const SectionInfo &info = sectionInfo[symbolIndex]; 
    unsigned index = info.secOffset + symbolOffset;

    if (index < info.secOffset) { return nullptr; }
    
    for (unsigned i = index; i < fileSize; i++) {
        unsigned char * curr = data + i;
        if (*curr == '\0') { return reinterpret_cast<const char *>(data + index); }
    }
    return "";
}
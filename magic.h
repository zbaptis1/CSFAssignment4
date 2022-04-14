#ifndef MAGIC_H
#define MAGIC_H

#ifdef __cplusplus
#include <cstdint>
extern "C" {
#else
#include <stdint.h>
#endif

struct SectionInfo { // Struct for the Sections within the ELF File
    int isValid;
    const char * name;
    unsigned secType, secOffset, secSize, secEntsize;

    SectionInfo() : isValid(0), name(""), secType(0), secOffset(0), secSize(0), secEntsize(0) { }
};

struct ELFFile { // Struct for the overall ELF file and helper methods
    int fd;
    size_t fileSize;
    unsigned char * data;
    unsigned offset, shnum, shentsize;
    unsigned shstrndx;
    unsigned strtabIndex; 
    unsigned symtabIndex;
    SectionInfo * sectionInfo;

    ELFFile();
    ~ELFFile();

    int mapFile(const char * filename);
    void unmapFile();

    unsigned char * getData(unsigned off, unsigned size);
    int isELF();

    Elf64_Ehdr * getELF();

    const char * getTypeName();
    const char * getMachineName();

    int findSectionHeader();
    int scanSections();
    void printSummary();
    void printSections();
    void printSymbols();

    const char * findString(unsigned symbolIndex, unsigned symbolOffset);
};


#ifdef __cplusplus
}
#endif

#endif // MAGIC_H

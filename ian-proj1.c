/**
 * @name 1. projekt - práce s ELF soubory
 * @author Kristián Kováč <xkovac61>
 * @date: 27/03/2022
 **/

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <fcntl.h>
#include <libelf.h>
#include <gelf.h>
#include <err.h>
#include <errno.h>
#include <string.h>

#define CASE(var)              \
	case PT_##var:             \
		printf("%-12s", #var); \
		break

void printSegmentType(const GElf_Phdr *phdr)
{
	switch (phdr->p_type)
	{
		CASE(NULL);
		CASE(LOAD);
		CASE(DYNAMIC);
		CASE(INTERP);
		CASE(NOTE);
		CASE(SHLIB);
		CASE(PHDR);

	default:
		printf("0x%-12x", phdr->p_type);
	}
}

#undef CASE

void printPermissions(const GElf_Phdr *phdr)
{
	printf("%c", (phdr->p_flags & PF_R) ? 'R' : '-');
	printf("%c", (phdr->p_flags & PF_W) ? 'W' : '-');
	printf("%c", (phdr->p_flags & PF_X) ? 'X' : '-');
}

bool isSectionInSegment(const GElf_Shdr *section, const GElf_Phdr *segment)
{
	Elf64_Xword segmentSize = (section->sh_type == SHT_NOBITS)
								  ? segment->p_memsz
								  : segment->p_filesz;

	return (section->sh_offset >= segment->p_offset &&
			section->sh_offset + section->sh_size <= segment->p_offset + segmentSize);
}

void printSegmentSections(Elf *elf, const GElf_Phdr *phdr)
{
	Elf_Scn *scn = NULL;
	GElf_Shdr shdr;
	size_t shstrndx;
	if (elf_getshdrstrndx(elf, &shstrndx) != 0)
	{
		errx(EXIT_FAILURE, "elf_getshdrstrndx() failed: %s.", elf_errmsg(-1));
	}

	while ((scn = elf_nextscn(elf, scn)) != NULL)
	{
		gelf_getshdr(scn, &shdr);
		char *name = elf_strptr(elf, shstrndx, shdr.sh_name);

		if (isSectionInSegment(&shdr, phdr))
			printf("%s ", name);
	}
}

void printSegmentInfo(Elf *elf, int id, const GElf_Phdr *phdr)
{
	printf("%02d", id);
	printf("\t");
	printSegmentType(phdr);
	printf("\t");
	printPermissions(phdr);
	printf("\t");
	printSegmentSections(elf, phdr);
	printf("\n");
}

void main(int argc, char **argv)
{
	if (argc != 2)
	{
		errx(EXIT_FAILURE, "Invalid arguments.");
	}
	int fd = open(argv[1], O_RDONLY);

	if (fd < 0)
	{
		errx(EXIT_FAILURE, "File error - %s.", strerror(errno));
	}

	if (elf_version(EV_CURRENT) == EV_NONE)
	{
		errx(EXIT_FAILURE, "Invalid version.");
	}

	Elf *elf = NULL;
	if ((elf = elf_begin(fd, ELF_C_READ, NULL)) == NULL)
	{
		errx(EXIT_FAILURE, "elf_begin() failed: %s.", elf_errmsg(-1));
	}

	size_t phdrCount;
	if (elf_getphdrnum(elf, &phdrCount) != 0)
	{
		errx(EXIT_FAILURE, "elf_getphdrnum() failed: %s.", elf_errmsg(-1));
	}

	for (int i = 0; i < phdrCount; i++)
	{
		GElf_Phdr phdr;
		gelf_getphdr(elf, i, &phdr);

		printSegmentInfo(elf, i, &phdr);
	}

	elf_end(elf);
	close(fd);
}
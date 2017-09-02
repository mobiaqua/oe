//
// signGP.c
// Read the x-load.bin file and write out the x-load.bin.ift file.
// The signed image is the original pre-pended with the size of the image
// and the load address.  If not entered on command line, file name is
// assumed to be x-load.bin in current directory and load address is
// 0x40200800.

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <string.h>
#ifdef __APPLE__
#include <malloc/malloc.h>
#else
#include <malloc.h>
#endif

struct ch_toc {
	unsigned int section_offset;
	unsigned int section_size;
	unsigned char unused[12];
	unsigned char section_name[12];
} __attribute__ ((__packed__));

struct chsettings {
	unsigned int section_key;
	unsigned char valid;
	unsigned char version;
	unsigned short reserved;
	unsigned int flags;
} __attribute__ ((__packed__));

struct ch_chsettings_nochram {
	struct ch_toc toc_chsettings;
	struct ch_toc toc_terminator;
	struct chsettings section_chsettings;
	unsigned char padding1[512 -
		    (sizeof(struct ch_toc) * 2 +
		     sizeof(struct chsettings))];
} __attribute__ ((__packed__));

static struct ch_chsettings_nochram config_header = {
	/* CHSETTINGS TOC */
	{(sizeof(struct ch_toc)) * 2,
	 sizeof(struct chsettings),
	 "",
	 {"CHSETTINGS"}
	},
	/* toc terminator */
	{0xFFFFFFFF,
	 0xFFFFFFFF,
	 {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
	  0xFF},
	 {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
	  0xFF}
	},
	/* CHSETTINGS section */
	{
	 0xC0C0C0C1,
	 0,
	 1,
	 0,
	 0
	},
	""
};

main(int argc, char *argv[])
{
	int	i;
	char	ifname[FILENAME_MAX], ofname[FILENAME_MAX], ch;
	FILE	*ifile, *ofile;
	unsigned long	loadaddr, len;
	struct stat	sinfo;
	int ch_add = 0;

	// Default to x-load.bin and 0x40200800.
	strcpy(ifname, "x-load.bin");
	loadaddr = 0x40200800;

	if ((argc == 2) || (argc == 3) || (argc == 4))
		strcpy(ifname, argv[1]);

	if ((argc == 3) || (argc == 4))
		loadaddr = strtol(argv[2], NULL, 16);

	if (argc == 4)
		ch_add = strtol(argv[3], NULL, 16);

	// Form the output file name.
	strcpy(ofname, ifname);
	strcat(ofname, ".ift");

	// Open the input file.
	ifile = fopen(ifname, "rb");
	if (ifile == NULL) {
		printf("Cannot open %s\n", ifname);
		exit(0);
	}

	// Get file length.
	stat(ifname, &sinfo);
	len = sinfo.st_size;

	// Open the output file and write it.
	ofile = fopen(ofname, "wb");
	if (ofile == NULL) {
		printf("Cannot open %s\n", ofname);
		fclose(ifile);
		exit(0);
	}

	if (ch_add) {
		if (fwrite(&config_header, 1, 512, ofile) <= 0) {
			fclose(ifile);
			exit(0);
		}
	}

	fwrite(&len, 1, 4, ofile);
	fwrite(&loadaddr, 1, 4, ofile);
	for (i=0; i<len; i++) {
		fread(&ch, 1, 1, ifile);
		fwrite(&ch, 1, 1, ofile);
	}

	fclose(ifile);
	fclose(ofile);
}

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define DISK_SIZE 0x200000
#define NULL_FILE "|0|"

void write_disk_image(size_t size, char name[32], char* data)
{
	name[31] = '\0';

	FILE *outputFile = fopen(name, "wb+");
	size_t written = fwrite(data, 1, size, outputFile);
	printf("DiskUtil : Wrote %lu Byte\n", written);
	fclose(outputFile);
}

void read_disk_image(size_t size, char name[32], char* data)
{
	name[31] = '\0';

	FILE *inputFile = fopen(name, "rb");
	size_t read = fread(data, 1, size, inputFile);
	printf("DiskUtil : Read %lu Bytes\n", read);

	fclose(inputFile);
}

int main(int argc, char* argv[])
{
	if (argc != 3) return -1;

	char *A = malloc(DISK_SIZE);

	if (strcmp(NULL_FILE, argv[1]) != 0)
        {
		read_disk_image(DISK_SIZE, argv[1], A);
	}

	write_disk_image(DISK_SIZE, argv[2], A);
	free(A);	
	
	return 0;
}

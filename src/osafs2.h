#ifndef OSAFS2
#define OSAFS2

#include <stdint.h>
#include <stdbool.h>
#include "string.h"
#include "io.h"
#define CLUSTSZ 503
#define MFC 64
#define MCC 64
#define EOF -1

int current_path_idx = -1;

enum
{
        MODE_ERR,
        MODE_READ,
        MODE_WRITE,
        MODE_APPEND,
};

typedef struct
{
        int position;
        int file;
        int mode;
} FILE;

#include "rtc.h"

typedef struct
{
        char Name[16];
        bool Exists;
        bool HasChildren;
        int  FFAT;
        int  ParentIdx;
        int  Size;
        DATE Modified;
} FileDescriptor;

typedef struct
{
        bool Taken;
        int  Fileidx;
        int  NextFAEIdx; // 0 on end of file
        char Cluster[CLUSTSZ];
} FAE; // fat allocation entry, part of the FAT.

typedef struct {
        char     Sign[4];
        uint8_t  Checksum;
        uint32_t StartOffset;
        uint32_t Version;
} ProgramHeader;

#define CHECKSUM 0xAA
#define PHVERSION 0x1

FAE            *FAT0;
FileDescriptor *FDS0;

#define StartOfFS 128

void init_fs()
{
        FAT0 = malloc(sizeof(FAE)*MCC);
        FDS0 = malloc(sizeof(FileDescriptor)*MFC);
        if (!FAT0 || !FDS0)
        {
                printf("failed to load fs");
                return;
        }
        memset(FAT0,0,sizeof(FAE)*MCC);
        memset(FDS0,0,sizeof(FileDescriptor)*MFC);
        printf("RAMFS Initialized\n");
}

int ffefae()
{
        for (int i = 0; i < MFC; ++i)
        {
                if (!FAT0[i].Taken)
                {
                        return i;
                }
        }

        return -1;
}

int ffefd() // find first _Empty file desc
{
        for (int i = 0; i < MFC; ++i)
        {
                if (FDS0[i].Exists == false)
                {
                        return i;
                }
        }

        return -1;
}

int _Exists(const char * name, int parent_idx) // return index + 1 if _Exists
{
        for (int i = 0; i < MFC; ++i)
        {
                if (FDS0[i].ParentIdx == parent_idx && !strncmp(FDS0[i].Name, name, 16))
                {
                        return i+1;
                }
        }

        return 0;
}

void FreeFAE(int FAEIdx)
{
        if (FAEIdx == -1 || FAEIdx >= MFC) return;
        memset(FAT0[FAEIdx].Cluster,0,CLUSTSZ);
        FAT0[FAEIdx].Fileidx=-1;
        if (FAT0[FAEIdx].NextFAEIdx)
        {
                FreeFAE(FAT0[FAEIdx].NextFAEIdx);
        }
        FAT0[FAEIdx].NextFAEIdx=0;
        FAT0[FAEIdx].Taken=false;
}

void _Empty(int fileIDX)
{
        int ffae = FDS0[fileIDX].FFAT;
        if (fileIDX == -1 || fileIDX >= MFC || ffae == -1) return;
        FreeFAE(ffae);
        FDS0[fileIDX].FFAT=-1;
}

void _CreateF(const char *,int);
FILE* fopen(const char * name, const char *mode)
{
        FILE * f = malloc(sizeof(f));
        f->file = _Exists(name, current_path_idx);

        f->position = 0;
        if (strcmp(mode,"r")==0) {f->mode = MODE_READ;}
        if (strcmp(mode,"w")==0) { _CreateF(name, current_path_idx); f->file = _Exists(name, current_path_idx); f->mode = MODE_WRITE; _Empty(f->file);}
        if (strcmp(mode,"a")==0) {f->mode = MODE_APPEND;}
        return f;
}

void fclose(FILE * fp)
{
        fp->file=0;
        fp->mode=0;
        fp->position=0;
        free(fp);
}

int FlFAE(int fileIDX)
{
        if (fileIDX == -1 || fileIDX >= MFC || FDS0[fileIDX].FFAT == -1) return -1;
        int fae = FDS0[fileIDX].FFAT;
        int prev = 0;
        do
        {
                prev = fae;
                fae = FAT0[fae].NextFAEIdx;
        } while (fae);

        return prev;
}

void AllocFAE(int fileIDX)
{
        int fafae = ffefae();

        if (fileIDX == -1 || fileIDX >= MFC || fafae == -1) return;

        FAT0[fafae].Taken = true;
        FAT0[fafae].Fileidx=fileIDX;
        FAT0[fafae].NextFAEIdx=0;
        if (FDS0[fileIDX].FFAT == -1)
        {
                FDS0[fileIDX].FFAT = fafae;
                return;
        }

        int lastidx = FlFAE(fileIDX);
        FAT0[lastidx].NextFAEIdx = fafae;
}

void Cd(const char * name)
{
        if (strncmp(name,"..",2)==0 && current_path_idx != -1)
        {
                current_path_idx = FDS0[current_path_idx].ParentIdx;
                return;
        }
        int x = _Exists(name, current_path_idx)-1;
        if (x == -1)
                return;
        current_path_idx = x;
}

void _CreateF(const char * name, int parent_idx)
{
        int fefd = ffefd();
        if (fefd == -1 || _Exists(name, parent_idx))
        {
                return;
        }

        FDS0[parent_idx].HasChildren = true;
        FDS0[fefd].Exists = true;
        FDS0[fefd].ParentIdx=parent_idx;
        FDS0[fefd].FFAT=-1;
        FDS0[fefd].HasChildren=false;
        FDS0[fefd].Size=0;
        strncpy(FDS0[fefd].Name,name,16);
}

int ClusterCount(int fileIDX)
{
        if (fileIDX == -1 || fileIDX >= MFC || FDS0[fileIDX].FFAT == -1) return 0;
        int fae = FDS0[fileIDX].FFAT;
        int c=0;
        do
        {
                fae = FAT0[fae].NextFAEIdx;
                ++c;
        } while (fae);
        return c;
}

void _ReadF(const char *name, int parent_idx, char *buff, int len)
{
        int idx = _Exists(name, parent_idx) - 1;
        if (idx == -1 || len <= 0)
        {
                return;
        }

        int fae = FDS0[idx].FFAT;
        int br = 0;

        while (fae != -1 && br < len)
        {
                int bytcpy = (len - br > CLUSTSZ) ? CLUSTSZ : (len - br);
                memcpy(buff + br, FAT0[fae].Cluster, bytcpy);
                br += bytcpy;

                fae = FAT0[fae].NextFAEIdx;
        }

        if (br < len)
        {
                buff[br] = '\0';
        }
}

void _WriteF(const char *name, int parent_idx, const char *data, int data_length)
{
        if (data_length <= 0) return;
        _CreateF(name,parent_idx);
        int idx = _Exists(name,parent_idx) - 1;
        _Empty(idx);
        FDS0[idx].Size = data_length;
        int clusters_needed = (data_length + CLUSTSZ - 1) / CLUSTSZ; // Calculate the number of clusters needed
        const char *data_ptr = data;
        for (int i = 0; i < clusters_needed; ++i)
        {
                AllocFAE(idx);
                int last_cluster = FlFAE(idx);

                if (last_cluster == -1)
                {
                        return;
                }

                int bytcpy = (data_length > CLUSTSZ) ? CLUSTSZ : data_length;
                memcpy(FAT0[last_cluster].Cluster, data_ptr, bytcpy);

                data_ptr += bytcpy;
                data_length -= bytcpy;
        }

        FDS0[idx].Modified = get_date();
}

FILE fgetf(const char * name, int parent_idx)
{
        FILE f;
        f.position = 0;
        f.file = _Exists(name,parent_idx);
        return f;
}

int fputc(char c, FILE *fp)
{
        if (!fp || fp->file == 0 || fp->file >= MFC)
                return -1;

        int fileIdx = fp->file-1;
        if (!FDS0[fileIdx].Exists)
                return -1;

        int pos = FDS0[fileIdx].Size;
        int clusterIdx = FDS0[fileIdx].FFAT;
        int br = 0;

        while (clusterIdx != -1 && br + CLUSTSZ <= pos)
        {
                br += CLUSTSZ;
                clusterIdx = FAT0[clusterIdx].NextFAEIdx;
        }

        if (clusterIdx == -1)
        {
                AllocFAE(fileIdx);
                clusterIdx = FlFAE(fileIdx);
                if (clusterIdx == -1)
                return -1;
        }

        int clusterOffset = pos - br;
        FAT0[clusterIdx].Cluster[clusterOffset] = c;
        FDS0[fileIdx].Size++;
        return 0;
}

enum SEEK_T
{
        SEEK_SET,
        SEEK_CUR,
        SEEK_END,
};

void fseek(FILE *fp, int p, int t)
{
        if (fp->file <= 0 || fp->file > MFC)
                return;
        int fileIdx = fp->file-1;
        if (!FDS0[fileIdx].Exists || FDS0[fileIdx].FFAT == -1)
                return;
        
        if (t == (int)SEEK_SET)
        {
                fp->position = p;
        }
        else if (t == (int)SEEK_CUR)
        {
                fp->position += p;
        }
        else if (t == (int)SEEK_END)
        {
                fp->position  = FDS0[fileIdx].Size;
                fp->position -= p;
        }
}

inline void rewind(FILE *fp)
{
        fseek(fp,0,SEEK_END);
}

char fgetc(FILE *fp)
{
        if (!fp || fp->file <= 0 || fp->file > MFC)
                return -1;

        int fileIdx = fp->file-1;
        if (!FDS0[fileIdx].Exists || FDS0[fileIdx].FFAT == -1)
                return -1;

        int ci = FDS0[fileIdx].FFAT;
        int p = fp->position;
        if (p>FDS0[fileIdx].Size)
        {
                return -1;
        }
        int br = 0;

        while (ci != -1 && br + CLUSTSZ <= p)
        {
                br += CLUSTSZ;
                ci = FAT0[ci].NextFAEIdx;
        }

        if (ci == -1 || p - br >= CLUSTSZ)
                return -1;

        char ch = FAT0[ci].Cluster[p - br];
        ++fp->position;

        return ch;
}

void _DeleteF(const char * name, int parent_idx);

static void _DeleteChildren(int parent_idx)
{
        if ((FDS0[parent_idx].Exists && FDS0[parent_idx].HasChildren) || parent_idx == -1)
        {
                for (int i = 0; i < MFC; ++i)
                {
                        if (FDS0[i].Exists && FDS0[i].ParentIdx == parent_idx)
                        {
                                _DeleteF(FDS0[i].Name, parent_idx);
                        }
                }

                if (parent_idx != -1) // root
                        FDS0[parent_idx].HasChildren = false;
        }
}

void _DeleteF(const char *name, int parent_idx)
{
        int idx = _Exists(name, parent_idx) - 1;
        if (idx == -1)
        {
                return;
        }

        _DeleteChildren(idx);
        _Empty(idx);
        FDS0[idx].Exists = false;
}

void ListF()
{
        for (int i = 0; i < MFC; ++i)
        {
                if (FDS0[i].Exists && FDS0[i].ParentIdx == current_path_idx)
                {
                        printf("%d/%d/%d  %d",FDS0[i].Modified.D,FDS0[i].Modified.M,FDS0[i].Modified.Y+BASE_YEAR,FDS0[i].Size);
                        putc('\t');
                        putsn(FDS0[i].Name,16);
                        putc(FDS0[i].HasChildren ? '/' : '*');
                        putc('\n');
                }
        }
}

void ListPath(char *path)
{
        int tmp = current_path_idx;
        char *name = strtok(path,"/");
        while (name)
        {
                name = strtok(NULL,"/");
                Cd(name);
        }

        for (int i = 0; i < MFC; ++i)
        {
                if (FDS0[i].Exists && FDS0[i].ParentIdx == current_path_idx)
                {
                        printf("%d/%d/%d  %d",FDS0[i].Modified.D,FDS0[i].Modified.M,FDS0[i].Modified.Y+BASE_YEAR,FDS0[i].Size);
                        putc('\t');
                        putsn(FDS0[i].Name,16);
                        putc(FDS0[i].HasChildren ? '/' : '*');
                        putc('\n');
                }
        }
        current_path_idx = tmp;
}

int ftell(FILE *fp)
{
        return fp->position;
}

extern void jump_usermode(int addr);

int AppendTaskRing0(char * name, void (*start)(void));
int _ExecuteF(char *filename, int parentidx)
{
        int idx = _Exists(filename, parentidx) - 1;
        if (idx == -1)
        {
                return -1;
        }

        int file_size = FDS0[idx].Size;
        if (file_size <= 0)
        {
                return 0;
        }

        char *buffer = malloc(file_size);
        ProgramHeader * progh = (ProgramHeader *)buffer;
        if (!buffer)
        {
                return -1;
        }
        _ReadF(filename, parentidx, buffer, file_size);
        if (progh->Checksum == CHECKSUM && strncmp(progh->Sign,"OSAX",4)==0 && progh->Version == PHVERSION && progh->StartOffset>0)
        {
                void (*func_ptr)() = (void (*)())progh->StartOffset+(int)buffer;
                return AppendTaskRing0(filename,func_ptr);
        }
        else
        {
                free(buffer);
                buffer=NULL;
                return -1;
        }
}

void CreateF(const char * name)
{
        _CreateF(name,current_path_idx);
}

void DeleteF(const char * name)
{
        _DeleteF(name,current_path_idx);
}

void WriteF(const char * name, void * data, int size)
{
        _WriteF(name,current_path_idx,data,size);
}

void ReadF(const char * name, void * data, int size)
{
        _ReadF(name,current_path_idx,data,size);
}

int ExecuteF(char * name)
{
        return _ExecuteF(name,current_path_idx);
}

int Exists(const char * name)
{
        return _Exists(name,current_path_idx);
}

const char * ActiveDir()
{
        if (current_path_idx == -1)
        {
                return "\0";
        }
        return FDS0[current_path_idx].Name;
}

const char * ActiveDirParen()
{
        if (current_path_idx == -1)
        {
                return "\0";
        }
        return FDS0[FDS0[current_path_idx].ParentIdx].Name;
}

int fwrite(void *src, int size, int count, FILE *fp)
{
        char *buff = src;
        if (!fp || fp->file == 0 || !size || !count)
                return -1;
        int len = size*count;
        int idx = fp->file-1;
        if (fp->mode == MODE_WRITE || fp->mode == MODE_APPEND)
        {
                /*:trollface:*/
                int written = 0;
                while (len--)
                {
                        fputc(*(buff++),fp);
                        ++written;
                }
        
                FDS0[idx].Modified = get_date();
                return written;
        }
        return -1;
}

int fread(void *dst, int size, int count, FILE *fp)
{
        char *buff = dst;
        if (!fp || fp->file == 0 || !size || !count)
        {
                return -1;
        }

        int len = size*count;
        if (fp->mode == MODE_READ)
        {
                /*:trollface:*/
                char chr = 0;
                int read = 0;
                while (len-- && chr != EOF)
                {
                        *(buff++) = fgetc(fp);
                        ++read;
                }

                return read;
        }
        return -1;
}

void *wfread(FILE *fp) /* read whole file */
{
        fp->position=0;
        char *data = malloc(FDS0[fp->file-1].Size);
        if (!data) return NULL;
        fread(data,FDS0[fp->file-1].Size,1,fp);
        return data;
}

#endif

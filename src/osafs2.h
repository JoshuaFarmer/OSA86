#ifndef OSAFS2
#define OSAFS2

#include <stdint.h>
#include <stdbool.h>
#include "string.h"
#define CLUSTSZ 503
#define MFC 512
#define MCC 512
#define EOF -1

int current_path_idx = -1;

typedef int FILE;

typedef struct
{
        char Name[16];
        char padd[2];
        bool Exists;
        bool HasChildren;
        int  FFAT;
        int  ParentIdx;
        int  Size;
} FileDescriptor;

typedef struct
{
        bool Taken;
        int  Fileidx;
        int  NextFAEIdx; // 0 on end of file
        char Cluster[CLUSTSZ];
} FAE; // fat allocation entry, part of the FAT.

FAE            * FAT0=NULL;
FileDescriptor * FDS0=NULL;

void InitRamFS()
{
        if (FAT0) {free(FAT0);}
        if (FDS0) {free(FDS0);}
        FAT0=malloc(sizeof(FAE)*MCC);
        FDS0=malloc(sizeof(FileDescriptor)*MFC);
        memset(FAT0,0,sizeof(FAE)*MCC);
        memset(FDS0,0,sizeof(FileDescriptor)*MFC);
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

FILE* fopen(const char * name, ...)
{
        FILE * f = malloc(sizeof(f));
        *f = _Exists(name, current_path_idx);
        return f;
}

void fclose(FILE * fp)
{
        free(fp);
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
        int idx = _Exists(name, parent_idx) - 1;
        if (idx == -1 || data_length <= 0)
        {
                return;
        }

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
}

FILE fgetf(const char * name, int parent_idx)
{
        int f = _Exists(name,parent_idx);
        return f;
}

int fputc(char c, FILE * fp)
{
    if (*fp <= 0 || *fp > MFC)
        return -1;

    int fileIdx = *fp-1;
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

    if (clusterIdx == -1) // Allocate a new cluster if needed
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

static int pos[MFC] = {0};

enum    SEEK_T
{
        SEEK_SET,
        SEEK_CUR,
        SEEK_END,
};

void fseek(FILE fp, int p, int t)
{
        if (fp <= 0 || fp > MFC)
                return;
        int fileIdx = fp-1;
        if (!FDS0[fileIdx].Exists || FDS0[fileIdx].FFAT == -1)
                return;
        
        if (t == (int)SEEK_SET)
        {
                pos[fileIdx] = p;
        }
        else if (t == (int)SEEK_CUR)
        {
                pos[fileIdx] += p;
        }
        else if (t == (int)SEEK_END)
        {
                pos[fileIdx]  = FDS0[fileIdx].Size;
                pos[fileIdx] += p;
        }
}

inline void rewind(FILE fp)
{
        fseek(fp,0,SEEK_END);
}

char fgetc(FILE fp)
{
        if (fp <= 0 || fp > MFC)
                return -1;

        int fileIdx = fp-1;
        if (!FDS0[fileIdx].Exists || FDS0[fileIdx].FFAT == -1)
                return -1;

        int ci = FDS0[fileIdx].FFAT;
        int p = pos[fileIdx];
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
        pos[fileIdx]++;

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
                        putsn(FDS0[i].Name,16);
                        printf("%c .sz=%d\n",FDS0[i].HasChildren ? '/' : '*', FDS0[i].Size);
                }
        }
}

int ftell(FILE fp)
{
        if (fp <= 0 || fp > MFC)
                return 0;
        return pos[fp-1];
}

int _ExecuteF(const char *filename, int parentidx)
{
        int idx = _Exists(filename, parentidx) - 1;
        if (idx == -1) {
                return 2;
        }

        int file_size = FDS0[idx].Size;
        if (file_size <= 0) {
                return 0;
        }

        char *buffer = malloc(file_size);
        if (!buffer) {
                return -1;
        }

        _ReadF(filename, parentidx, buffer, file_size);

        int (*func_ptr)(int x) = (int (*)())buffer;
        int res = func_ptr((int)CallF);
        free(buffer);
        return res;
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

int ExecuteF(const char * name)
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

#endif

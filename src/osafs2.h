#ifndef OSAFS2
#define OSAFS2

#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#define CLUSTSZ 500
#define MFC 32
#define MCC 16
#define EOF -1

typedef struct
{
        char Name[16];
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

FAE            FAT0[MCC];
FileDescriptor FDS0[MFC];

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

int ffefd() // find first empty file desc
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

int Exists(const char * name, int parent_idx) // return index + 1 if exists
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

void Empty(int fileIDX)
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

void CreateF(const char * name, int parent_idx)
{
        int fefd = ffefd();
        if (fefd == -1 || Exists(name, parent_idx))
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

void Read(const char *name, int parent_idx, char *buff, int len)
{
        int idx = Exists(name, parent_idx) - 1;
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

void Write(const char *name, int parent_idx, const char *data, int data_length)
{
        int idx = Exists(name, parent_idx) - 1;
        if (idx == -1 || data_length <= 0)
        {
                return;
        }

        Empty(idx);
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

typedef int FILE;

FILE fgetf(const char * name, int parent_idx)
{
        int f = Exists(name,parent_idx);
        return f;
}

int fputc(char c, FILE fp)
{
    if (fp <= 0 || fp > MFC)
        return -1;

    int fileIdx = fp-1;
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
    FDS0[fileIdx].Size++; // Increment the file size

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

void DeleteF(const char *name, int parent_idx);

static void DeleteChildren(int parent_idx)
{
        if ((FDS0[parent_idx].Exists && FDS0[parent_idx].HasChildren) || parent_idx == -1)
        {
                for (int i = 0; i < MFC; ++i)
                {
                        if (FDS0[i].Exists && FDS0[i].ParentIdx == parent_idx)
                        {
                                DeleteF(FDS0[i].Name, parent_idx);
                        }
                }

                if (parent_idx != -1) // root
                        FDS0[parent_idx].HasChildren = false;
        }
}

void DeleteF(const char *name, int parent_idx)
{
        int idx = Exists(name, parent_idx) - 1;
        if (idx == -1)
        {
                return;
        }

        DeleteChildren(idx);
        Empty(idx);
        FDS0[idx].Exists = false;
}

#endif

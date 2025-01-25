#pragma once

#include "malloc.h"
//#include "osafs.h"

typedef enum {
        ROOT_LEVEL,
        USER_LEVEL,
} PermLevel;

typedef struct {
        PermLevel perms;
        char name[32];
} User;

char* create_info(User* usr) {
        char* num = malloc(32);
        itoa(usr->perms, num, 10);

        num[strlen(num)] = '\n';
        char* x = strcat(num, usr->name);
        free(num);
        return x;
}

void create_user(const char name[], PermLevel perms) {
        char* nam = strcat("home/", name);
        char* hom = strcat(nam, "/info.txt");

        User x;
        x.perms = perms;
        strcpy((char*)x.name, name);

        //if (create(nam, true)) {
        //        create(hom, false);
        //        char* data = create_info(&x);
        //        //write_file(hom, (const uint8_t*)data, strlen(data));
        //        free(data);
        //}

        free(nam);
        free(hom);
}

/*
void switch_user(const char name[]) {
}
*/
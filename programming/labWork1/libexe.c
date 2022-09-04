#include "plugin_api.h"
#include <stdlib.h>
#include <string.h>
#include <errno.h>

static char* author = "Liponina Ekaterina Alekseevna N3248";
static char* purpose = "find executable files";

static char* key1 = "exe";
static char* key2 = "exe-magic";

static char* descr1 = "find files which allowed to execute";
static char* descr2 = "find files which contain magic numbers";

static struct plugin_option exe_plugin_option[] = {
	{"exe", 0, 0, 0},
	{"exe-magic", 0, 0, 0}
};

int plugin_get_info(struct plugin_info *ppi){
    if (!ppi) {
        fprintf(stderr, "ERROR: invalid argument\n");
        return -1;
    }
    ppi->plugin_author = author;
    ppi->plugin_purpose = purpose;
    ppi->sup_opts_len = 2;
    ppi->sup_opts = exe_plugin_option;
    ppi->sup_opts[0].opt_descr = descr1;
    ppi->sup_opts[1].opt_descr = descr2;
}

int plugin_process_file(const char *fname, struct option in_opts[], size_t in_opts_len){

    if (!fname || !in_opts || in_opts_len <= 0){
        errno = EINVAL;
        return -1;
    }

    int* true_ = (int*)malloc(sizeof(int) * in_opts_len);
    if (!true_){
        perror("malloc");
        exit(EXIT_FAILURE);
    }
    for (int i = 0; i < in_opts_len; i++)
        true_[i] = 0;
    for (int i = 0; i < in_opts_len; i++){
        if (!strcmp(in_opts[i].name, "exe")){
            struct stat buf;
            stat(fname, &buf);
            if (buf.st_mode & S_IXUSR || buf.st_mode & S_IXGRP || buf.st_mode& S_IXOTH){
                true_[i] = 1;
            }       
        }
        else {
            //exe-magic
            true_[i] = 1;
        }
    }
    int res = true_[0];
    for (int i = 1; i < in_opts_len; i++){
        res = res & true_[i];
    }
    if(true_) free(true_);
    if (res == 1)
        return 0;
    else if (res == 0)
        return 1;
    else 
        return -1;
    return 0;
}
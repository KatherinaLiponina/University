#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include "plugin_api.h"

static char* author = "Liponina Ekaterina Alekseevna N3248";
static char* purpose = "find if file contains float number";

static char* key = "float-in-bin";
static char* descr = "find if binary file contain float number";

static struct plugin_option float_in_bin_plugin_option[] = { 
    { "float-in-bin", required_argument, 0, 0}
};

int plugin_get_info(struct plugin_info *ppi){
    ppi->plugin_author = author;
    ppi->plugin_purpose = purpose;
    ppi->sup_opts_len = 1;
    ppi->sup_opts = float_in_bin_plugin_option;
    ppi->sup_opts->opt_descr = descr;
    return 0;
}

int plugin_process_file(const char *fname, struct option in_opts[], size_t in_opts_len){
    static char** symbol_storage = NULL; //все аналогично другому плагину, за исключением symbol_storage
    static int** adress_storage = NULL;
    static int st_size = 0;

    static int debuging = -1;
    if (debuging == -1){
        char* DEBUG = getenv("LAB1DEBUG");
        if (DEBUG != NULL && DEBUG[0] == '1'){
            debuging = 1;
        }
        else {
            debuging = 0;
        }   
    }

    if (in_opts_len == -1){
        for (int i = 0; i < st_size; i++){
            free(symbol_storage[i]);
        }
        free(symbol_storage);
        symbol_storage = NULL;
        free(adress_storage);
        adress_storage = NULL;
        if (debuging) printf("freed static in plugin_process_file in float-in-bin\n");
        return -1;
    }

    if (!fname || !in_opts || in_opts_len <= 0){
        errno = EINVAL;
        return -1;
    }

    int *true_ = (int *)malloc(sizeof(int) * in_opts_len);
    if (!true_) {
        perror("malloc");
        return -1;
    }
    for (int i = 0; i < in_opts_len; i++)
        true_[i] = 0;
    int fd = open(fname, O_RDONLY);
    if (fd == -1) {
        perror("open");
        return -1;
    }

    for (int j = 0; j < in_opts_len; j++){
        char* symbol_string;
        char reverse[5];
        for (int i = 0; i < st_size; i++){
            if (in_opts[j].flag == adress_storage[i]){
                symbol_string = symbol_storage[i];
                goto done;
            }
        }
        st_size++;
        symbol_storage = (char**)realloc(symbol_storage, st_size);
        adress_storage = (int**)realloc(adress_storage, st_size);
        adress_storage[st_size - 1] = in_opts[j].flag;
        char* str = (char*)in_opts[j].flag;
        int len = 0;
        while (str[len++] != '\0'); len--;
        char* endptr = NULL, **end;
        end = &endptr;
        errno = 0;
        float res = strtof(str, end);
        if (errno != 0){
            strerror(errno);
            return -1;
        }
        if (endptr && (str + len) != endptr){
            printf("Wrong argument in --float-bin %s\n", str);
            return -1;
        }
        char symbol[5];
        char* ptr = (char*)&res; //смотрим не на биты, а интерпретируем их сразу как char
        for (int i = 0; i < 4; i++){
            symbol[i] = *(ptr + i);
        }
        symbol[4] = '\0';
        symbol_storage[st_size - 1] = (char*)malloc(sizeof(char) * 5);
        for (int k = 0; k < 5; k++){
            symbol_storage[st_size - 1][k] = symbol[k];
        }
        symbol_string = symbol_storage[st_size - 1];

        done:
        for (int i = 0; i < 4; i++){
            reverse[i] = symbol_string[3 - i];
        }
        reverse[4] = '\0';

        int n;
        char read_buf[32 * 8 + 1];
        int found;
        while ((n = read(fd, read_buf, 256)) >= 4)
        {
            for (int p = 0; p < n - 4; p++)
            {
                int k = 0;
                while (*(read_buf + p + k) == symbol_string[k])
                {
                    k++;
                    if (k == 3)
                    {
                        true_[j] = 1;
                        if (debuging){
                            printf("\tdebug:\n");
                            printf("\t\tin file %s\n", fname);
                            printf("\t\tfinded ");
                            for (int l = 0; l < 4; l++){
                                printf("(int)%d ", (int)symbol_string[l]);
                            }
                            printf("on symbol %d\n", p);
                        }
                        goto finded;
                    }
                }
                k = 0;
                while (*(read_buf + p + k) == reverse[k])
                {
                    k++;
                    if (k == 3)
                    {
                        true_[j] = 1;
                        if (debuging){
                            printf("\tdebug:\n");
                            printf("\t\tin file %s\n", fname);
                            printf("\t\tfinded ");
                            for (int l = 0; l < 4; l++){
                                printf("(int)%d ", (int)reverse[l]);
                            }
                            printf("on symbol %d (reverse)\n", p);
                        }
                        goto finded;
                    }
                }
            }
            lseek(fd, -3, SEEK_CUR);
        }
    finded:
        close(fd);      
    }
    int res = true_[0];
    for (int i = 1; i < in_opts_len; i++)
    {
        res = res & true_[i];
    }
    if (true_)
        free(true_);
    if (res == 1) //true
        return 0;
    else if (res == 0) //false
        return 1;
    else
        return -1; //???
}
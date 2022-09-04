#include "plugin_api.h"
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <sys/mman.h>
#include <stdint.h> 
#include <math.h>
extern errno; 

static char* author = "Wonderful person";
static char* purpose = "find subsequence";
static char* key = "bin-seq";
static char* descr = "find subsequence";

static struct plugin_option float_plugin_option[] = {
    { "bin-seq", required_argument, 0, 0} 
};

int plugin_get_info(struct plugin_info *ppi) {
    if (!ppi) {
        fprintf(stderr, "ERROR: invalid argument\n");
        return -1;
    }
    ppi->plugin_author = author;
    ppi->plugin_purpose = purpose;
    ppi->sup_opts_len = 1;
    ppi->sup_opts = float_plugin_option;
    ppi->sup_opts->opt_descr = descr;
    return 0;
}

int strtoi(char*);
char* itostr(int);
char* strtostr(char*, int);

int plugin_process_file(const char *fname, struct option in_opts[], size_t in_opts_len){
    int debug = 0;
    if (getenv("LAB1DEBUG") != NULL){
        debug = 1;
    }

    if (!fname || !in_opts || in_opts_len <= 0){
        errno = EINVAL;
        return -1;
    }
    if (in_opts->name != "bin-seq"){
        printf("error: option doesn't fit\n");
        return -1;
    }
    int fd = -1, n = 0;
    if (-1 == (fd = open(fname, O_RDONLY))){
        perror("open");
        exit(EXIT_FAILURE);
    }

    char * str = (char*)in_opts->flag;
    unsigned int num = strtoi(str);
    char* substr = itostr(num);
    if (num == 0 || num == 1){
        return 1;
    }
    int bit_req = (int)log2(num) + 1; //сколько битов в последовательности
    int byte_req = bit_req / 8 + 1;

    int length;
    struct stat st;
    if (-1 == fstat(fd, &st)){
        perror("stat");
        exit(EXIT_FAILURE);
    }
    length = st.st_size;
    void * ptr = mmap(0, length, PROT_READ, MAP_PRIVATE, fd, 0);

    char * pstr = (char*)ptr;
    int res = 1;
    for (int i = 0; i < length - byte_req + 1; i++){
        char * tmp = strtostr(pstr + i, byte_req);
        if (strstr(tmp, substr) != NULL){
            res = 0;
            free(tmp);
            break;
        }
        free(tmp);
        tmp = NULL;
    }
    munmap(ptr, length);
    free(substr);
    return res;
}

int strtoi(char* str){
    int len = strlen(str);
    for (int i = 0; i < len; i++){
        if (!(str[i] >= '0' && str[i] <= '9' || str[i] == 'b' || str[i] == 'x' || str[i] >= 'a' && str[i] <= 'f')){
            return -1;
        }
    }
    int result = 0, power = 1;
    if (str[1] == 'x'){
        for (int i = len - 1; i >= 2; i--){
            if (str[i] >= '0' && str[i] <= '9'){
                result += ((int)str[i] - 48) * power;
            }
            else if (str[i] >= 'a' && str[i] <= 'f'){
                result += ((int)str[i] - 97 + 10) * power;
            }
            else{
                return -1;
            }
            power *= 16;
        }
    }
    else if (str[1] == 'b'){
        for (int i = len - 1; i >= 2; i--){
            if (str[i] != '0' && str[i] != '1'){
                return -1;
            }
            result += ((int)str[i] - 48) * power;
            power *= 2;
        }
    }
    else {
        for (int i = len - 1; i >= 0; i--){
            if (str[i] >= '0' && str[i] <= '9'){
                result += ((int)str[i] - 48) * power;
                power *= 10;
            }
            else
                return -1;
        }
    }
    return result;
}

char* itostr(int n){
    char * tmp = NULL;
    int size = 0;
    for (size = 1; n != 0; size++) {
        char c = n % 2 ? '1' : '0';
        n >>= 1;
        tmp = (char*)realloc(tmp, sizeof(char) * (size + 1));
        tmp[size - 1] = c;
        tmp[size] = '\0';
    }
    size--;
    for (int i = 0; i < size / 2; i++) {
        char t = tmp[i];
        tmp[i] = tmp[size - i - 1];
        tmp[size - i - 1] = t;
    }
    return tmp;
}

char* strtostr(char* str, int len){
    char* tmp = (char*)malloc(sizeof(char) * (len * 8 + 1));
    for (int i = 0; i < len; i++){
        for (int j = 7; j >= 0; j--){
            tmp[i * 8 + 7 - j] = (str[i] >> j) % 2 ? '1' : '0';
        }
    }
    tmp[len*8] = '\0';
    return tmp;
}
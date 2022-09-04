#include "plugin_api.h"
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <sys/mman.h>
#include <stdint.h>
extern errno; 

static char* author = "Liponina Ekaterina Alekseevna";
static char* purpose = "calculate control sum";
static char* key = "crc-32";
static char* descr = "find if file have same sum";

static struct plugin_option float_plugin_option[] = {
    { "crc-32", required_argument, 0, 0} 
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
uint_least32_t crc32(unsigned char * buf, size_t len);

int plugin_process_file(const char *fname, struct option in_opts[], size_t in_opts_len){

    int debug = 0;
    if (getenv("LAB1DEBUG") != NULL){
        debug = 1;
    }

    if (!fname || !in_opts || in_opts_len <= 0){
        errno = EINVAL;
        return -1;
    }
    if (in_opts->name != "crc-32"){
        printf("error: option doesn't fit\n");
        return -1;
    }
    int fd = -1, n = 0;
    if (-1 == (fd = open(fname, O_RDONLY))){
        perror("open");
        exit(EXIT_FAILURE);
    }

    char * str = (char*)in_opts->flag;
    int base_sum = strtoi(str);
    if (base_sum == -1){
        printf("error: wrong argument\n");
        return -1;
    }

    int length;
    struct stat st;
    if (-1 == fstat(fd, &st)){
        perror("stat");
        exit(EXIT_FAILURE);
    }
    length = st.st_size;
    void * ptr = mmap(0, length, PROT_READ, MAP_PRIVATE, fd, 0);
    int result = crc32(ptr, length);
    munmap(ptr, length);
    if(debug){
        printf("file: %s, sum = 0x%x\n", fname, result);
    }
    if (result == base_sum){
        return 0;
    }
    return 1;
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

uint_least32_t crc32(unsigned char * buf, size_t len){
    uint_least32_t crc_table[256];
    uint_least32_t crc;
    for (int i = 0; i < 256; i++){
        crc = i;
        for (int j = 0; j < 8; j++){
            crc = crc & 1 ? (crc >> 1) ^ 0xEDB88320UL : crc >> 1;
        }
        crc_table[i] = crc;
    }
    crc = 0xFFFFFFFFUL;
    while(len--){
        crc = crc_table[(crc ^ *buf++) & 0xFF] ^ (crc >> 8);
    }
    return crc ^ 0xFFFFFFFFUL;
}

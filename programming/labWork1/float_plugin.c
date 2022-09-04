#include "plugin_api.h"
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
extern errno; 

static char* author = "Liponina Ekaterina Alekseevna N3248";
static char* purpose = "find float number in text file";
static char* key = "float-bin";
static char* descr = "find if there is this float number in text file";

static struct plugin_option float_plugin_option[] = {
    { "float-bin", required_argument, 0, 0} 
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

int plugin_process_file(const char *fname, struct option in_opts[], size_t in_opts_len){
    static char** byte_storage = NULL; //статические переменные для храниния уже повстерчавшихся аргументов
    static int** adress_storage = NULL; //сравниваются адреса
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

    if (in_opts_len == -1){ //статические переменные надо освободить, для этого в main вызывается
        for (int i = 0; i < st_size; i++){ //plugin_process_file(NULL, NULL, -1);
            free(byte_storage[i]);
        }
        free(byte_storage);
        byte_storage = NULL;
        free(adress_storage);
        adress_storage = NULL;
        if (debuging) printf("freed static in plugin_process_file in float-bin\n");
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
        char* byte_string;
        char reverse[33];
        for (int i = 0; i < st_size; i++){
            if (in_opts[j].flag == adress_storage[i]){
                byte_string = byte_storage[i]; //если уже есть, берем ее
                goto done;
            }
        }
        st_size++;
        byte_storage = (char**)realloc(byte_storage, st_size * sizeof(char*)); //иначе записываем в массив
        adress_storage = (int**)realloc(adress_storage, st_size * sizeof(int*));
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
        char bytes[33];
        int *p = (int *)&res;
        for (int i = 31; i >= 0; i--)
        {
            int k = (*p >> i) % 2;
            if (k < 0)
                k *= -1;
            bytes[i] = (char)(k + '0');
        }
        bytes[32] = '\0';
        byte_storage[st_size - 1] = (char*)malloc(sizeof(char) * 33);
        for (int k = 0; k < 33; k++){
            byte_storage[st_size - 1][k] = bytes[k];
        }
        byte_string = byte_storage[st_size - 1];
        //printf("byte_string: %s\n", byte_string);

        done:
        for (int i = 0; i < 32; i++){ //обратная строка
            reverse[i] = byte_string[31 - i];
        }
        reverse[32] = '\0';
        //printf("reverse_string: %s\n", reverse);


        int n;
        char read_buf[32 * 8 + 1];
        int found;
        while ((n = read(fd, read_buf, 256)) >= 32) //читаем по 256 байт
        {
            for (int p = 0; p < n - 32; p++) //и просматриваем по 32
            {
                int k = 0;
                while (*(read_buf + p + k) == byte_string[k]) //если совпадает - переход на finded
                {
                    if (k == 31)
                    {
                        true_[j] = 1;
                        if (debuging){
                            printf("\tdebug:\n");
                            printf("\t\tin file %s\n", fname);
                            printf("\t\tfinded %s on symbol %d\n", byte_string, p);
                        }
                        goto finded;
                    }
                    k++;
                }
                k = 0;
                while (*(read_buf + p + k) == reverse[k])
                {
                    if (k == 31)
                    {
                        true_[j] = 1;
                        if (debuging){
                            printf("\tdebug:\n");
                            printf("\t\tin file %s\n", fname);
                            printf("\t\tfinded %s on symbol %d\n", reverse, p);
                        }
                        goto finded;
                    }
                    k++;
                }
            }
            lseek(fd, -31, SEEK_CUR);
        }
    finded:
        close(fd);      
    }
    int res = true_[0];
    for (int i = 1; i < in_opts_len; i++) //все аргументы и друг с другом
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
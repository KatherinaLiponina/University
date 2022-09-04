#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <getopt.h>
#include <dirent.h>
#include <dlfcn.h>
#include <fcntl.h>
#include <ftw.h>
#include "plugin_api.h"

int debug = 0;
extern char* optarg;
extern int optind, opterr, optopt;

typedef struct {
    int and_;
    int or_;
    int not_;
    char* libraries;
    char* directory;

    int plugin_amount;
    void ** handler;

    int option_amount;
    struct option* options;
    int * pl_num;

    struct option ** in_opts;
    int * in_opts_len;

} data;
data input;

int filter(const struct dirent *dir);
void free_func(void);

void p_finding(int argc, char * argv[]);
void where_i_am();
int is_really_dir(char*);
int check(const char*, const struct stat *, int);

int main(int argc, char * argv[]){

    input.and_ = 1;
    input.or_ = 0;
    input.not_ = 0;
    input.directory = NULL;
    input.libraries = NULL;
    input.handler = NULL;
    input.options = NULL;
    input.pl_num = NULL;
    input.in_opts = NULL;
    input.in_opts_len = NULL;
    atexit(free_func);

    char *deb = getenv("LAB1DEBUG");
    if (deb != NULL && deb[0] == '1') //проверка пользовательской переменной окружения
    {
        debug = 1;
        printf("printing debug information\n");
        opterr = 1;
    }

    p_finding(argc, argv);
    if (input.libraries == NULL){
        //then
        where_i_am();
    }

    struct dirent **name_list;
    input.plugin_amount = scandir(input.libraries, &name_list, filter, alphasort);
    if (input.plugin_amount == -1){
        perror("scandir");
        exit(EXIT_FAILURE);
    }
    if (input.plugin_amount == 0)
    {
        printf("error: no plugins are finded\n");
        exit(EXIT_FAILURE);
    }
    char** plugins;
    plugins = (char **)malloc(sizeof(char *) * input.plugin_amount);
    int lpath_len = strlen(input.libraries);

    input.directory = strdup(argv[argc - 1]);
    int only_printing = 1;
    if (is_really_dir(input.directory) != 0 && (strcmp(input.directory, input.libraries) != 0) && argc > 1){
        only_printing = 0;
    }

    for (int i = 0; i < input.plugin_amount; i++) 
    {
        int len = strlen(name_list[i]->d_name); 
        plugins[i] = (char *)malloc(sizeof(char) * (len + 1 + lpath_len + 1));
        strcpy(plugins[i], input.libraries);
        plugins[i][lpath_len] = '/';
        strcpy(plugins[i] + lpath_len + 1, name_list[i]->d_name);
        if (debug) {
            printf("debug: plugin: %s\n", name_list[i]->d_name);
            printf("path = %s\n", plugins[i]);
        }
        void *dl = dlopen(plugins[i], RTLD_LAZY | RTLD_GLOBAL);
        if (!dl) {
            printf("error: in dlopen() %s\n", dlerror());
            continue;
        }
        void *func = dlsym(dl, "plugin_get_info");
        if (!func) {
            printf("error: plugin_get_info is not found\n");
            continue;
        }
        struct plugin_info inf = {0};
        typedef int (*plugin_inf_t)(struct plugin_info *);
        plugin_inf_t plugin_ginfo = (plugin_inf_t)func;
        int res = plugin_ginfo(&inf);
        if (only_printing){
            printf("plugin #%d\n", i);
            printf("author: %s\n", inf.plugin_author);
            printf("purpose: %s\n", inf.plugin_purpose);
            printf("key(s):\n");
            for (int k = 0; k < inf.sup_opts_len; k++)
            {
                printf("\t--%s\n", inf.sup_opts[k].opt.name);
                printf("\t\tuse to %s\n", inf.sup_opts[k].opt_descr);
            }
        }
        else {
            input.option_amount += inf.sup_opts_len;
        }
        free(name_list[i]); 
        dlclose(dl);
    }
    free(name_list);
    if (only_printing){
        for (int i = 0; i < input.plugin_amount; i++){
            free(plugins[i]);
        }
        free(plugins);
        input.plugin_amount = 0;
        exit(EXIT_SUCCESS);
    }
    if (input.option_amount == 0){
        printf("error: really strange\n");
        exit(EXIT_FAILURE);
    }


    input.options = (struct option*)malloc(sizeof(struct option) * (input.option_amount + 1));
    input.pl_num = (int*)malloc(sizeof(int) * input.option_amount);
    input.handler = (void**)malloc(sizeof(void*) * input.plugin_amount);
    int cur_size = 0;
    for (int i = 0; i < input.plugin_amount; i++) {
        input.handler[i] = dlopen(plugins[i], RTLD_LAZY | RTLD_GLOBAL);
        if (!input.handler[i]) {
            printf("error: in dlopen() %s\n", dlerror());
            continue;
        }
        void *func = dlsym(input.handler[i], "plugin_get_info");
        if (!func) {
            printf("error: function plugin_get_info is not found\n");
            continue;
        }
        struct plugin_info inf = {0};
        typedef int (*plugin_inf_t)(struct plugin_info *);
        plugin_inf_t plugin_ginfo = (plugin_inf_t)func;
        int res = plugin_ginfo(&inf);
        for (int j = 0; j < inf.sup_opts_len; j++){
            input.options[cur_size] = inf.sup_opts[j].opt;
            input.pl_num[cur_size] = i;
            cur_size++;
        }
    }
    input.options[input.option_amount].name = NULL;
    input.options[input.option_amount].has_arg = 0;
    input.options[input.option_amount].flag = NULL;
    input.options[input.option_amount].val = 0;
    if (debug){
        printf("debug: collected struct option\n");
        for (int i = 0; i < input.option_amount; i++){
            printf("\tkey = %s, arg = %d, flag = %p, val = %d\n", input.options[i].name, input.options[i].has_arg, input.options[i].flag == NULL ? 0 : input.options[i].flag, input.options[i].val);
        }
    }
    for (int i = 0; i < input.plugin_amount; i++){
        free(plugins[i]);
    }
    free(plugins);
    input.in_opts = (struct option **)malloc(sizeof(struct option *) * input.plugin_amount);
    input.in_opts_len = (int*)malloc(sizeof(int) * input.plugin_amount);
    for (int i = 0; i < input.plugin_amount; i++){
        input.in_opts[i] = NULL;
        input.in_opts_len[i] = 0;
    }

    int n, index;
    while ((n = getopt_long(argc, argv, "P:AONvh", input.options, &index)) != -1)
    {
        switch (n)
        {
        case 0:
            if (input.options[index].flag != NULL){
                printf("error: double key\n");
                exit(EXIT_FAILURE);
            }
            if (input.options[index].has_arg == 1)
            {
                input.options[index].flag = (int*)optarg;
            }   
            input.in_opts_len[input.pl_num[index]]++;
            input.in_opts[input.pl_num[index]] = (struct option *)realloc(input.in_opts[input.pl_num[index]], sizeof(struct option) * input.in_opts_len[input.pl_num[index]]);
            input.in_opts[input.pl_num[index]][input.in_opts_len[input.pl_num[index]] - 1] = input.options[index];         
            break;
        case 1:
            printf("error: double key\n");
            exit(EXIT_FAILURE);
        case 'v':
            break;
        case 'h':
            break;
        case 'O':
            input.or_ = 1;
            input.and_ = 0;
            break;
        case 'A':
            break;
        case 'N':
            input.not_ = 1;
            break;
        case 'P':
            break;
        default:
            fprintf(stderr, "Option input error\n");
            exit(EXIT_FAILURE);
        }
    }


    /*for (int i = 0; i < input.plugin_amount; i++){
        printf("plugin #%d\n", i);
        for (int j = 0; j < input.in_opts_len[i]; j++){
            printf("\t%s, %s\n", input.in_opts[i][j].name, (char*)input.in_opts[i][j].flag);
        }
    }*/

    if (-1 == ftw(input.directory, check, 40)){
        perror("ftw");
        exit(EXIT_FAILURE);
    }

}

int filter(const struct dirent *dir) //для scandir (искать файлы, которые содержат .so)
{
    int i = 0;
    while (dir->d_name[i] != '\0') {
        if (dir->d_name[i] == '.') {
            if (dir->d_name[i + 1] == 's')
                if (dir->d_name[i + 2] == 'o')
                    return 1;
        }
        i++;
    }
    return 0;
}

void free_func(void){
    if (input.directory) free(input.directory);
    if (input.libraries) free(input.libraries);
    for (int i = 0; i < input.plugin_amount; i++){
        dlclose(input.handler[i]);
    }
    if (input.handler) free(input.handler);
    if (input.options) free(input.options);
    if (input.pl_num) free(input.pl_num);
    for (int i = 0; i < input.plugin_amount; i++){
        if (input.in_opts[i]) free(input.in_opts[i]);
    }
    if (input.in_opts) free(input.in_opts);
    if (input.in_opts_len) free(input.in_opts_len);
}

void p_finding(int argc, char * argv[]){
    for (int i = 0; i < argc - 1; i++){
        if (argv[i][0] == '-' && argv[i][1] == 'P'){
            int yes = is_really_dir(argv[i+1]);
            if (yes == 1){
                input.libraries = strdup(argv[i+1]);
            }  
            else{
                printf("error: not a dir\n");
                exit(EXIT_FAILURE);
            }
            break;
        }
    }
    for (int i = 0; i < argc; i++){
        if (argv[i][0] == '-' && argv[i][1] == 'v'){
            printf("version: 1.7.3\nauthor: Rizaev Nikita Sergeevich\ngroup number:N3250\n");
            exit(0);
        }
    }
    for (int i = 0; i < argc; i++){
        if (strcmp(argv[i], "-h") == 0){
            printf("Опции командной строки, поддерживаемые плагином:\n");
            printf("-P dir - Каталог с плагинами\n");
            printf("-A -> Объединение опций плагинов с помощью операции «И» (действует по умолчанию).\n");
            printf("-O - Объединение опций плагинов с помощью операции «ИЛИ»\n");
            printf("-N - Инвертирование условия поиска (после объединения опций плагинов с помощью -A или -O)\n");
            printf("-h - Вывод справки по опциям\n");
            printf("-v - Вывод версии программы и информации о программе (ФИО исполнителя, номер группы, номер варианта лабораторной)\n");
            exit(0);
        }
    }
    return;
}

void where_i_am(){
    struct stat sb;
    ssize_t size;

    if (stat("/proc/self/exe", &sb) == -1) {
        perror("stat");
        exit(EXIT_FAILURE);
    }
    input.libraries = (char*)malloc(sb.st_size + 1);
    if (input.libraries == NULL) {
        perror("malloc");
        exit(EXIT_FAILURE);
    }
    size = readlink("/proc/self/exe", input.libraries, sb.st_size + 1);
    if (size == -1) {
        perror("readlink");
        exit(EXIT_FAILURE);
    }
    input.libraries[size] = '\0';
    for (int i = strlen(input.libraries) - 1;; i--){
        if (input.libraries[i] == '/'){
            input.libraries = (char*)realloc(input.libraries, i * sizeof(char) + 1);
            input.libraries[i] = '\0';
            return;
        }
    }
    return;
}

int is_really_dir(char* path){
    int yes = 0;
    struct stat st;
    if (stat(path, &st) == -1){
        return 0;
    }
    if ((st.st_mode & S_IFMT) == S_IFDIR){
        return 1;
    }
    if ((st.st_mode & S_IFMT) == S_IFREG){
        return 2;
    }
    return 0;
}

int check(const char* fpath, const struct stat * sb, int type){
    if (type != FTW_F){
        return 0;
    }
    int result = 0;
    for (int i = 0; i < input.plugin_amount; i++){
        if (input.in_opts[i] == NULL){
            continue;
        }
        void* func = dlsym(input.handler[i], "plugin_process_file");
        if (!func) {
            printf("error: function plugin_get_info is not found\n");
            continue;
        }
        typedef int (*plugin_function)(const char *, struct option *, size_t);
        plugin_function function = (plugin_function)func;
        int res = function(fpath, input.in_opts[i], input.in_opts_len[i]);
        if (res == -1){
            printf("error mistake accured in plugin\n");
            exit(EXIT_FAILURE);
        }
        result += res;
        if (debug){
            printf("\tfile: %s, plugin#%d, res = %s\n", fpath, i, res ? "false" : "true");
        }
    }
    int size = 0;
    for (int i = 0; i < input.plugin_amount; i++){
        size += input.in_opts_len[i];
    }
    if (input.not_ == 0){
        if (input.and_ == 1){
            if (result == 0)
                printf("%s\n", fpath);
        }
        else {
            if (result < size)
                printf("%s\n", fpath);
        }
    }
    else {
        if (input.and_ == 1){
            if (result != 0)
                printf("%s\n", fpath);
        }
        else {
            if (result == size)
                printf("%s\n", fpath);
        }
    }
    return 0;
}
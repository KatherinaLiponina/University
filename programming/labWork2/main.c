#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h> 
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>
#include <sys/wait.h>
#include <time.h>
#include <poll.h>
#include <netinet/in.h> //sockaddr_in
#include <arpa/inet.h>   //getsockname
#include "linkedlist.h"
#include <pthread.h>
#include <syslog.h>

#define PORT "1255" //значения по умолчанию
#define LOG_BY_DEF "/tmp/lab2.log"
#define MEMORY "/tmp/numbers.txt"
#define TRUE 1

typedef struct {
    int wait_time;
    char* log_file;
    char* adress;
    char* port;
    int demon;
} input_data; 
typedef struct {
    time_t time_sec;
    int succeed;
    pthread_mutex_t suc_lock;
    int failed;
    pthread_mutex_t fa_lock;
} statistic;
statistic statt;
int fd; //файловый дескриптор лог-файла
List_numbers * start;
pthread_mutex_t st_lock;
FILE * stream; //хранилище
char* debug;
int wait_time;

void die(const char*);
int init_server(input_data);
void* process_request(void* client);
void what_time_is_it(int fd);
void sig_handler(int sig);
void freedom();
void demonize();

int main(int argc, char* argv[]){
    input_data data;
    data.wait_time = 0;
    data.log_file = LOG_BY_DEF;
    data.adress = NULL;
    data.port = PORT;
    data.demon = 0;

    struct sigaction sa;
    sa.sa_flags = 0;
    sa.sa_handler = sig_handler;
    sigemptyset(&sa.sa_mask);
    sigaddset(&sa.sa_mask, SIGINT);
    sigaddset(&sa.sa_mask, SIGTERM);
    sigaddset(&sa.sa_mask, SIGQUIT);
    sigaddset(&sa.sa_mask, SIGUSR1);
    if (-1 == sigaction(SIGINT, &sa, NULL)) die("sigaction()");
    if (-1 == sigaction(SIGTERM, &sa, NULL)) die("sigaction()");
    if (-1 == sigaction(SIGQUIT, &sa, NULL)) die("sigaction()");
    if (-1 == sigaction(SIGUSR1, &sa, NULL)) die("sigaction()");

    statt.failed = 0;
    statt.succeed = 0;

    char* lab_wait = getenv("LAB2WAIT");
    if (lab_wait) data.wait_time = atoi(lab_wait);
    char* lab_log = getenv("LAB2LOGFILE");
    if (lab_log) data.log_file = lab_log;
    char* lab_addr = getenv("LAB2ADDR");
    if (lab_addr) data.adress = lab_addr;
    char* lab_port = getenv("LAB2PORT");
    if (lab_port) data.port = lab_port;
    debug = getenv("LAB2DEBUG");

    if (argc != 1){
        while(TRUE){
            int n = getopt(argc, argv, "dw:l:vha:p:");
            if (n == 'd'){
                demonize();
                data.demon = 1;
            }
            else if (n == 'w'){
                data.wait_time = atoi(optarg);
            }
            else if (n == 'l'){
                data.log_file = strdup(optarg);
            }
            else if (n == 'v'){
                printf("Сервер\n");
                printf("Версия 1.1.1\n");
                exit(EXIT_SUCCESS);
            }
            else if (n == 'h'){
                printf("Используйте:\n");
                printf("-w N для установки задержки\n");
                printf("-d демонизировать сервер\n");
                printf("-l path чтобы задать путь для лог-файла\n");
                printf("-v чтобы получить версию программы\n");
                printf("-h --вы находитесь здесь--\n");
                printf("-a ip чтобы установить IP\n");
                printf("-p port чтобы установить порт\n");
                exit(EXIT_SUCCESS);
            }
            else if (n == 'a'){
                data.adress = strdup(optarg);
            }
            else if (n == 'p'){
                data.port = strdup(optarg);
                int dt = atoi(data.port);
                if (dt < 1024){
                    printf("Задан неверный порт, будет использоваться порт по умолчанию (1255)\n");
                    data.port = strdup(PORT);
                }
            }
            else if (n == '?'){
                printf("Задана неизвестная опция: %s\n", argv[optind]);
                exit(EXIT_FAILURE);
            }
            else if (n == -1){
                break;
            }
        } 
    }
    
    statt.time_sec = time(NULL);
    fd = open(data.log_file, O_RDWR | O_APPEND | O_CREAT, S_IRWXU);//открытие лог-файла
    if (fd == -1){
        perror("open()");
        fd = open(LOG_BY_DEF, O_RDWR | O_APPEND | O_CREAT, S_IRWXU);
        if (fd == -1){
            die("open()");
        }
    }
    what_time_is_it(fd);
    if (-1 == write(fd, " server starts working\n", 24)) die("write");
    if (data.demon){
        dup(fd);
        dup(fd);
    }

    int sockfd = init_server(data); //инициализация сервера

    stream = fopen(MEMORY, "r+");
    if (!stream){
        start = NULL;
        stream = fopen(MEMORY, "a");
        if (!stream) die("fopen");
    }
    else {
        start = read_file(stream);
    }

    if (pthread_mutex_init(&statt.suc_lock, NULL) == -1) die("pthread_mutex_init");
    if (pthread_mutex_init(&statt.fa_lock, NULL) == -1) die("pthread_mutex_init");
    if (pthread_mutex_init(&st_lock, NULL) == -1) die("pthread_mutex_init");

    wait_time = data.wait_time;

    if (strcmp(data.log_file, LOG_BY_DEF) != 0) free(data.log_file);
    if (strcmp(data.port, PORT) != 0) free(data.port);
    if (data.adress) free(data.adress);

    while(TRUE){
        int clientfd;
        while ((clientfd = accept(sockfd, NULL, NULL)) < 0){
            if (errno == EINTR)
                continue;
            die("accept()");
        }
        what_time_is_it(fd);
        if (-1 == write(fd, " new client conected\n", 22)) die("write");
        pthread_t new_thread;
        if (pthread_create(&new_thread, NULL, process_request, &clientfd) == -1) die("pthread_create");
        if (pthread_detach(new_thread) != 0) die("pthread_detach");
    }

    return 0;
}

int init_server(input_data data){ //сервер запускается
    int sockfd, on = 1; //3
    struct addrinfo *res, *res0, servAddr = {0};
    servAddr.ai_family = AF_INET;
    servAddr.ai_socktype = SOCK_STREAM;
    servAddr.ai_protocol = IPPROTO_TCP;
    servAddr.ai_flags = AI_PASSIVE;
    if(-1 == (getaddrinfo(data.adress, data.port, &servAddr, &res0)))
        die("getaddrinfo()");
    for(res = res0; res; res = res->ai_next) { //функция вернула несколько - пытаемся сделать все функции для какого-нибудь
        if(-1 == (sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol))) {
            perror("socket()");
            continue;
        }
 
        if(-1 == (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, (char*)&on, sizeof(int)))) {
            perror("setsockopt()");
            continue;
        }
 
        if(-1 == (bind(sockfd, res->ai_addr, res->ai_addrlen))) {
            perror("bind");
            continue;
        }
 
        break;
    }
    if(-1 == sockfd){
        printf("error: no adresses allowed\n");
        exit(EXIT_FAILURE);
    }
    freeaddrinfo(res);
    if(-1 == (listen(sockfd, 32)))
        die("listen()");

    if (data.adress == NULL){
        struct sockaddr_in * ad;
        ad = (struct sockaddr_in*) res->ai_addr;
        char buffer[20];
        const char* p = inet_ntop(AF_INET, &ad->sin_addr, buffer, 100);
        printf("Мой адрес: %s\n", buffer);
    }
    else 
        printf("Мой адрес: %s\n", data.adress);
    printf("Мой порт: %s\n", data.port);
    printf("Мой PID: %d\n", getpid());
    printf("Подключайтесь, люди и роботы!\n");
    return sockfd;
}

void die(const char* str){
    perror(str);
    exit(EXIT_FAILURE);
}

#define ADD(len, num) num < 10 ? len+1 : len+2
void what_time_is_it(int fd){ //текущее время запись в лог
    time_t t = time(0);
    struct tm* now = localtime(&t);
    int len = 0;
    char* time = (char*)malloc(sizeof(char) * (19));//DD:MM:YY hh:mm:ss
    memset(time, 0, 18);
    sprintf(time + len, "%d", now->tm_mday);
    len = ADD(len, now->tm_mday);
    time[len++] = ':';
    sprintf(time + len, "%d", now->tm_mon + 1);
    len = ADD(len, now->tm_mon);
    time[len++] = ':';
    sprintf(time + len, "%d", now->tm_year - 100);
    len = ADD(len, now->tm_year - 100);
    time[len++] = ' ';
    sprintf(time + len, "%d", now->tm_hour);
    len = ADD(len, now->tm_hour);
    time[len++] = ':';
    sprintf(time + len, "%d", now->tm_min);
    len = ADD(len, now->tm_min);
    time[len++] = ':';
    sprintf(time + len, "%d", now->tm_sec);
    len = ADD(len, now->tm_sec);
    time[len++] = ' ';
    time[len++] = '\0';
    if (-1 == write(fd, time, 17)){
        die("write()");
    }
    free(time);
}

void sig_handler(int sig){ //поведение при завершении
    if (sig == SIGINT || sig == SIGTERM || sig == SIGQUIT){
        what_time_is_it(fd);
        if (sig == SIGINT){
            if (-1 == write(fd, " killed by signal SIGINT\n", 26)){
                exit(EXIT_FAILURE);
            }
        }
        else if (sig == SIGTERM){
            if (-1 == write(fd, " killed by signal SIGTERM\n", 26)){
                exit(EXIT_FAILURE);
            }
        }
        else if (sig == SIGQUIT){
            if (-1 == write(fd, " killed by signal SIGQUIT\n", 26)){
                exit(EXIT_FAILURE);
            }
        }
        freedom();
        exit(EXIT_SUCCESS);
    }
    else if (sig == SIGUSR1){
        time_t now = time(NULL);
        fprintf(stderr, "Server works %ld sec\n", now - statt.time_sec);
        fprintf(stderr, "Requests proceed succesfully: %d\n", statt.succeed);
        fprintf(stderr, "Requests failed: %d\n", statt.failed);
    }
}

#define GET "GET"
#define SET "SET"
#define DELETE "DELETE"
char* take_message(int clientfd);
void send_random(int clientfd, int N);
int process_string(int clientfd, char* order, char* message);
void * process_request(void * client){
    if (wait_time){
        sleep(wait_time);
    }
    int clientfd;
    clientfd = *((int*)client);
    int i = 0, length = 0;
    char* message = take_message(clientfd);\
    if (!message){
        if (-1 == write(clientfd, "ERROR 1\n", 9)) die("write");
        return NULL;
    }
    length = strlen(message);
    int LF = 0;
    for (int j = 0; j < length; j++){
        if (message[j] == '\n') LF++;
    }
    char* order = NULL;
    if (LF >= 1){ //обе строчки получены
        while(i < length && message[i] != '\n'){
            order = add_sym(order, message[i]);
            i++;
        }
        order = add_sym(order, '\0');
        if (message[i] != '\n') {
            if (-1 == write(clientfd, "ERROR 1\n", 9)) die("write"); //ERROR 1 - wrong command
            if (debug) printf("wrong command recieved\n");
            goto fail;
        }
        i++;
        if (LF == 1){
            free(message);
            message = take_message(clientfd);
            if (!message){
                if (-1 == write(clientfd, "ERROR 1\n", 9)) die("write");
                return NULL;
            }
            if (strncmp(message, "\n", 1) == 0){
                if (-1 == write(clientfd, "ERROR 1\n", 9)) die("write");
                return NULL;
            }
            i = 0;
            length = strlen(message);
        }
        if (strcmp(order, GET) == 0){
            char* Num = NULL;
            while (i < length && message[i] != '\n'){
                if (message[i] == ' ' || message[i] == '\t'){
                    i++; 
                    continue;
                }
                Num = add_sym(Num, message[i]);
                i++;
            }
            if (!Num){
                if (-1 == write(clientfd, "ERROR 1\n", 9)) die("write");
                goto fail;
            }
            int N = atoi(Num);
            free(Num);
            if (N <= 0){
                if (-1 == write(clientfd, "ERROR 2\n", 9)) die("write"); //ERROR 2 - wrong N from GET
                if (debug) printf("wrong N for GET\n");
                goto fail;
            }
            if (start == NULL){
                if (-1 == write(clientfd, "ERROR 7\n", 9)) die("write"); //ERROR 8 - no doubles
                if (debug) printf("no doubles for GET\n");
                goto fail;
            }
            send_random(clientfd, N);
            goto success;
        }
        else if (strcmp(order, DELETE) == 0 || strcmp(order, SET) == 0){
            int k = process_string(clientfd, order, message + i);
            if (k == 0){
                if (-1 == write(clientfd, "OK\n", 3)) die("write");
                goto success;
            }
            else if (k == 1){
                if (-1 == write(clientfd, "ERROR 5\n", 9)) die("write");
                goto fail;
            }
            else {
                if (-1 == write(clientfd, "ERROR 6\n", 9)) die("write");
                goto fail;
            }
        }
        else {
            if (-1 == write(clientfd, "ERROR 3\n", 9)) die("write"); //ERROR 3 - unknown command
            if (debug) printf("unknown command\n");
            goto fail;
        }
    }
    else {
        if (-1 == write(clientfd, "ERROR 1\n", 9)) die("write");
        if (debug) printf("wrong command recieved\n");
        goto fail;
    }
    
success:
    free(message);
    free(order);
    close(clientfd);
    pthread_mutex_lock(&statt.suc_lock);
    statt.succeed++;
    pthread_mutex_unlock(&statt.suc_lock);
    what_time_is_it(fd);
    if (-1 == write(fd, " request was proceeded succesfully\n", 36)) die("write");
    //free(client);
    return NULL;

fail:
    if (message) free(message);
    if (order) free(order);
    close(clientfd);
    //free(client);
    pthread_mutex_lock(&statt.fa_lock);  
    statt.failed++;
    pthread_mutex_unlock(&statt.fa_lock);
    what_time_is_it(fd);
    if (-1 == write(fd, " request was failed\n", 21)) die("write");
    return NULL;
}

char* take_message(int clientfd){
    char buf[129];
    char* message = NULL;
    int length = 0, n;
    while ((n = recv(clientfd, buf, 128, 0)) == 128){
        length += n;
        message = (char*)realloc(message, sizeof(char) * (length + 1));
        for (int j = 0; j < n; j++){
            message[length - n + j] = buf[j];
        }
        message[length] = '\0';
    }
    if (n == 0) {
        return message;
    }
    if (n == -1) die("recv");
    length += n;
    message = (char*)realloc(message, sizeof(char) * (length + 1));
    for (int j = 0; j < n; j++){
        message[length - n + j] = buf[j];
    }
    message[length] = '\0';
    return message;
}

int process_string(int clientfd, char* order, char* message){
    int i = 0, length = strlen(message), f = 0;
    char* value = NULL;
    while (message[i] != '\0'){
        if (message[i] == ' ' || message[i] == '\t' || message[i] == '\n'){
            if (value){
                if (strcmp(order, DELETE) == 0){
                    pthread_mutex_lock(&st_lock);
                    int c = delete_n(start, value);
                    if (c == -1) { 
                        f++; 
                        //if (-1 == write(clientfd, "ERROR 5\n", 9)) die("write"); 
                        if (debug) printf("not exist (DELETE)\n");
                    } //ERROR 5 - not exist
                    else if (c == 0) {       
                        //if (-1 == write(clientfd, "OK\n", 3)) die("write"); 
                        if (debug) printf("deleted %s\n", value); 
                    }
                    else {
                        List_numbers * t = start;
                        start = start->next;
                        clean_n(t);
                        //if (-1 == write(clientfd, "OK\n", 3)) die("write");
                        if (debug) printf("deleted %s\n", value);
                    }
                    pthread_mutex_unlock(&st_lock);
                }
                else {
                    double check;
                    char *end;
                    check = strtod(value, &end);
                    if (*end != '\0' && *end != '\n' && *end != ' '){
                        //if (-1 == write(clientfd, "ERROR 7\n", 9)) {die("write"); }//ERROR 7 - not a double
                        f++;
                        if (debug) printf("not a double %s\n", value);
                        goto end;
                        printf("not here\n");
                    }
                    int tmp = find_in_list(start, value);
                    if (tmp == 0){
                        //if (-1 == write(clientfd, "ERROR 6\n", 9)) { die("write");  }
                        f++;
                        if (debug) printf("already exist (SET)\n");
                        //ERROR 6 - already exist
                    }
                    else {
                        pthread_mutex_lock(&st_lock);
                        if (!start){
                            start = create_n();
                            start->number = strdup(value);
                        }
                        else{
                            add_n(start, value);
                        }
                        //if (-1 == write(clientfd, "OK\n", 3)) die("write");
                        if (debug) printf("added %s\n", value);
                        pthread_mutex_unlock(&st_lock);
                    }
                }
                end:
                free(value);
                value = NULL;
                //print_list(start);
            }
            i++;
        }
        else {
            value = add_sym(value, message[i]);
            i++;
        }
    }
    if (f == 1) return 1;
    if (f > 1) return 2;
    return 0;
}

void freedom(){
    close(fd);
    freopen(MEMORY, "w", stream);
    write_file(stream, start);
    fclose(stream);
    clean_rev(start);
    //if (debug) free(debug);
}

void send_random(int clientfd, int N){
    int length = 0;
    List_numbers * tmp = start;
    while(tmp){
        length++;
        tmp = tmp->next;
    }
    for (int i = 0; i < N; i++){
        tmp = start;
        int k = rand() % length;
        for (int j = 0; j < k; j++){
            tmp = tmp->next;
        }
        if (-1 == write(clientfd, tmp->number, strlen(tmp->number))) die("write");
        if (-1 == write(clientfd, " ", 1)) die("write");
    }
    if (debug) printf("printed %d numbers\n", N);
}

void demonize(){
    pid_t pid = fork();
    if (pid < 0){
        die("fork");
    }
    else if (pid > 0){
        exit(0);
    }
    if (setsid() < 0) die("setsid");
    struct sigaction sa;
    sa.sa_handler = SIG_IGN;
    sa.sa_flags = 0;
    if (-1 == sigaction(SIGHUP, &sa, NULL)) die("sigaction()");
    pid = fork();
    if (pid < 0){
        die("fork");
    }
    else if (pid > 0){
        exit(0);
    }
    umask(0);
    //chdir("/");
    int x;
    for (x = sysconf(_SC_OPEN_MAX); x>= 0; x--){
        close(x);
    }
}

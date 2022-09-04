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
#include <netinet/in.h> //sockaddr_in
#include <arpa/inet.h>
#define TRUE 1
typedef struct {
    char* adress;
    char* port;
    char* order;
    char* value;
} input_data; 
void die(const char*);

int main(int argc, char* argv[]){
    input_data data;
    data.adress = NULL;
    data.port = NULL;
    data.order = NULL;
    data.value = NULL;
    if (argc != 1){
        while(TRUE){
            int n = getopt(argc, argv, "vha:p:");
            if (n == 'v'){
                printf("Клиент\n");
                printf("Версия 1.1.1\n");
                exit(EXIT_SUCCESS);
            }
            else if (n == 'h'){
                printf("Используйте:\n");
                printf("-a ip чтобы установить IP\n");
                printf("-p port чтобы установить порт\n");
                printf("-h --вы находитесь здесь--\n");
                printf("-v чтобы получить версию программы\n");
                printf("-o чтобы задать команду\n");
                printf("-d чтобы задать аргумент команды");
                exit(EXIT_SUCCESS);
            }
            else if (n == 'a'){
                data.adress = strdup(optarg);
            }
            else if (n == 'p'){
                data.port = strdup(optarg);
                int dt = atoi(data.port);
                if (dt < 1024){
                    printf("Задан неверный порт\n");
                    return -1;
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
    if (!data.adress || !data.port){
        printf("Необходимо задать адрес и порт сервера\n");
        printf("Запрос к серверу тоже, очевидно, необходим\n");
        exit(1);
    }

    struct addrinfo * ailist, *aip, hint;
    int sockfd = -1, err;
    memset(&hint, 0, sizeof(hint));
    hint.ai_socktype = SOCK_STREAM;
    hint.ai_canonname = NULL;
    hint.ai_addr = NULL;
    hint.ai_next = NULL;
    if ((err = getaddrinfo(data.adress, data.port, &hint, &ailist)) != 0) die("getaddrinfo");
    for (aip = ailist; aip != NULL; aip = aip->ai_next){
        sockfd = socket(aip->ai_family, SOCK_STREAM, 0);
        if (sockfd < 0) continue;
        if (connect(sockfd, aip->ai_addr, aip->ai_addrlen) == 0){
            break;
        }
        else{
            continue;
        }
    }
    freeaddrinfo(ailist);
    if (sockfd == -1){
        printf("Не удалось соединиться\n");
    }
    size_t k;
    char* buf = NULL;
    printf("Введите команду: ");
    getline(&buf, &k, stdin);
    int len = strlen(buf);
    int space = -1;
    for (int i = 0; i < len; i++){
        if (buf[i] == ' '){
            space = i;
            break;
        }
    }
    if (space == -1){
        printf("Неверная команда\n");
        close(sockfd);
        exit(1);
    }
    data.order = (char*)malloc(sizeof(char) * (space + 2));
    for (int i = 0; i < space; i++){
        data.order[i] = buf[i];
    }
    data.order[space] = '\n';
    data.order[space + 1] = '\0';

    data.value = (char*)malloc(sizeof(char) * (len - space + 1));
    for (int i = space + 1; i < len; i++){
        data.value[i - space - 1] = buf[i];
    }
    free(buf);
    data.value[len - space - 1] = '\n';
    data.value[len - space] = '\0';
    int len_order = strlen(data.order);
    int len_value = strlen(data.value);
    if (-1 == write(sockfd, data.order, len_order)) die("write");
    if(-1 == write(sockfd, data.value, len_value)) die("write");
    free(data.order);
    free(data.value);
    free(data.adress);
    free(data.port);
    int n, length = 0;
    char buff[129];
    char *message = NULL;
    while((n = recv(sockfd, buff, 128, 0)) > 0){
        length += n;
        message = (char*)realloc(message, sizeof(char) * (length + 1));
        for (int i = 0; i < n; i++){
            message[length - n + i] = buff[i];
        }
        message[length] = '\0';
    }
    printf("%s\n", message);
    free(message);

    return 0;
}

void die(const char* str){
    perror(str);
    exit(EXIT_FAILURE);
}

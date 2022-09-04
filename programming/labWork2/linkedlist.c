#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "linkedlist.h"

List_numbers* create_n(){
    List_numbers * new = (List_numbers*)malloc(sizeof(List_numbers));
    if (!new) return NULL;
    new->next = NULL;
    new->number = NULL;
    return new;
}
int add_n(List_numbers* start, char* num){
    List_numbers * new = create_n();
    if (!new) return -1;
    new->number = strdup(num);
    List_numbers * temp = start;
    while (temp->next) temp = temp->next;
    temp->next = new;
    return 0;
}
void clean_rev(List_numbers * l){
    if (l) {
        if (l->number) free(l->number);
        if (l->next) clean_rev(l->next);
        free(l);
        l = NULL;
    }
}
void clean_n(List_numbers * l){
    if (l){
        if (l->number) free(l->number);
        free(l);
    }   
}
int delete_n(List_numbers* start, char* num){
    List_numbers * temp = start;
    List_numbers * previous = start;
    while (temp){
        if (strcmp(temp->number, num) == 0){
            if (temp == previous){
                return 1;
            }
            else {
                previous->next = temp->next;
                clean_n(temp);
                return 0;
            }
        }
        previous = temp;
        temp = temp->next;
    }
    return -1;
}

char* add_sym(char* str, char c){
    char* temp = NULL;
    int len = 0;
    if (str != NULL){
        len = strlen(str);
    } 
    temp = (char*)realloc(str, (len + 2) * sizeof(char));
    if (!temp){
        perror("realloc");
        exit(1);
    }
    temp[len] = c;
    temp[len + 1] = '\0';
    return temp;
}

int find_in_list(List_numbers * start, char* key){ //поиск в списке по ключу значение
    List_numbers * l = start;
    while (l){
        if (strcmp(l->number, key) == 0){
            return 0;
        }
        l = l->next;
    }
    return -1;
}

void print_list(List_numbers * start){
    List_numbers * l = start;
    int i = 0;
    while(l){
        i++;
        printf("%d. %s\n", i, l->number);
        l = l->next;
    }
}

List_numbers * read_file(FILE * stream){
    size_t n;
    char* buf = NULL;
    List_numbers * start = NULL;
    while((n = getdelim(&buf, &n, ' ', stream)) != -1){
        n = strlen(buf);
        buf = (char*)realloc(buf, sizeof(char) * n);
        buf[n-1] = '\0'; 
        if (!start){
            start = create_n();
            start->number = strdup(buf);
        }
        else {
            add_n(start, buf);
        }
        free(buf);
        buf = NULL;
    }
    free(buf);
    return start;
}

void write_file(FILE * stream, List_numbers * start){
    List_numbers * tmp = start;
    if (!start) return;
    while(tmp){
        if (-1 == fwrite(tmp->number, sizeof(char), strlen(tmp->number), stream)) {
            perror("fwrite");
            exit(-1);
        }
        if (tmp->next != NULL) {
            if (-1 == fwrite(" ", sizeof(char), 1, stream)){
                perror("fwrite");
                exit(-1);
            }
        }
        else {
            if (-1 == fwrite("\n", sizeof(char), 1, stream)){
                perror("fwrite");
                exit(-1);
            }
        }
        tmp = tmp->next;
    }
}
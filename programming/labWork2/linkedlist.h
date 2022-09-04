#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>
typedef struct List_numbers{
    struct List_numbers * next;
    char* number;
} List_numbers;

List_numbers* create_n();
int add_n(List_numbers* start, char* num);
void clean_rev(List_numbers * l);
void clean_n(List_numbers * l);
int delete_n(List_numbers* start, char* num);
char* add_sym(char* str, char c);
int find_in_list(List_numbers * start, char* key);
void print_list(List_numbers * start);
List_numbers * read_file(FILE * stream);
void write_file(FILE * stream, List_numbers * start);
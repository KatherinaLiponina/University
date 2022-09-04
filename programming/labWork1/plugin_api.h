#ifndef _PLUGIN_API_H
#define _PLUGIN_API_H

#include <getopt.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#define PLUGIN_API_VERSION  1

/*
    Структура, описывающая опцию, поддерживаемую плагином.
*/
struct plugin_option {
    /* Опция в формате, поддерживаемом getopt_long (man 3 getopt_long). */
    struct option opt;
    /* Описание опции, которое предоставляет плагин. */
    const char *opt_descr;
};

/*
    Структура, содержащая информацию о плагине.
*/
struct plugin_info {
    /* Назначение плагина */
    const char *plugin_purpose;
    /* Автор плагина, например "Иванов Иван Иванович, N32xx" */
    const char *plugin_author;
    /* Длина списка опций */
    size_t sup_opts_len;
    /* Список опций, поддерживаемых плагином */
    struct plugin_option *sup_opts;
};


int plugin_get_info(struct plugin_info* ppi);
/*
    plugin_get_info()
    
    Функция, позволяющая получить информацию о плагине.
    
    Аргументы:
        ppi - адрес структуры, которую заполняет информацией плагин.
        
    Возвращаемое значение:
          0 - в случае успеха,
        < 0 - в случае неудачи (в этом случае продолжать работу с этим плагином нельзя).
*/



int plugin_process_file(const char *fname,
        struct option in_opts[],
        size_t in_opts_len);
/*
    plugin_process_file()
    
    Фунция, позволяющая выяснить, отвечает ли файл заданным критериям.
    
    Аргументы:
        fname - путь к файлу (полный или относительный), который проверяется на
            соответствие критериям, заданным с помощью массива in_opts.

        in_opts - список опций (критериев поиска), которые передаются плагину.
            struct option {
               const char *name;
               int         has_arg;
               int        *flag;
               int         val;
            };
            Поле name используется для передачи имени опции, поле flag - для передачи
            значения опции (в виде строки). Если у опции есть аргумент, поле has_arg
            устанавливается в ненулевое значение. Поле val не используется.
           
        in_opts_len - длина списка опций.
                    
    Возвращаемое значение:
          0 - файл отвечает заданным критериям,
        > 0 - файл НЕ отвечает заданным критериям,
        < 0 - в процессе работы возникла ошибка
        
    В случае, если произошла ошибка, переменная errno должна устанавливаться 
    в соответствующее значение.
*/
        
#endif

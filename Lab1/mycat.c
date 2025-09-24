#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
//#include <getopt.h>

extern int optind;

#define FLAG_N (1 << 0)//нумерация всех строк
#define FLAG_B (1 << 1)//нумерация только не пустых строк
#define FLAG_E (1 << 2)//маркировка конца каждой строки

void process_file(FILE* file, int flags);

int main(int argc, char* argv[]){
    int flags = 0;
    int opt;
//    printf("%3d\n", 14);
   // printf("%d\n", FLAG_E | FLAG_N | FLAG_B);
    while((opt = getopt(argc, argv, "nbE")) != -1){
        switch(opt){
            case 'n':
                flags |= FLAG_N;
                break;
            case 'b':
                flags |= FLAG_B;
                break;
            case 'E':
                flags |= FLAG_E;
                break;
            default:
                printf("что за флаг, я такого не знаю\n");
                return EXIT_FAILURE;
        }
    }
    if (argc == optind){
        perror("нет файлов на вывод\n");
    }
    //обработка файлов
    else {
        for (int i = optind; i < argc; i++){
            FILE* file;
            file = fopen(argv[i], "r");
            if (file == NULL){
                perror(argv[i]);
                continue;
            }
            process_file(file, flags);
        } 
    }
    return 0;
}

void process_file(FILE *file, int flags){    
    ssize_t read;
    size_t len = 0;
    char *line = NULL;
    int line_number = 0;
    while((read = getline(&line, &len, file)) != -1){
        int is_empty = (read == 1 && line[0] == '\n');
        
        if (flags & FLAG_B){
            if(!is_empty){
                line_number++;
                printf("%6d\t", line_number);          
            }
        } else if(flags & FLAG_N){
            line_number++;
            printf("%6d\t", line_number);
        }
        
        if (flags & FLAG_E){
            if (read > 0 && line[read - 1] == '\n'){
                line[read - 1] = '\0';
                printf("%s$\n", line);
            } else{
                printf("%s$", line);
            }
        } else{
            printf("%s", line);
        }
    }
    free(line);
}

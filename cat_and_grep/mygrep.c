#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <regex.h>

void grep_file(FILE *file, regex_t *regex); 

int main(int argc, char* argv[]){
    int opt; 
    while ((opt = getopt(argc, argv, "")) != -1){
        perror("Я не буду обрабатывать твой флаг\n");
    }

    if (optind == argc){
        perror("Нет регулярки");
        return EXIT_FAILURE;
    }
    char* pattern = argv[optind];
    optind++;

    regex_t regex;
    int value = regcomp(&regex, pattern, 0);
    if (value != 0){
        perror("Ошибка в регулярном выражении");
        exit(EXIT_FAILURE);
    }

    if (optind >= argc){
        grep_file(stdin, &regex);
    } else {
        for (int i = optind; i < argc; i++){
            FILE *file;
            file = fopen(argv[i], "r");
            if (!file){
                perror(argv[i]);
                continue;
            }
            grep_file(file, &regex);
        }
    }
    regfree(&regex);
    return 0;
}

void grep_file(FILE *file, regex_t *regex){
    ssize_t read;
    size_t len;
    char* line = NULL;
    while((read = getline(&line, &len, file)) != -1){
        int value = regexec(regex, line, 0, NULL, 0);
        if (value == 0) {//есть совпадения
            printf("%s", line);
        }
    }
    free(line);
}   

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>

// проверкa существования файла
int file_exists(const char *filename) {
    return access(filename, F_OK) == 0;
}

//применение символьного режима
int apply_symbolic_mode(const char *mode_str, mode_t *current_mode) {
    mode_t new_mode = *current_mode;
    const char *p = mode_str;
    
    // определяем, для кого применяются изменения
    int users = 0; // битовая маска: 1=user, 2=group, 4=other
    
    // Парсим пользователей до операции
    while (*p && *p != '+' && *p != '-' && *p != '=') {
        switch (*p) {
            case 'u': users |= 1; break;
            case 'g': users |= 2; break;
            case 'o': users |= 4; break;
            case 'a': users |= 7; break; // все
            default:
                fprintf(stderr, "mychmod: неверный символ в указании пользователя: '%c'\n", *p);
                return -1;
        }
        p++;
    }
    
    // Если не указаны пользователи, по умолчанию все
    if (users == 0) {
        users = 7; // все
    }
    
    if (*p == '\0') {
        fprintf(stderr, "mychmod: неверный формат режима: '%s'\n", mode_str);
        return -1;
    }
    
    char operation = *p;
    p++;
    
    if (*p == '\0') {
        fprintf(stderr, "mychmod: отсутствуют права доступа после операции: '%s'\n", mode_str);
        return -1;
    }
    
    // определяем какие права изменяем
    int permissions = 0;
    while (*p) {
        switch (*p) {
            case 'r': permissions |= 4; break;
            case 'w': permissions |= 2; break;
            case 'x': permissions |= 1; break;
            default:
                fprintf(stderr, "mychmod: неверное право доступа: '%c'\n", *p);
                return -1;
        }
        p++;
    }
    
    // Применяем изменения
    if (operation == '+') {
        if (users & 1) new_mode |= (permissions << 6); // user
        if (users & 2) new_mode |= (permissions << 3); // group
        if (users & 4) new_mode |= permissions;        // other
    } else if (operation == '-') {
        if (users & 1) new_mode &= ~(permissions << 6); // user
        if (users & 2) new_mode &= ~(permissions << 3); // group
        if (users & 4) new_mode &= ~permissions;        // other
    }
    *current_mode = new_mode;
    return 0;
}

// Функция для применения числового режима
int apply_numeric_mode(const char *mode_str, mode_t *current_mode) {
    char *endptr;
    long mode_value = strtol(mode_str, &endptr, 8);
    
    if (*endptr != '\0' || mode_value < 0 || mode_value > 0777) {
        fprintf(stderr, "mychmod: неверный числовой режим: '%s'\n", mode_str);
        return -1;
    }
    
    *current_mode = (mode_t)mode_value;
    return 0;
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "mychmod: неверное количество аргументов\n");
        return EXIT_FAILURE;
    }
    
    const char *mode_str = argv[1];
    const char *filename = argv[2];
    
    // проверяем существование файла
    if (!file_exists(filename)) {
        fprintf(stderr, "mychmod: невозможно получить доступ к '%s'\n", filename);
        return EXIT_FAILURE;
    }
    
    // получаем текущие права доступа
    struct stat st;
    if (stat(filename, &st) == -1) {
        perror("mychmod");
        return EXIT_FAILURE;
    }
    
    mode_t current_mode = st.st_mode;
    mode_t new_mode = current_mode;
    
    int result;
    
    // определяем тип режима: символьный или числовой
    if (strchr(mode_str, '+') != NULL || 
        strchr(mode_str, '-') != NULL) {
        // символьный режим
        result = apply_symbolic_mode(mode_str, &new_mode);
    } else {
        // числовой режим
        result = apply_numeric_mode(mode_str, &new_mode);
    }
    
    if (result != 0) {
        return EXIT_FAILURE;
    }
    
    // Применяем новые права доступа
    if (chmod(filename, new_mode) == -1) {
        perror("mychmod");
        return EXIT_FAILURE;
    }
    
    return EXIT_SUCCESS;
}

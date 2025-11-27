#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include <errno.h>

#define NAME_SIZE 256

typedef struct {
    char name[NAME_SIZE];
    mode_t mode;
    off_t size;
    uid_t uid;
    gid_t gid;
} FileHeader;

void print_help() {
    printf("Usage:\n");
    printf("  ./archiver arch_name -i(--input) file1 [file2 ...]  - add files to archive\n");
    printf("  ./archiver arch_name -e(--extract) file1 [file2 ...] - extract files from archive\n");
    printf("  ./archiver arch_name -s(--stat)                     - show archive contents\n");
    printf("  ./archiver -h(--help)                              - show this help\n");
}

int add_file_to_archive(const char *archive_name, const char *filename) {
    int arch_fd, file_fd;
    FileHeader header;
    struct stat file_stat;
    char buffer[4096];
    ssize_t bytes_read;
    
    // получаем информацию о файле
    if (stat(filename, &file_stat) < 0) {
        perror("stat");
        return -1;
    }
    
    // проверяем, что это обычный файл
    if (!S_ISREG(file_stat.st_mode)) {
        fprintf(stderr, "%s is not a regular file\n", filename);
        return -1;
    }
    
    file_fd = open(filename, O_RDONLY);
    if (file_fd < 0) {
        perror("open file");
        return -1;
    }
    
    // открываем архив для записи (добавляем в конец)
    arch_fd = open(archive_name, O_WRONLY | O_CREAT | O_APPEND, 0644);
    if (arch_fd < 0) {
        perror("open archive");
        close(file_fd);
        return -1;
    }
    
    // заполняем заголовок
    memset(&header, 0, sizeof(FileHeader));
    strncpy(header.name, filename, NAME_SIZE - 1);
    header.mode = file_stat.st_mode;
    header.size = file_stat.st_size;
    header.uid = file_stat.st_uid;
    header.gid = file_stat.st_gid;
    
    // записываем заголовок в архив
    if (write(arch_fd, &header, sizeof(FileHeader)) != sizeof(FileHeader)) {
        perror("write header");
        close(file_fd);
        close(arch_fd);
        return -1;
    }
    
    // копируем содержимое файла в архив
    while ((bytes_read = read(file_fd, buffer, sizeof(buffer))) > 0) {
        if (write(arch_fd, buffer, bytes_read) != bytes_read) {
            perror("write data");
            close(file_fd);
            close(arch_fd);
            return -1;
        }
    }
    
    if (bytes_read < 0) {
        perror("read file");
        close(file_fd);
        close(arch_fd);
        return -1;
    }
    
    close(file_fd);
    close(arch_fd);
    
    printf("Added file: %s\n", filename);
    return 0;
}

int extract_file_from_archive(const char *archive_name, const char *filename) {
    int arch_fd, file_fd;
    FileHeader header;
    char buffer[4096];
    ssize_t bytes_read;
    off_t total_read;
    
    // открываем архив для чтения
    arch_fd = open(archive_name, O_RDONLY);
    if (arch_fd < 0) {
        perror("open archive");
        return -1;
    }
    
    // ищем файл в архиве
    while (read(arch_fd, &header, sizeof(FileHeader)) == sizeof(FileHeader)) {
        if (strcmp(header.name, filename) == 0) {
            // нашли нужный файл
            printf("Extracting: %s\n", filename);
            
            // создаем файл с оригинальными атрибутами
            file_fd = open(filename, O_WRONLY | O_CREAT | O_TRUNC, header.mode);
            if (file_fd < 0) {
                perror("create file");
                close(arch_fd);
                return -1;
            }
            
            // восстанавливаем владельца и группу
            if (fchown(file_fd, header.uid, header.gid) < 0) {
                perror("fchown");
                // продолжаем даже если не удалось изменить владельца
            }
            
            // копируем содержимое
            total_read = 0;
            while (total_read < header.size) {
                size_t bytes_to_read =(size_t) header.size - total_read;
                if(bytes_to_read > sizeof(buffer)){
                    bytes_to_read = sizeof(buffer);
                }
                bytes_read = read(arch_fd, buffer, bytes_to_read);
                if (bytes_read <= 0) {
                    break;
                }
                
                if (write(file_fd, buffer, bytes_read) != bytes_read) {
                    perror("write file");
                    close(file_fd);
                    close(arch_fd);
                    return -1;
                }
                total_read += bytes_read;
            }
            
            close(file_fd);
            close(arch_fd);
            return 0;
        } else {
            // пропускаем данные этого файла
            lseek(arch_fd, header.size, SEEK_CUR);
        }
    }
    
    printf("File not found in archive: %s\n", filename);
    close(arch_fd);
    return -1;
}

int remove_file_from_archive(const char *archive_name, const char *filename) {
    int arch_fd, temp_fd;
    FileHeader header;
    char temp_name[] = "/tmp/archiver_temp_XXXXXX";
    char buffer[4096];
    ssize_t bytes_read;
    int found = 0;
    off_t total_read; 

    // создаем временный файл
    temp_fd = mkstemp(temp_name);
    if (temp_fd < 0) {
        perror("create temp file");
        return -1;
    }
    
    // открываем оригинальный архив
    arch_fd = open(archive_name, O_RDONLY);
    if (arch_fd < 0) {
        perror("open archive");
        close(temp_fd);
        unlink(temp_name);
        return -1;
    }
    
    // копируем все файлы кроме удаляемого
    while (read(arch_fd, &header, sizeof(FileHeader)) == sizeof(FileHeader)) {
        if (strcmp(header.name, filename) == 0) {
            // пропускаем этот файл (удаляем)
            printf("Removing from archive: %s\n", filename);
            lseek(arch_fd, header.size, SEEK_CUR);
            found = 1;
        } else {
            // копируем заголовок и данные
            write(temp_fd, &header, sizeof(FileHeader));
            
            total_read = 0;
            while (total_read < header.size) {
                size_t bytes_to_read =(size_t) header.size - total_read;
                if(bytes_to_read > sizeof(buffer)){
                    bytes_to_read = sizeof(buffer);
                }
                bytes_read = read(arch_fd, buffer, bytes_to_read);
                if (bytes_read <= 0) break;
                write(temp_fd, buffer, bytes_read);
                total_read += bytes_read;
            }
        }
    }
    
    close(arch_fd);
    close(temp_fd);
    
    if (found) {
        // заменяем оригинальный архив временным
        if (rename(temp_name, archive_name) < 0) {
            perror("rename");
            unlink(temp_name);
            return -1;
        }
    } else {
        printf("File not found in archive: %s\n", filename);
        unlink(temp_name);
        return -1;
    }
    
    return 0;
}

int show_archive_stat(const char *archive_name) {
    int arch_fd;
    FileHeader header;
    struct stat arch_stat;
    int file_count = 0;
    off_t total_size = 0;
    
    // получаем информацию об архиве
    if (stat(archive_name, &arch_stat) < 0) {
        perror("stat archive");
        return -1;
    }
    
    // открываем архив для чтения
    arch_fd = open(archive_name, O_RDONLY);
    if (arch_fd < 0) {
        perror("open archive");
        return -1;
    }
    
    printf("Archive: %s\n", archive_name);
    printf("Total archive size: %ld bytes\n", arch_stat.st_size);
    printf("\nFiles in archive:\n");
    printf("%-30s %-10s %-8s %-8s\n", "Name", "Size", "UID", "GID");
    printf("------------------------------------------------------------\n");
    
    // читаем все заголовки и выводим информацию
    while (read(arch_fd, &header, sizeof(FileHeader)) == sizeof(FileHeader)) {
        printf("%-30s %-10ld %-8d %-8d\n", 
               header.name, header.size, header.uid, header.gid);
        file_count++;
        total_size += header.size;
        
        // пропускаем данные файла
        lseek(arch_fd, header.size, SEEK_CUR);
    }
    
    printf("\nTotal files: %d\n", file_count);
    printf("Total files size: %ld bytes\n", total_size);
    
    close(arch_fd);
    return 0;
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        print_help();
        return 1;
    }
    
    if (strcmp(argv[1], "-h") == 0 || strcmp(argv[1], "--help") == 0) {
        print_help();
        return 0;
    }
    
    if (argc < 3) {
        print_help();
        return 1;
    }
    
    const char *archive_name = argv[1];
    const char *operation = argv[2];
    
    if (strcmp(operation, "-i") == 0 || strcmp(operation, "--input") == 0) {
        if (argc < 4) {
            fprintf(stderr, "Error: No files specified for adding\n");
            return 1;
        }
        
        for (int i = 3; i < argc; i++) {
            if (add_file_to_archive(archive_name, argv[i]) < 0) {
                fprintf(stderr, "Failed to add file: %s\n", argv[i]);
            }
        }
        
    } else if (strcmp(operation, "-e") == 0 || strcmp(operation, "--extract") == 0) {
        if (argc < 4) {
            fprintf(stderr, "Error: No files specified for extraction\n");
            return 1;
        }
        
        for (int i = 3; i < argc; i++) {
            if (extract_file_from_archive(archive_name, argv[i]) < 0) {
                fprintf(stderr, "Failed to extract file: %s\n", argv[i]);
            }
        }
        
        // после извлечения удаляем файлы из архива
        for (int i = 3; i < argc; i++) {
            remove_file_from_archive(archive_name, argv[i]);
        }
        
    } else if (strcmp(operation, "-s") == 0 || strcmp(operation, "--stat") == 0) {
        if (show_archive_stat(archive_name) < 0) {
            fprintf(stderr, "Failed to show archive stat\n");
            return 1;
        }
        
    } else {
        fprintf(stderr, "Unknown operation: %s\n", operation);
        print_help();
        return 1;
    }
    
    return 0;
}

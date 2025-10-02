#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <stdlib.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <pwd.h>
#include <grp.h>
#include <time.h>
#include <errno.h>

#define FLAG_A (1 << 0)
#define FLAG_L (1 << 1) 


int flags = 0;
size_t files_size = 0;

const char* prefix;

const int PATH_SIZE = 1024;
char* path = NULL;

DIR* dir = NULL;
struct dirent** files_list = NULL;

void process_dir(const char* dir);
void prepare_files();
void prepare_path();
void print_files();
void print_long_format();
void print_short_format();
void file_path(const char* file);
const char* get_file_color(mode_t mode);

void get_permission(mode_t mode, char* str);
void format_time(time_t t, char* buffer, size_t size);

int dirent_cmp(const void* a, const void* b);

void clean();

#define COLOR_RESET     "\033[0m"
#define COLOR_GREEN     "\033[1;32m"
#define COLOR_BLUE      "\033[1;34m"
#define COLOR_CYAN      "\033[1;36m"

int main(int argc, char* argv[]){
    int opt;
    while((opt = getopt(argc, argv, "la")) != -1){
        switch(opt){
            case 'a':
                flags |= FLAG_A;
                break;
            case 'l':
                flags |= FLAG_L;
                break;
            default:
                printf("Неизвестный флаг %c\n", opt);
                return EXIT_FAILURE;
        }
    }
    if (argc == optind){
        process_dir(".");
        clean();
    }else if(argc == optind + 1){
        //обработка только одной дир.
        process_dir(argv[optind]);
        clean();
    }else{
        //обработка нескольких дир.
        for(int i = optind; i < argc; i++){
            printf("%s:\n", argv[i]);
            process_dir(argv[i]);
            clean();
            printf("\n");
        }
    }
    //if(files_list != NULL) free(files_list);
    return EXIT_SUCCESS;
}

void process_dir(const char* dirname){

    dir = opendir(dirname);
    if(!dir){
        printf("Не получилось обработать директорию '%s':  %s\n", dirname, strerror(errno));
        return;
    }   

    prefix = dirname;
    prepare_files(); 
    prepare_path();

    print_files(); 

    closedir(dir);
}

void prepare_files(){
    files_size = 0;
    for(struct dirent *cur_file; (cur_file = readdir(dir)) != NULL;){
        if(cur_file->d_name[0] == '.' && !(flags & FLAG_A)) continue;
        files_size++;
        
    }
    if(files_list != NULL) {
        free(files_list);
        files_list = NULL;
    }
    files_list =(struct dirent**) malloc(files_size * sizeof(struct dirent*));
    
    rewinddir(dir);
    int i = 0;
    for(struct dirent* cur_file; (cur_file = readdir(dir)) != NULL;){
        if(cur_file->d_name[0] == '.' && !(flags & FLAG_A)) continue;
        files_list[i++] = cur_file;
    }
    qsort(files_list, files_size, sizeof(struct dirent*), dirent_cmp);
}

int dirent_cmp(const void* a, const void* b){
    struct dirent *f = *(struct dirent **)a, *s = *(struct dirent **)b;
    return strcmp(f->d_name, s->d_name);
}

void print_files(){
    if(flags & FLAG_L) print_long_format();
    else print_short_format();
}

void print_short_format(){
    for(size_t i = 0; i < files_size; i++){
        struct stat file_info;
        file_path(files_list[i]->d_name);
        if(lstat(path, &file_info) == -1){
            continue;
        }

        printf("%s%s ", get_file_color(file_info.st_mode),  files_list[i]->d_name);
    }
    printf("%s\n", COLOR_RESET);
    printf("\n");
}

const char* get_file_color(mode_t mode){
    if (S_ISLNK(mode)) return COLOR_CYAN;
    if (S_ISDIR(mode)) return COLOR_BLUE;
    if (mode & S_IXUSR) return COLOR_GREEN;
    return COLOR_RESET;
}

void print_long_format(){
    int max_links = 0, max_user = 0, max_group = 0, max_size = 0;
    long long total_blocks = 0;
    char time_buf[32];
    char perm_buf[11];

    for(size_t i = 0; i < files_size; i++){
        struct stat st;
        file_path(files_list[i]->d_name);
        if(lstat(path, &st) == -1) continue;
        struct passwd* pwd = getpwuid(st.st_uid);
        struct group* grp = getgrgid(st.st_gid);

        int links_len = snprintf(NULL, 0, "%lu", (unsigned long)st.st_nlink);
        if (links_len > max_links) max_links = links_len;

        if (pwd){
            int user_len = strlen(pwd->pw_name);
            if (user_len > max_user) max_user = user_len;
        }

        if (grp){
            int group_len = strlen(grp->gr_name);
            if (group_len > max_group) max_group = group_len;
        }

        int size_len = snprintf(NULL, 0, "%ld", (long)st.st_size);
        if (size_len > max_size) max_size = size_len;

        total_blocks += st.st_blocks;
    }
    
    printf("total %lld\n", total_blocks / 2);
    for(size_t i = 0; i < files_size; i++){
        struct stat st;
        file_path(files_list[i]->d_name);
        if(lstat(path, &st) == -1) continue;
        struct passwd* pwd = getpwuid(st.st_uid);
        struct group* grp = getgrgid(st.st_gid);
        
        get_permission(st.st_mode, perm_buf);
        printf("%s ", perm_buf);
        
        printf("%*lu ", max_links, (unsigned long)st.st_nlink);

        if (pwd){
            printf("%-*s ", max_user, pwd->pw_name);
        } else {
            printf("%-*d ", max_user, st.st_uid);
        }

        if (grp){
            printf("%-*s ", max_group, grp->gr_name);
        } else {
            printf("%-*d ", max_group, st.st_gid); 
        }

        printf("%*ld ", max_size, (long)st.st_size);
        
        format_time(st.st_mtime, time_buf, sizeof(time_buf));
        printf("%s ", time_buf);

        const char* color = get_file_color(st.st_mode);
        printf("%s%s%s", color, files_list[i]->d_name, COLOR_RESET);

        if (S_ISLNK(st.st_mode)){
            char link_target[1024];
            ssize_t len = readlink(path, link_target, sizeof(link_target) - 1);
            if (len != -1){
                link_target[len] = '\0';
                printf(" -> %s", link_target);
            }
        }

        printf("\n");
    }
}

void get_permission(mode_t mode, char* str){
    str[0] = (S_ISDIR(mode)) ? 'd' : (S_ISLNK(mode)) ? 'l' : '-';
    str[1] = (mode & S_IRUSR) ? 'r' : '-';
    str[2] = (mode & S_IWUSR) ? 'w' : '-';
    str[3] = (mode & S_IXUSR) ? 'x' : '-';
    str[4] = (mode & S_IRGRP) ? 'r' : '-';
    str[5] = (mode & S_IWGRP) ? 'w' : '-';
    str[6] = (mode & S_IXGRP) ? 'x' : '-';
    str[7] = (mode & S_IROTH) ? 'r' : '-';
    str[8] = (mode & S_IWOTH) ? 'w' : '-';
    str[9] = (mode & S_IXOTH) ? 'x' : '-';
    str[10] = '\0';
}

void format_time(time_t t, char* buffer, size_t size) {
    struct tm* tm_info = localtime(&t);
    time_t now = time(NULL);
    
    // Если файл старше 6 месяцев, показываем год вместо времени
    if (now - t > 15778476) { // ~6 месяцев в секундах
        strftime(buffer, size, "%b %e  %Y", tm_info);
    } else {
        strftime(buffer, size, "%b %e %H:%M", tm_info);
    }
}

void prepare_path(){
    if (path != NULL) free(path);
    path = (char*)malloc(PATH_SIZE);
}

void file_path(const char* file){
    path[0] = '\0';
    strcat(path, prefix);
    strcat(path, "/");
    strcat(path, file);
}

void clean(){
    if(path != NULL) {
        free(path);
        path = NULL;
    }
    files_size = 0;
    if(files_list != NULL) {
        free(files_list);
        files_list = NULL;
    }
}

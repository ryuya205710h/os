#include <stdio.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#include <pwd.h>
#include <grp.h>
#include <time.h>
#include <stdlib.h>
#include <string.h>

#define INITIAL_STACK_SIZE 10

typedef struct {
    DIR *dir;
    int depth;
} DirStackEntry;

typedef struct {
    DirStackEntry *entries;
    size_t size;
    size_t capacity;
} DirStack;

void initDirStack(DirStack *stack) {
    stack->entries = malloc(INITIAL_STACK_SIZE * sizeof(DirStackEntry));
    stack->size = 0;
    stack->capacity = INITIAL_STACK_SIZE;
}

void pushDirStack(DirStack *stack, DIR *dir, int depth) {
    if (stack->size == stack->capacity) {
        stack->capacity *= 2;
        stack->entries = realloc(stack->entries, stack->capacity * sizeof(DirStackEntry));
    }
    stack->entries[stack->size].dir = dir;
    stack->entries[stack->size].depth = depth;
    stack->size++;
}

DirStackEntry popDirStack(DirStack *stack) {
    if (stack->size == 0) {
        DirStackEntry emptyEntry = {NULL, 0};
        return emptyEntry;
    }
    stack->size--;
    return stack->entries[stack->size];
}

void listFilesRecursively(const char *path) {
    DirStack stack;
    initDirStack(&stack);

    DIR *dir = opendir(path);

    if (dir == NULL) {
        perror("opendir");
        return;
    }

    pushDirStack(&stack, dir, 0);

    while (stack.size > 0) {
        DirStackEntry current = popDirStack(&stack);
        DIR *currentDir = current.dir;
        int depth = current.depth;

        struct dirent *entry;
        struct stat info;

        while ((entry = readdir(currentDir)) != NULL) {
            char fullpath[1024];
            snprintf(fullpath, sizeof(fullpath), "%s/%s", path, entry->d_name);

            if (stat(fullpath, &info) != -1) {
                struct passwd *pw = getpwuid(info.st_uid);
                struct group *gr = getgrgid(info.st_gid);

                char perm[11];
                snprintf(perm, sizeof(perm), "%c%c%c%c%c%c%c%c%c%c",
                    (S_ISDIR(info.st_mode)) ? 'd' : '-',
                    (info.st_mode & S_IRUSR) ? 'r' : '-',
                    (info.st_mode & S_IWUSR) ? 'w' : '-',
                    (info.st_mode & S_IXUSR) ? 'x' : '-',
                    (info.st_mode & S_IRGRP) ? 'r' : '-',
                    (info.st_mode & S_IWGRP) ? 'w' : '-',
                    (info.st_mode & S_IXGRP) ? 'x' : '-',
                    (info.st_mode & S_IROTH) ? 'r' : '-',
                    (info.st_mode & S_IWOTH) ? 'w' : '-',
                    (info.st_mode & S_IXOTH) ? 'x' : '-');

                char time_str[20];
                strftime(time_str, sizeof(time_str), "%b %d %H:%M", localtime(&info.st_mtime));

                for (int i = 0; i < depth; i++) {
                    printf("  ");
                }

                printf("%s %lu %-8s %-8s %lld %s %s\n",
                    perm,
                    (unsigned long)info.st_nlink,
                    pw->pw_name,
                    gr->gr_name,
                    (long long)info.st_size,
                    time_str,
                    entry->d_name);
            } else {
                perror("stat");
            }

            if (S_ISDIR(info.st_mode) && strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0) {
                DIR *subdir = opendir(fullpath);
                if (subdir != NULL) {
                    pushDirStack(&stack, subdir, depth + 1);
                }
            }
        }

        closedir(currentDir);
    }

    free(stack.entries);
}

int main() {
    const char *path = "."; // リストアップするディレクトリのパス
    listFilesRecursively(path);

    return 0;
}

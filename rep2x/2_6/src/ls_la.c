#include <stdio.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#include <pwd.h>
#include <grp.h>
#include <time.h>

void listFiles(const char *path) {
    DIR *dir = opendir(path);

    if (dir == NULL) {
        perror("opendir");
        return;
    }

    struct dirent *entry;
    struct stat info;

    while ((entry = readdir(dir)) != NULL) {
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
    }

    closedir(dir);
}

int main() {
    const char *path = "."; // リストアップするディレクトリのパス
    listFiles(path);

    return 0;
}

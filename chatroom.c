//  chatroom.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <errno.h>

static void check_room_exists(const char *room_dir) {
   
    if (mkdir(room_dir, 0777) == -1 && errno != EEXIST) {  // creates room if missing
        exit(1);
    }
}

static void check_user_fifo_exists(const char *fifo_path) {
    
    if (mkfifo(fifo_path, 0666) == -1 && errno != EEXIST) { // creates user fifo if missing
        exit(1);
    }
}

static void receiver(const char *my_fifo_path) {
    
    int fd = open(my_fifo_path, O_RDONLY); // opens fifo for reading
    if (fd < 0) exit(1);

    char buf[4096];

   
    while (1) {  // continuously reads and print anything that arrives
        ssize_t n = read(fd, buf, sizeof(buf) - 1);
        if (n > 0) {
            buf[n] = '\0';
            printf("%s", buf);
            fflush(stdout);
        }
    }
}


static void send_one(const char *fifo_path, const char *msg) { // sends one message to one users fifo
    int fd = open(fifo_path, O_WRONLY);   // blocks until receiver opens fifo
    if (fd < 0) exit(0);

    write(fd, msg, strlen(msg));
    close(fd);
    exit(0);
}

static void broadcast(const char *room_dir, const char *roomname,const char *username, const char *text) {
    char msg[8192];
    snprintf(msg, sizeof(msg), "[%s] %s: %s\n", roomname, username, text);

   
    DIR *d = opendir(room_dir);  // iterates over all files in the room directory
    if (!d) return;

    struct dirent *ent;
    while ((ent = readdir(d)) != NULL) {
        if (strcmp(ent->d_name, ".") == 0 || strcmp(ent->d_name, "..") == 0)
            continue;

        
        if (strcmp(ent->d_name, username) == 0)
            continue;

        
        char other_fifo[8192]; // build path to the other users fifo
        snprintf(other_fifo, sizeof(other_fifo), "%s/%s", room_dir, ent->d_name);

        
        if (fork() == 0) { // uses a child to write
            send_one(other_fifo, msg);
        }
    }

    closedir(d);

    
    while (waitpid(-1, NULL, WNOHANG) > 0) {}
}

int main(int argc, char **argv) {
    
    if (argc != 3) {
        return 1;
    }

    const char *roomname = argv[1];
    const char *username = argv[2];

    // room folder
    char room_dir[8192];
    snprintf(room_dir, sizeof(room_dir), "/tmp/chatroom-%s", roomname);

    // user fifo
    char my_fifo[8192];
    snprintf(my_fifo, sizeof(my_fifo), "%s/%s", room_dir, username);

    check_room_exists(room_dir);
    check_user_fifo_exists(my_fifo);

    printf("Welcome to %s!\n", roomname);
    fflush(stdout);

    // fork one child to continuously receive messages
    if (fork() == 0) {
        receiver(my_fifo);
        exit(0);
    }

    
    else {
        
        char line[4096];

        while (1) {
            printf("[%s] %s > ", roomname, username);
            fflush(stdout);

            if (!fgets(line, sizeof(line), stdin)){
                break;
            }

           
            size_t L = strlen(line);
            if (L > 0 && line[L - 1] == '\n'){
                line[L - 1] = '\0';
            }
            if (line[0] == '\0'){
                continue;
            }
            broadcast(room_dir, roomname, username, line);
        }
    }

    return 0;
}

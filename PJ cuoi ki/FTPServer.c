#include <stdio.h>
#include <dirent.h>
#include <string.h>
#include <malloc.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>
#include <stdlib.h>
#include <netinet/in.h> 
#include <ctype.h>
#include <sys/socket.h>

#define INVALID_SOCKET -1
#define MAX_CLIENT 1024
#define MAX_READ 255
#define SERVERPORT 5000
typedef struct sockaddr_in SOCKADDR_IN;
typedef struct sockaddr SOCKADDR;


char* getUsername(char infor[]) {
    char* username = (char*)malloc(255* sizeof(char));
    strcpy(username, infor);
    for(int i = 0; i< strlen(username); i++) {
        if(username[i] == '-'){
            username[i] = '\0';
            return username;
        }
    }
    return NULL;
}
int validateUser(char buffer[]){
    FILE* fptr = fopen("user/user.txt", "r");
    char user_infor[255] = {0};
    char* username = (char*)malloc(255* sizeof(char));

    char* inputusername = (char*)malloc(255* sizeof(char));
    while(fgets(user_infor, 255, fptr) != NULL) {
        user_infor[strlen(user_infor)-1] = '\0';
        strcpy(username,getUsername(user_infor));
        strcpy(inputusername, getUsername(buffer));
        if(strcmp(username, inputusername) == 0) {
            free(username);
            free(inputusername);
            if(strcmp(buffer, user_infor) == 0) {
                //user hợp lệ
                return 1;
            } else 
            //đúng username nhưng sai password
            return 0;
        }
    }
    //user ko tồn tại
    free(username);
    free(inputusername);
    return -1;
}

void checklogin(int client_fd){
    char buffer[255] = {0};
    int n = recv(client_fd, buffer, sizeof(buffer) - 1, 0);
    int check = validateUser(buffer);
    if(check != 1) {
        send(client_fd, "0", 2, 0);
    }
    else send(client_fd, "1", 2, 0);
}

void signup(int client_fd){
    FILE* fptr = fopen("user/user.txt", "a+");
    char buffer[255] = {0};
   
    int n = recv(client_fd, buffer, sizeof(buffer) - 1, 0);

    int check = validateUser(buffer);
    if(check == -1) {
        fprintf(fptr, "%s\n", buffer);
        send(client_fd, "1", 2, 0);
    }
    else {
        send(client_fd, "0", 2, 0);
    }
    fclose(fptr);
}

int downloadFile(int client_fd, char*currentDir) {
    char buffer[255] = {0};
    char* filename = (char*)malloc(255*sizeof(char));
    strcpy(filename, currentDir);
    strcat(filename, "/");
    recv(client_fd, buffer, sizeof(buffer) - 1, 0);
    strcat(filename, buffer);
    printf("%s\n", filename);
    FILE* fptr = fopen(filename, "r");
    if(fptr == NULL) {
        send(client_fd, "0", 1, 0);
        return 0;
    }
    else {
        send(client_fd, "1", 1, 0);
        fseek(fptr, 0, SEEK_END); //di ve cuoi
            int size = ftell(fptr); //lay size
            char* data = (char*)calloc(size, 1); //cap bo nho = size
            fseek(fptr, 0, SEEK_SET); //ve lai dau
            fread(data, 1, size, fptr); //doc het vao data
            fclose(fptr);
            int sent = 0; //tong so bite da gui
            char filesize[10] = {0};
            sprintf(filesize, "%d", size);
            int m = send(client_fd, filesize, 10, 0);
            printf("%s\n", filesize);
            while (sent <= size) //cho den khi gui het
            {
                printf("Sent %d/%d\n",sent,size);
                if(sent == size) {
                    break;
                }
                int tmp = send(client_fd, data + sent, size - sent, 0); 
                sent += tmp;
            }
            free(data);
    return 1;
    }
}

int uploadFiles(int client_fd, char*currentDir) {
    char* filename= (char*)calloc(255, sizeof(char)); ;
    char buffer[255] = {0};
    strcpy(filename, currentDir);
    strcat(filename, "/");
    recv(client_fd, buffer, 255, 0);
    printf("%s\n", buffer);
    strcat(filename, buffer);
    printf("%s\n", filename);
    FILE* fptr = fopen(filename, "w+");
    char filesizeStr[10] = {0};
    int m = recv(client_fd, filesizeStr, 10, 0);
    int size = atoi(filesizeStr);
    printf("%d\n", size);
    char* buff = (char*)calloc(size + 1, sizeof(char)); //cap bo nho = size
    int n = 4;
    while(n == 4) {
        n = recv(client_fd, buff, 4, 0);
        printf("%s", buff);
        fwrite(buff, 1, strlen(buff), fptr);
        memset(buff, 0, 4);
    }
    fclose(fptr);
    free(buff);
    free(filename);
    return 1;

}

void displayFileList(int client_fd, char*currentDir) {
    char*cmd = (char*)calloc(255, sizeof(char));
    strcpy(cmd, "ls ");
    strcat(cmd, currentDir);
    strcat(cmd, " > filelist.txt");
    system(cmd);
    FILE* fptr = fopen("filelist.txt", "r");
    fseek(fptr, 0, SEEK_END); //di ve cuoi
    int size = ftell(fptr); //lay size
    char* data = (char*)calloc(size, 1); //cap bo nho = size
    fseek(fptr, 0, SEEK_SET); //ve lai dau
    fread(data, 1, size, fptr); //doc het vao data
    fclose(fptr);
    char filesize[10] = {0};
    sprintf(filesize, "%d", size);
    int m = send(client_fd, filesize, 10, 0);
    send(client_fd, data, size, 0);
    free(cmd);
    free(data);
}

void changeDirectory(int client_fd, char*currentDir) {
    char tmp[255] = {0};
    int n = recv(client_fd, tmp, 255, 0);
    printf("%s", tmp);
    DIR* dir = opendir(tmp);
    if(dir) {
        strcpy(currentDir, tmp);
        send(client_fd, "1", 1, 0);
    }
    else send(client_fd, "0", 1, 0);
}

void handle(int client_fd) {
    char* currentDir = (char*)calloc(255, sizeof(char));
    strcpy(currentDir, "files");
    int status;
    while (0 == 0) {
       
        char choose[2] = {0};
        recv(client_fd, choose, 1, 0);
        int i = atoi(choose);
        switch(i) {
            case 1: 
            checklogin(client_fd);
            break;

            case 2:
            signup(client_fd);
            break;

            case 3:
            displayFileList(client_fd, currentDir);
            break;
            case 4:
            changeDirectory(client_fd, currentDir);
            break;
            case 5:
            // printf("start");
            status = downloadFile(client_fd, currentDir);
            if(status == 1) {
                printf("done\n");
            }
            break;

            case 6:
            status = uploadFiles(client_fd, currentDir);
            if(status == 1) {
                printf("done\n");
            }
            break;
        }
        // break;
    }
    free(currentDir);
}

int main() {
    int server_fd;
    SOCKADDR_IN serverAddr, clientAddr;
    //tạo socket phía server
    if((server_fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0) {
        printf("\n Socket creation error \n"); 
        return -1; 
    }
    //setup port
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(SERVERPORT);
    serverAddr.sin_addr.s_addr = 0;
    //bind
    if(bind(server_fd, (SOCKADDR*)&serverAddr, sizeof(serverAddr)) < 0) {
        printf("\n Socket bind error\n");
        return -1;
    }
    //listen
    listen(server_fd, 100);

    //accept
    while(0 == 0) {
    int clen = sizeof(serverAddr);
    int client_fd = accept(server_fd, (SOCKADDR*)&serverAddr, &clen);
    if(fork() == 0) {
        close(server_fd);
        handle(client_fd);
    }
    else close(client_fd);
    }
}
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
#include <stdio_ext.h>
#include <time.h>
#define INVALID_SOCKET -1
#define MAX_CLIENT 1024
#define SERVERPORT 5000
typedef struct sockaddr_in SOCKADDR_IN;
typedef struct sockaddr SOCKADDR;

int login(int client_fd, SOCKADDR_IN addr) {
        char username[100];
        char password[100];
        char user_infor[200] = {0};
        char rep[2] = {0};
        printf("Nhập username:\n");
        __fpurge(stdin);
        scanf("%s", username);
        printf("Nhập password:\n");
        __fpurge(stdin);
        scanf("%s", password);
        strcat(user_infor, username);
        strcat(user_infor, "---");
        strcat(user_infor, password);
        printf("%s\n", user_infor);
        //gửi đến server lệnh đã chọn
        send(client_fd, "1", 1, 0);
        //gửi đến server thông tin user để kiểm tra
        int n = send(client_fd, user_infor, strlen(user_infor), 0);
        //nhận response từ server
        recv(client_fd, rep, n, 0);
        int signal = atoi(rep);
        if(signal == 0) {
            printf("User không tồn tại hoặc password không đúng !\n");
            return 0;
        }
        else {
            printf("Đăng nhập thành công!\n");
            return 1;
        }
}

int createNewUser(int client_fd, SOCKADDR_IN addr) {
        char username[100];
        char password[100];
        char user_infor[200] = {0};
        char rep[2] = {0};
        printf("Nhập username:\n");
        __fpurge(stdin);
        scanf("%s", username);
        printf("Nhập password:\n");
        __fpurge(stdin);
        scanf("%s", password);
        strcat(user_infor, username);
        strcat(user_infor, "---");
        strcat(user_infor, password);
        printf("%s\n", user_infor);
        //gửi đến server lệnh đã chọn
        send(client_fd, "2", 1, 0);
        //gửi đến server thông tin user để tạo mới
        int n = send(client_fd, user_infor, strlen(user_infor), 0);
        //nhận response từ server
        recv(client_fd, rep, sizeof(rep), 0);
        int signal = atoi(rep);
        if(signal == 0) {
            printf("Username đã tồn tại trong hệ thống!\n");
            return 0;
        }
        else {
            printf("Tạo mới user và login thành công!\n");
            return 1;
        }
}

int downloadFiles(int client_fd, SOCKADDR_IN addr) {
    char* filename= (char*)malloc(255*sizeof(char));
    char response[2] = {0};
    printf("Enter a filename you want to download! \n");
    __fpurge(stdin);
    fgets(filename, 255, stdin);
    filename[strlen(filename)-1] = '\0';
    // printf("%s\n", filename);
    send(client_fd, "5", 1, 0);
    int n = send(client_fd, filename, strlen(filename), 0);
    recv(client_fd, response, sizeof(response), 0);
    int signal = atoi(response);
    if(signal == 0 ) {
        printf("File %s không tồn tại !\n", filename);
        return -1;
    }
    else {
        printf("Start downloading %s ....\n", filename);
        FILE* fptr = fopen(filename, "wt");
        clock_t start = clock();
        char filesizeStr[10] = {0};
        int m = recv(client_fd, filesizeStr, 10, 0);
        int size = atoi(filesizeStr);
        char* buff = (char*)calloc(size, sizeof(char)); //cap bo nho = size
        int n = recv(client_fd, buff, size, 0);
        // printf("%s\n", buff);
        fwrite(buff, 1, n, fptr);
        fclose(fptr);
        free(buff);
        clock_t end = clock();
        double run_time = (double)(end-start)/CLOCKS_PER_SEC;
        printf("Downloading successfully after %lf seconds.", run_time);
    }
    free(filename);
    return 1;

}

int uploadFiles(int client_fd) {
    char* filename= (char*)malloc(255*sizeof(char));
    char response[2] = {0};
    printf("Enter a filename you want to upload! \n");
    __fpurge(stdin);
    fgets(filename, 255, stdin);
    filename[strlen(filename)-1] = '\0';
    FILE* fptr = fopen(filename, "r");
    if(fptr == NULL) {
        printf("File %s is not exist !", filename);
        return 0;
    }
    else {
        send(client_fd, "6", 1, 0);
        send(client_fd, filename, strlen(filename) + 1, 0);
        fseek(fptr, 0, SEEK_END); //di ve cuoi
            int size = ftell(fptr); //lay size
            char* data = (char*)calloc(size, 1); //cap bo nho = size
            fseek(fptr, 0, SEEK_SET); //ve lai dau
            fread(data, 1, size, fptr); //doc het vao data
            // printf("%s\n", data);
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
    }
}

int main() {
    int isLoggedIn = 0;
    int download;
    char* directory = (char*)calloc(255, sizeof(char));
    int client_fd, new_socket;
    SOCKADDR_IN addr;
    //tạo socket phía server
    if((client_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        printf("\n Socket creation error \n"); 
        return -1; 
    }
    //setup port
    addr.sin_family = AF_INET;
    addr.sin_port = htons(SERVERPORT);
    addr.sin_addr.s_addr = 0;
    //connect
    if(connect(client_fd, (SOCKADDR*)&addr, sizeof(addr)) < 0) {
        printf("\n Socket bind error\n");
        return -1;
    }
    char c;
    while(c != '7') {
        printf("\n-----Menu-----\n");
        printf("1. Login\n");
        printf("2. Register\n");
        printf("3. Get file list\n");
        printf("4. Change directory\n");
        printf("5. Download files\n");
        printf("6. Upload files\n");
        printf("7. Exit\n");
        __fpurge(stdin);
        scanf("%c", &c);
        switch(c){
            case '1':
                isLoggedIn = login(client_fd, addr);
            break;

            case '2':
                isLoggedIn = createNewUser(client_fd, addr);
            break;

            case '3':
            if(isLoggedIn > 0) {
                //gửi đến server lệnh đã chọn
                send(client_fd, "3", 1, 0);
                
                char filesizeStr[10] = {0};
                int m = recv(client_fd, filesizeStr, 10, 0);
                int size = atoi(filesizeStr);
                char* buff = (char*)calloc(size, sizeof(char)); //cap bo nho = size
                m = recv(client_fd, buff, size, 0);
                printf("%s", buff);
                free(buff);
            }
            else printf("Please login to use this function\n");
            break;

            case '4':
            if(isLoggedIn > 0) {
                printf("Nhập tên đường dẫn bạn muốn thay đổi:\n");
                __fpurge(stdin);
                scanf("%s", directory);
                send(client_fd, "4", 1, 0);
                send(client_fd, directory, strlen(directory), 0);
                char tmp[10] = {0};
                int n = recv(client_fd, tmp, 1, 0);
                int signal = atoi(tmp);
                if(signal== 1) printf("Change directory successfully to %s\n", directory);
                else printf("the directory %s is not exist !\n", directory);
            } else printf("Please login to use this function\n");
            break;

            case '5':
            if(isLoggedIn > 0) {
                downloadFiles(client_fd, addr);
            } else printf("Please login to use this function\n");
            break;

            case '6':
            if(isLoggedIn > 0) {
                uploadFiles(client_fd);
            } else printf("Please login to use this function\n");
            break;

            case '7':
            close(client_fd); 
            break;
        }
    }
}
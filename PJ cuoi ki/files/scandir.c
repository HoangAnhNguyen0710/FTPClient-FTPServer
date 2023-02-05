#include <stdio.h>
#include <dirent.h>
#include <string.h>
#include <malloc.h>

void Append(char** pdst, const char* src)
{
    char* dst = *pdst; //Xau ky tu cu
    int oldLen = (dst == NULL ? 0 : strlen(dst));
    int newLen = oldLen + strlen(src) + 1;
    dst = (char*)realloc(dst, newLen);
    memset(dst + oldLen, 0, strlen(src) + 1);
    sprintf(dst + oldLen, "%s", src);
    *pdst = dst;
}

int Compare(const struct dirent** a, const struct dirent** b)
{
    if ((*a)->d_type == (*b)->d_type)
        return 0;
    else if ((*a)->d_type == DT_DIR)
        return -1;
    else
        return 1;
}

int main()
{
    char* rootPath = "/mnt/e/";
    char* tmpPath = NULL;
    tmpPath = (char*)calloc(strlen(rootPath) + 1, 1);
    strcpy(tmpPath, rootPath);
    
    int exit = 0;
    do
    {
        struct dirent** output = NULL;
        char* html = NULL;
        Append(&html,"<html>");
        int n = scandir(tmpPath, &output, NULL, Compare);
        if (n > 0)
        {
            for (int i = 0;i < n;i++)
            {
                char line[1024] = { 0 };
                if (output[i]->d_type == DT_DIR)
                    sprintf(line, "<a href = \"%s\"><b>%s</b></a>", output[i]->d_name, output[i]->d_name);
                if (output[i]->d_type == DT_REG)
                    sprintf(line, "<a href = \"%s\"><i>%s</i></a>", output[i]->d_name, output[i]->d_name);
                Append(&html, line);
                Append(&html, "<br>");
            }
        }
        Append(&html, "</html>");
        FILE* f = fopen("output.html", "wt");
        fprintf(f, "%s", html);
        fclose(f);

        free(html);
        html = NULL;
        char command[1024] = { 0 };
        fgets(command, sizeof(command), stdin);
        while ( command[strlen(command) - 1] == '\r' || 
                command[strlen(command) - 1] == '\n')
        {
            command[strlen(command) - 1] = 0;
        }

        exit = 1;
        if (strcmp(command,"..") == 0)
        {
            exit = 0;
            if (strcmp(tmpPath, rootPath) != 0)
            {
                tmpPath[strlen(tmpPath) - 1] = 0; //Cat dau / o cuoi di
                while (tmpPath[strlen(tmpPath) - 1] != '/')
                {
                    tmpPath[strlen(tmpPath) - 1] = 0; //Cat ky tu cuoi
                }
            }
        }else 
        {
            for (int i = 0;i < n;i++)
            {
                if (output[i]->d_type == DT_DIR && 
                    strcmp(output[i]->d_name, command) == 0)
                {
                    tmpPath = (char*)realloc(tmpPath, strlen(tmpPath) + strlen(command) + 1);
                    sprintf(tmpPath + strlen(tmpPath),"%s/", command);
                    exit = 0;
                    break;    
                }
            }        
        }

        printf("%s\n", tmpPath);
        printf("%d\n", exit);

        for (int i = 0;i < n;i++)
        {
            free(output[i]);
            output[i] = NULL;
        }
        free(output);
        output = NULL;
    }while (!exit);
}
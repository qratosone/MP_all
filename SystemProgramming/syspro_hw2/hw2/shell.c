/** @file shell.c */
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include "log.h"
#define MAX_SIZE 1000

log_t Log;

/**
 * Starting point for shell.
 */
void prefix(){
    char* buffer=(char*)malloc(MAX_SIZE);
    getcwd(buffer,MAX_SIZE);
    printf("(pid=%d)%s$ ", getpid(), buffer);
    free(buffer);
}
void list_log(log_t *log){
    if(log->head!=NULL){
        node* this=log->head;
        while(this){
            printf("%s\n",this->cmd);
            this=this->next;
        }
    }
}
int split(char *args[],char* str,const char* spl){
    int n=0;
    char* result=NULL;
    result=strtok(str,spl);
    while(result!=NULL){
        args[n]=(char*)malloc(strlen(result)+1);
        strcpy(args[n++],result);
        result=strtok(NULL,spl);
    }
    return n;
}
int execute(char* buffer){
    int i=0;
    if(buffer[0]!='!'){
        log_push(&Log,buffer);
    }
    if(!strncmp("cd",buffer,2)){
        for(i=0;i<strlen(buffer);i++){
            if(buffer[i]==' '){
                break;
            }
        }
        int result=chdir(buffer+i+1);
        if(result<0){
            printf("%s: No such file or directory\n",buffer+i+1);
        }
    }else if(!strcmp("exit",buffer)){
        return 0;
    }else if(!strcmp("!#",buffer)){
        list_log(&Log);
    }else if(!strncmp("!",buffer,1)){
        char* result=log_search(&Log,buffer+1);
        if(result!=NULL){
            execute(result);
        }
        else{
            printf("No Match\n");
        }
    }else if(strlen(buffer)){
        char* args[10];
        int i=0;
        for(i=0;i<10;i++){
            args[i]=NULL;
        }
        split(args,buffer," ");
        pid_t pid=fork();
        if(pid>0){//father
            int status;
            waitpid(pid,&status,0);
        }
        else{//child
            //确保子进程被杀死
            if(execv(buffer,args)){
                printf("%s: not found\n",buffer);
            }
            log_destroy(&Log);
            for(i=0;i<10;i++){
                free(args[i]);
            }
            exit(0);

        }
        for(i=0;i<10;i++){
            free(args[i]);
        }

    }
    return 1;

}
int main() {
    log_init(&Log); //初始化Log
    prefix(); //显示路径
    size_t len=0;
    char* buffer=NULL;
    while(getline(&buffer,&len,stdin)!=-1){//读取命令
        buffer[strlen(buffer)-1]='\0';//末位设置为0
        if(!execute(buffer)){//执行命令
            break;
        }
        prefix();
        free(buffer);//释放内存
        buffer=NULL;
    }
    if(buffer!=NULL){//意外终止的处理
        free(buffer);
        buffer=NULL;
    }
    log_destroy(&Log);
    return 0;
}

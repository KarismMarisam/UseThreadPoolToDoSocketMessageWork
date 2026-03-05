#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <pthread.h>
#include "../ThreadPool/heads/ThreadPool.h"

struct SockInfo{//client socket info 
    struct sockaddr_in addr;
    int fd;
    SockInfo(){
        bzero(this,sizeof(this));
        fd=-1;
    }
};

struct PoolInfo{
    ThreadPool* pool;
    int fd;
    PoolInfo(ThreadPool* p,int f){
        pool=p;
        fd=f;
    }
};



void working(void* arg);//task for message
void acceptWork(void* arg);//task for accept clients request for link 

int main(){
    //create socket for monitor
    int fd=socket(AF_INET,SOCK_STREAM,0);//AF_INET means ipv4, soket_stream and 0  means tcp
    if(fd==-1){
        perror("socket");//use to print whats wrong with .h or sys call
        return -1;
    }
    
    //bind local ip and port,set sockaddr info
    struct sockaddr_in saddr;
    saddr.sin_family=AF_INET;
    saddr.sin_port=htons(9999);//host to net short
    saddr.sin_addr.s_addr=INADDR_ANY;//0.0.0.0 auto to read local ip
    int ret=bind(fd,(struct sockaddr*)&saddr,sizeof(saddr));
    if(ret==-1){
        perror("bind");
        return -1;
    }
    //set monitor
    ret=listen(fd,128);
    if(ret==-1){
        perror("listen");
        return -1;
    }
    ThreadPool* pool=new ThreadPool(3,8);
    struct PoolInfo* poolInfoP=new PoolInfo(pool,fd);
    pool->addTask(acceptWork,poolInfoP);
    pthread_exit(NULL);
}

void acceptWork(void* arg){
    struct PoolInfo* p=(struct PoolInfo*)arg;
    while(1){
        struct SockInfo* pinfo;
        socklen_t addrlen = sizeof(pinfo->addr);
        pinfo=new SockInfo();
        int cfd=accept(p->fd,(struct sockaddr*)&pinfo->addr,&addrlen);// return socket for message,if no client to connect it will stuck
        pinfo->fd=cfd;
        if(cfd==-1){
            perror("accept");
            break;
        }
        //add task for message
        p->pool->addTask(working,pinfo);
    }
    close(p->fd);
    delete(p);
}


void working(void* arg){
        struct SockInfo* pinfo=(struct SockInfo*)arg;
        char ip[32];
        printf("client ip:%s,client port:%d\n",inet_ntop(AF_INET,&pinfo->addr.sin_addr.s_addr,ip,sizeof(ip)),
    htons(pinfo->addr.sin_port));//message   

    while(1){
        char buff[1024];
        int len=recv(pinfo->fd,buff,sizeof(buff),0);
        if(len>0){
            printf("client say:%s\n",buff);
            send(pinfo->fd,buff,len,0);
        }
        else if(len==0){
            printf("client already cut off link\n");
            break;
        }
        else{
            perror("recv");
            break;
        }
    }
    close(pinfo->fd);//based on the fd to close the socket 
    delete(pinfo);
}
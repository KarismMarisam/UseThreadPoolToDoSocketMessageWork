#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <pthread.h>

struct SockInfo{//client socket info 
    struct sockaddr_in addr;
    int fd;
};
struct SockInfo infos[512];//support for max 512 clients
void* working(void* arg);

int main(){
    //create socket for monitor
    int fd=socket(AF_INET,SOCK_STREAM,0);//AF_INET means ipv4, soket_stream and 0  means tcp
    if(fd==-1){
        perror("socket");//use to print whats wrong with .h or sys call
        return -1;
    }
    //initial SockInfo
    int max=sizeof(infos)/sizeof(infos[0]);
    for(int i=0;i<max;++i){
        bzero(&infos[i],sizeof(infos[i]));
        infos[i].fd=-1;
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
   
    while(1){
        struct SockInfo* pinfo;
        for(int i=0;i<max;++i){
            if(infos[i].fd==-1){
                pinfo=&infos[i];
                break;
            }
        }
        
        socklen_t addrlen = sizeof(pinfo->addr);

        int cfd=accept(fd,(struct sockaddr*)&pinfo->addr,&addrlen);// return socket for message,if no client to connect it will stuck
        pinfo->fd=cfd;

        if(cfd==-1){
            perror("accept");
            break;
        }
        pthread_t tid;
        pthread_create(&tid,NULL,working,pinfo);
        pthread_detach(tid);   
    }
    close(fd);
    return 0;
}

void* working(void* arg){
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
    close(pinfo->fd);//based on the fd to close the  socket 
    pinfo->fd=-1;
    return NULL;  
}
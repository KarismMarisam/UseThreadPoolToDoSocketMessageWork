#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>

int main(){
    //create socket for message
    int fd=socket(AF_INET,SOCK_STREAM,0);//AF_INET means ipv4, soket_stream and 0  means tcp
    if(fd==-1){
        perror("socket");//use to print whats wrong with .h or sys call
        return -1;
    }
    //link server
    struct sockaddr_in saddr;
    saddr.sin_family=AF_INET;
    saddr.sin_port=htons(9999);//host to net short,link to server port 9999
    inet_pton(AF_INET,"192.168.126.128",&saddr.sin_addr.s_addr);//transform ip address from little en-dian to big en-dian
    int ret=connect(fd,(struct sockaddr*)&saddr,sizeof(saddr));
    if(ret==-1){
        perror("connect");
        return -1;
    }
    
    int number=0;

    //message   
    while(1){
        char buff[1024];
        sprintf(buff,"hello world,%d...\n",number++);
        send(fd,buff,strlen(buff)+1,0);
        memset(buff,0,sizeof(buff));
        int len=recv(fd,buff,sizeof(buff),0);
        if(len>0){
            printf("server say:%s\n",buff);
        }
        else if(len==0){
            printf("server already cut off link");
            break;
        }
        else{
            perror("recv");
            break;
        }
    sleep(1);
    }
    //close fd
    close(fd);
    return 0;  
}

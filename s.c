#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/types.h>
//type

#include <netinet/in.h>
#include <arpa/inet.h>      //sockaddr_in
#include <unistd.h> //fork;

#define SERVER_PORT 12138
#define lislen 10
#define Max_buf 4096

int main(int argc,char *argv[])
{
    struct sockaddr_in ssockaddr,csockaddr;
    
    int sockfd;
    int clientfd;
    
    char recvbuf[Max_buf], sendbuf[Max_buf];
    int recvsize,sendsize;
    int pid =0;
    
    if(argc!=1)
    {
        perror("use: ./news\n");
        exit(-1);
    }

    //建立套接字
    if((sockfd=socket(AF_INET,SOCK_STREAM,0))==-1)
    {
        perror("fail to establish a socket");
        exit(-1);
    }
    printf("Success to establish a socket ...\n");
    
    //初始化服务器套接字地址
    ssockaddr.sin_family=AF_INET;
    ssockaddr.sin_port=htons(SERVER_PORT);
    ssockaddr.sin_addr.s_addr=htonl(INADDR_ANY);
    
    //???
    int on =1;
    setsockopt(sockfd,SOL_SOCKET,SO_REUSEADDR,&on,sizeof(on));
    
    //bind()函数通过给一个未命名套接口分配一个本地名字来为套接口建立本地捆绑（主机地址/端口号）。
    if(bind(sockfd,(struct sockaddr *)&ssockaddr,sizeof(struct sockaddr))==-1)
    {
        perror("fail to bind\n");
        exit(-1);
    }
    printf("Success to bind the socket...\n");
    
    //
    if(listen(sockfd,lislen)==-1)
    {
        perror("fail to listen");
        exit(-1);
    }
    
    //接受客户的请求???
    int a=0;
    if((clientfd=accept(sockfd,(struct sockaddr*)& csockaddr,&a))==-1)
    {
        perror("fail to accept");
        exit(-1);
    }
    printf("Success to accept a connection request...\n");
    
    printf("%s join in!\n",inet_ntoa(csockaddr.sin_addr));//??? 
    
    if((pid=fork())<0)
    {
        perror("fail to fork");
    }
    else if(pid>0)
    {
        while(1)
        {
            fgets(sendbuf,Max_buf,stdin);
            if((sendsize=send(clientfd,sendbuf,strlen(sendbuf),0))!=strlen(sendbuf))
            {
                perror("fail to send datas");
                exit(1);
            }
            memset(sendbuf,0,sizeof(sendbuf));
        }

    }
    else
    {
        while(1)
            { 
                
                int recvSize;
                if((recvSize=recv(clientfd,recvbuf,Max_buf,0))==-1)
                {
                    perror("fail to receive datas");
                    exit(1);
                }
            printf("Client:%s\n",recvbuf);
            memset(recvbuf,0,Max_buf); 
        }
    }        
    return 0;
}





















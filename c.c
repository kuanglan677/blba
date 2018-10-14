#include <stdio.h>
#include <stdlib.h> //exit()
#include <errno.h>
#include <string.h>

#include <sys/socket.h>
#include<sys/types.h>
#include <netdb.h>          //gethostbyname

#include <netinet/in.h>
#include <arpa/inet.h>      //sockaddr_in

#include <unistd.h> //fork;

#define Max_buf 4096
#define SERVER_PORT 12138   //rand short_unsigned_int

int main(int argc,char **argv)
{
    char sendbuf[Max_buf],recvbuf[Max_buf];
    int sendsize,recvsize;
    char username[32];
    char *p;
    
    struct hostent *host;
    struct sockaddr_in servaddr;
    
    int sockfd;
    int pid;
    
    if(argc!=3)
    {
        //perror函数只是将你输入的一些信息和现在的errno所对应的错误一起输出。
        perror("usage: ./news [hostname] [username]");
        exit(-1);
    }
    
    //将username 保存 输出
    p=username;
    strcpy(p,argv[2]);
    printf("username:%s\n",username);
    
    //获取主机地址
    host=gethostbyname(argv[1]);
    if(host==NULL)
    {
        perror("fail to get host by name.");
        exit(-1);
    }
    printf("Success to get host by name...\n");
    
    //创建socket;
    //socket这个函数建立一个协议族为AF_INET、协议类型为SOCK_STREAM、协议编号为0的套接字文件描述符。如果函数调用成功，会返回一个标识这个套接字的文件描述符，失败的时候返回-1。
    if((sockfd=socket(AF_INET,SOCK_STREAM,0))==-1)//AF_INET IPv4 SOCK_STREAM TCP 流式连接 
    {
        perror("fail to establish a socket");
        exit(-1);
    }
    
    //初始化网络套接字地址
    servaddr.sin_family=AF_INET;                            //规定使用地址簇IPv4
    servaddr.sin_port=htons(SERVER_PORT);                   //16位端口号
    servaddr.sin_addr=*((struct in_addr *)host->h_addr);    //32位IP地址
    //bzero(&(servaddr.sin_zero),8);
    
    //
    if(connect(sockfd,(struct sockaddr *)&servaddr,sizeof(struct sockaddr_in))==-1)
    {
        perror("fail to connect the socket");
        exit(1);    
    }
    printf("Success to connect the socket...\n");
    
    send(sockfd,username,sizeof(username),0);
    
    if((pid=fork())<0)
    {
        perror("fail to fork");
        exit(-1);
    }

    else if(pid>0)
    {
        while(1)
        {
            //printf("you: ");
            fgets(sendbuf,Max_buf,stdin);
            (send(sockfd,sendbuf,sizeof(sendbuf),0)==-1);
            memset(sendbuf,0,sizeof(sendbuf));
        }

    }
    else
    {
        while(1)
        {
            if(recv(sockfd,recvbuf,Max_buf,0)==-1)
            {
                perror("Server maybe shutdown");
                exit(0);
            }
            printf("%s\n",recvbuf);
            memset(recvbuf,0,Max_buf); 
        }
    }        

    
    return 0;
}

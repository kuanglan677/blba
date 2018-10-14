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

#include <sys/select.h>
#include <sys/time.h>

#define SERVER_PORT 12138
#define lislen 10
#define Max_buf 4096

int MAX(int a ,int b)
{
    return (a>b)?a:b;
}

void getname(char s[],char rt[])
{
    for(int i=1;s[i]!=' ';i++)
        rt[i-1]=s[i];
} 

int getnum(char rt[],char fdb[][32])
{
    for(int i=0;i<lislen+1;i++)
    {
       if(strcmp(fdb[i],rt)==0)
           return i;
    }
    return -1;
}

int main(int argc,char *argv[])
{
    struct sockaddr_in ssockaddr,csockaddr;
    
    int sockfd;
    int clientfd;
    
    char recvbuf[Max_buf], sendbuf[Max_buf];
    int recvsize,sendsize;
    int pid =0;
    struct timeval timeout;
    
    fd_set servfd,recvfd;
    
    int fd_A[lislen+1];
    char fd_B[lislen+1][32];
    int fd_C[lislen+1];
    
    char name[32];
    
    
    int cnum;
    int max_servfd,max_recvfd;
    
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
    
    timeout.tv_sec=1;
    timeout.tv_usec=0;
    
    
    //接受客户的请求???
    socklen_t a=sizeof(csockaddr);
    
    FD_ZERO(&servfd);
    FD_ZERO(&recvfd);
    FD_SET(sockfd,&servfd);
    cnum=0;
    
    max_servfd=sockfd;
    max_recvfd=0;
    
    
    memset(fd_A,0,sizeof(fd_A));
    memset(fd_B,0,sizeof(fd_B));
    memset(fd_C,-1,sizeof(fd_C));
    while(1)
    {
        FD_ZERO(&servfd);
        FD_ZERO(&recvfd);
        FD_SET(sockfd,&servfd);
        
        switch(select(max_servfd+1,&servfd,NULL,NULL,&timeout))
        {
            case -1://失败
                perror("select error");
                break;
            case 0://规定时间内没有任何描述符有数据
                break;
            default:
                if(FD_ISSET(sockfd,&servfd))
                {
                        if((clientfd=accept(sockfd,(struct sockaddr*)&csockaddr,&a))==-1)
                        {
                            perror("fail to accept");
                            exit(-1);
                        }
                        printf("Success to accept a connection request...\n");
                        printf(">>>>>%s:%d join in!\n",inet_ntoa(csockaddr.sin_addr),ntohs(csockaddr.sin_port));//??? 
                        
                        
                        
                    recv(clientfd,(char *)&name,sizeof(name),0);
                    printf("客户名是：%s\t对应编号为：%d\n",name,clientfd);
                    
                    memset(recvbuf,0,sizeof(recvbuf));
                    
                    fd_A[cnum]=clientfd;
                    max_recvfd=MAX(max_recvfd,clientfd);
                    strcpy(fd_B[cnum],name);
                    cnum++;
                        
                }
                break;
        }
        for(int i=0;i<lislen;i++)
        {
            if(fd_A[i]!=0)
            {
                FD_SET(fd_A[i],&recvfd);
            }
        }
        switch(select(max_recvfd+1,&recvfd,NULL,NULL,&timeout))
        {
            case -1:
                break;
            case 0:
                break;
            default:
                for(int i=0;i<cnum;i++)
                {
                    if(FD_ISSET(fd_A[i],&recvfd))
                    {
                        if((recvsize=recv(fd_A[i],recvbuf,Max_buf,0))==-1||recvsize==0)
                        {
                            printf("fd %dclose\n",fd_A[i]);
                            FD_CLR(fd_A[i],&recvfd);
                            fd_A[i]=0;
                            
                            if(fd_C[i]!=-1)
                            {fd_C[fd_C[i]]=-1;fd_C[i]=-1;}
            
                            //strcpy(fd_B[i],NULL);
                        }
                        else
                        {
                            if(recvbuf[0]=='@')//私聊
                            {
                                char rt[10];//名字
                                memset(rt,0,10);
                                getname(recvbuf,rt);
                                int n;
                                if((n=getnum(rt,fd_B))==-1)//没有该人
                                {
                                    send(fd_A[i],"服务器：没有该用户或者该用户已下线",strlen("服务器：没有该用户或者该用户已下线"),0);
                                    printf("%s %d\n",rt,n);
                                    break;
                                }printf("%s %d\n",rt,n);
                                
                                int aaa=0;
                                if(fd_C[n]==-1){}
                                else if(fd_C[n]==i){}
                                else
                                {
                                    strcpy(sendbuf,"服务器：该用户正在与其他用户");
                                    strcat(sendbuf,fd_B[fd_C[n]]);
                                    strcat(sendbuf,"通信ing，请稍后联系");
                                    send(fd_A[i],sendbuf,strlen(sendbuf),0);
                                    
//                                     strcpy(sendbuf,"服务器：");
//                                     strcat(sendbuf,rt);
//                                     strcat(sendbuf,"现在处于空闲状态");
//                                     send(fd_A[i],sendbuf,strlen(sendbuf),0);
                                    
                                    aaa=1;
                                    //send(fd_A[i],fd_B[fd_C[n]],strlen(fd_B[fd_C[n]]),0);
                                }
                                if(aaa==1) break;
                                
                                
                                if(fd_C[i]==-1)
                                {
                                    fd_C[i]=n;fd_C[n]=i;
                                    strcpy(sendbuf,fd_B[i]);
                                    strcat(sendbuf," 对你私信：");
                                    strcat(sendbuf,recvbuf);
                                    send(fd_A[n],sendbuf,strlen(sendbuf),0);
                                }
                                else if(fd_C[i]==n)
                                {
                                    strcpy(sendbuf,fd_B[i]);
                                    strcat(sendbuf," 对你私信：");
                                    strcat(sendbuf,recvbuf);
                                    send(fd_A[n],sendbuf,strlen(sendbuf),0);
                                }
                                else
                                {
                                    strcpy(sendbuf,"服务器：你现在正与");
                                    strcat(sendbuf,fd_B[fd_C[i]]);
                                    strcat(sendbuf,"私聊，若想退出 请输入“!”");
                                    send(fd_A[i],sendbuf,strlen(sendbuf),0);
                                }
                                memset(sendbuf,0,Max_buf);
                            }
                            else if(recvbuf[0]=='!')//取消私聊
                            {
                                if(fd_C[i]==-1)
                                {
                                    send(fd_A[i],"服务器：你现在尚无进行私聊，无需取消",strlen("服务器：你现在尚无进行私聊，无需取消"),0);
                                }
                                else
                                {
                                    send(fd_A[fd_C[i]],recvbuf,strlen(recvbuf),0);
                                    memset(recvbuf,0,Max_buf);
                                    send(fd_A[fd_C[i]],"服务器：你已被请求取消私聊",strlen("服务器：你已被请求取消私聊"),0);
                                    
                                    
                                    strcpy(sendbuf,"广播：");
                                    strcat(sendbuf,fd_B[fd_C[i]]);
                                    strcat(sendbuf,"和");
                                    strcat(sendbuf,fd_B[i]);
                                    strcat(sendbuf,"刚刚结束了私聊，快去找他们聊天吧～");
                                    for(int j=0;j<lislen;j++)
                                    {
                                        if(fd_A[j]!=0&&i!=j&&j!=fd_C[i])
                                        {
                                            send(fd_A[j],sendbuf,strlen(sendbuf),0)!=strlen(sendbuf);
                                        }
                                    }
                                    

                                    fd_C[fd_C[i]]=-1;fd_C[i]=-1;
                                    send(fd_A[i],"服务器：你已成功取消私聊",strlen("服务器：你已成功取消私聊"),0);
                                }
                            }
                            else
                            {
                                strcpy(sendbuf,fd_B[i]);
                                strcat(sendbuf,":");
                                strcat(sendbuf,recvbuf);
                                printf("数据是:%s\n",sendbuf);
                            
                                //获得私聊名字
                            
                                for(int j=0;j<lislen;j++)
                                {
                                    if(fd_A[j]!=0&&i!=j)
                                    {
                                    
                                        printf("数据发往%d，",fd_A[j]);
                                        if(send(fd_A[j],sendbuf,strlen(sendbuf),0)!=strlen(sendbuf))
                                        {
                                            perror("fail");exit(-1);
                                        }
                                        else
                                        {
                                            printf("Success\n");
                                        }
                                    }
                                }
                                memset(recvbuf,0,Max_buf);
                                memset(sendbuf,0,Max_buf);
                            }
                        }
                    }
                }
                break;
        }
    }
    
    return 0;
}

#include <iostream>
#include <sys/socket.h>
#include <sys/select.h>
#include <arpa/inet.h>
#include <thread>
#include <sys/types.h>
#include <netinet/in.h>
#include <unistd.h>
#include <netdb.h>
#include <string.h>
#include <fcntl.h>
#include <sys/epoll.h>
#define MAXDATASIZE 10240
#define POLL_SIZE	10240
using namespace std;
typedef struct clientinfo
{
    unsigned char state=0;
    int fd;
    long t;
    struct clientinfo* n=NULL;
} CLIENTINFO,*PCLIENTINFO;

int main(int argc,char**argv){
    PCLIENTINFO clients=PCLIENTINFO(calloc(sizeof(CLIENTINFO),POLL_SIZE));
	int port=7901;
	unsigned char buf[MAXDATASIZE];
	char tbuf[256];
	int dl=0;
	if(argc>0){
		try{
			sscanf(argv[0],"%d",&port);
		}catch(...){
			
		}
	}
	cout<<"listen 0.0.0.0:"<<port<<endl;
	int sockfd = socket( AF_INET, SOCK_STREAM, 0 );
	if(sockfd==-1){
		
	}else{
		gethostname(tbuf,sizeof(tbuf));
		cout<<tbuf<<endl;
		struct hostent *he = gethostbyname("0.0.0.0");
		struct sockaddr_in saddr;
		saddr.sin_family = AF_INET;
		saddr.sin_port = htons( port );
		inet_aton("0.0.0.0",&(saddr.sin_addr));
		bzero( &(saddr.sin_zero), 8 );
		if(fcntl(sockfd,F_SETFL,fcntl(sockfd,F_GETFL)|O_NONBLOCK)<0){
			return 0;
		}
		if( bind( sockfd, (sockaddr*)&saddr, sizeof( saddr ) ) == -1 ){
			return 0;
		}
		if(listen(sockfd,20)==-1){
			return 0;
		}
		cout<<"start working"<<endl;
		int epfd = epoll_create(POLL_SIZE);
		int nfds;
		struct epoll_event ev;
		ev.data.fd=sockfd;
		ev.events=EPOLLIN|EPOLLET;
		epoll_ctl(epfd,EPOLL_CTL_ADD,sockfd,&ev);
		struct sockaddr_in aaddr;
		socklen_t aaddrlen;
		for(;;){
			struct epoll_event events[20];
			nfds = epoll_wait(epfd, events, 20, 500);
			for (int n = 0; n < nfds; ++n) {
                int cfd=events[n].data.fd;
				if (cfd== sockfd) {
					aaddrlen=sizeof(aaddr);
					int nfd=accept(sockfd,(struct sockaddr *)&aaddr,&aaddrlen);
					cout<<"accept"<<" "<<nfd<<" "<<inet_ntoa(aaddr.sin_addr)<<":"<<ntohs(aaddr.sin_port)<<endl;
                    if(nfd>0){
                        fcntl(nfd,F_SETFL,fcntl(nfd,F_GETFL)|O_NONBLOCK);
                        ev.data.fd=nfd;
                        epoll_ctl(epfd,EPOLL_CTL_ADD,nfd,&ev);
                    }
                }else if(events[n].events&EPOLLIN==EPOLLIN){
                    int rl=recv(cfd,buf,MAXDATASIZE,0);
                    if(rl<=0){
                        close(cfd);
                    }else{
                        buf[rl]=0;
                        cout<<cfd<<" "<<buf<<endl;
                    }
                }else{
					cout<<events[n].data.fd <<endl;
				}
			}
		}
	}
	return 0;
}

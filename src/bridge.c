#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <poll.h>

#ifndef __linux
#include <net/bpf.h>
#endif

#include "main.h"

#include "daemon.h"
#include "socket.h"
#include "packet.h"
#include "log.h"



typedef struct {
	char	ifname[64];
	int		socket;
} DEVICE;

//使用するインタフェース２つ
#ifndef __linux
DEVICE Device[2] = {
	{"em0"},
	{"em1"},
};
#else
DEVICE Device[2] = {
	{"eth0"},
	{"eth1"},
};
#endif


int AnalyzePacket(int deviceNo, u_char *data, int size){
	if(size < sizeof(struct ether_header *)){
		InfoLog("not ehter_header[%d]:lest=%d",deviceNo,size);
		return -1;
	}
	PrintPacket(data);
	return 0;
}

void ReadPacket(int socket, int deviceNo){
	int size = 0;
	u_char buf[8192];

#ifndef __linux
	size = read(socket, buf, sizeof(buf));
	if(size <= 0){
		ErrorLog("read");
	}else{
		InfoLog("--------size=%d,deviceNo=%d",size,deviceNo);

		int bpf_len=size;
		struct bpf_hdr *bp;
		void *data_pointer;
		int data_size;
		int one_size;
		bp = (struct bpf_hdr *)buf;

		while(bpf_len > 0){
			data_pointer = (char *)bp + bp->bh_hdrlen;
			data_size = bp->bh_caplen;

			if(AnalyzePacket(deviceNo, data_pointer, data_size) != -1){
				if((size=write(Device[(!deviceNo)].socket, data_pointer, data_size)) <= 0){
					ErrorLog("write");
				}
			}
			one_size = BPF_WORDALIGN(bp->bh_hdrlen + bp->bh_caplen);
			bpf_len -= one_size;
			bp = (struct bpf_hdr *)((void *)bp + one_size);
		}
	}
#else
	size = read(socket, buf, sizeof(buf));
	if(size <= 0){
		ErrorLog("read");
	}else{
		if(AnalyzePacket(deviceNo, buf, size) != -1){
			if((size=write(Device[(!deviceNo)].socket, buf, size)) <= 0){
				ErrorLog("write");
			}
		}
	}
#endif
}

void Bridge(){
	struct pollfd	targets[2];
	int nready, i;

	targets[0].fd = Device[0].socket;
	targets[0].events = POLLIN|POLLERR;
	targets[1].fd = Device[1].socket;
	targets[1].events = POLLIN|POLLERR;

	while(1){
		switch(nready = poll(targets,2,100)){
		case	-1:
			if(errno != EINTR){
				ErrorLog("poll");
				exit(EXIT_FAILURE);
			}
			break;
		case	0:
			break;
		default:
			for(i=0; i<=1; i++){
				if(targets[i].revents&(POLLIN|POLLERR)){
					ReadPacket(targets[i].fd,i);
				}
			}
			break;
		}
	}

}


void StartService(){
	int i;
	for(i=0; i<2; i++){
		if((Device[i].socket = InitRawSocket(Device[i].ifname, 1, 0)) == -1){
			ErrorLog("InitRawSocket Error");exit(EXIT_FAILURE);
		}
		InfoLog("socket%d=%d",i,Device[i].socket);
	}

	InfoLog("start bridge");
	Bridge();
}

void StopService(){
	InfoLog("stop bridge");

	int i;
	for(i=0; i<2; i++){
		close(Device[i].socket);
	}
}

int main(int argc, char *argv[], char *envp[]){
	char *mode;

	chdir(SERVER_ROOT);

	if(argc == 2){
		mode = argv[1];
		if(strcmp(mode,"start")==0){
			StartServer();
		}else if(strcmp(mode,"stop")==0){
			StopServer();
		}else if(strcmp(mode,"restart")==0){
			StopServer();
			StartServer();
		}else{
			printf("usage: start stop restart\n");
			exit(EXIT_FAILURE);
		}
	}else{
		printf("usage: start stop restart\n");
		exit(EXIT_FAILURE);
	}

	return 0;
}

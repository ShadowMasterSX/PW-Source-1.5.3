#ifndef __CM_LIB_NET__
#define __CM_LIB_NET__

#ifdef __cplusplus
extern "C" {
#endif

int bind_and_listen(unsigned short port,int qsize);
int bind_and_listen_un(const char * file,int qsize); //UNIX 

int write_fd(int fd, void *ptr, size_t nbytes, int sendfd);	//UNIX socket
int read_fd(int fd, void *ptr, size_t nbytes, int *recvfd);	//UNIX socket

#ifdef __cplusplus
}
#endif

#endif

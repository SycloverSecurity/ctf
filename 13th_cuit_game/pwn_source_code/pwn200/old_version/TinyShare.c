#include <arpa/inet.h>
#include <ctype.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#define BUFFER_SIZE 4096
#define SERVER_NAME "EasyShare"

#define USAGE                                                                  \
  "Usage:\n"                                                                   \
  "    tinyshare <port> <directory>\n"


void log_func(int log_count,const char *content){
	printf("[*]Log(%d):\n%s\n",log_count,content);
}

struct args {
  int port;
  const char *workdir;
};

void errexit(const char *errtext) {
  perror(errtext);
  exit(0);
}

int sendf(int csock, const char *filename) {
  FILE *fp;
  char *sendbuff = (char *)malloc(0x100);
  int ret_length = 0;
  if ((fp = fopen(filename, "r")) != NULL) {
    ret_length = snprintf(sendbuff, 0x100,
    				  "HTTP/1.1 200 OK\r\n"
                      "Server: tinyfs\r\n"
                      "Filename: %s\r\n"
                      "Content-Type: application/octet-stream\r\n\r\n",
                       filename);
    //heap info leak
    write(csock, sendbuff, ret_length);
    fgets(sendbuff, sizeof(sendbuff), fp);
    while (!feof(fp)) {
      write(csock, sendbuff, strlen(sendbuff));
      fgets(sendbuff, sizeof(sendbuff), fp);
    }
    fclose(fp);
    return 1;
  } else {
    ret_length = snprintf(sendbuff, 0x100,
    				  "HTTP/1.1 404 Not Found\r\n"
                      "Server: tinyshare\r\n"
                      "Filename: %s\r\n"
                      "Content-Type: text/html\r\n\r\n"
                      "<p>404 Not Found</p>",
                      filename);
    //heap info leak
    write(csock, sendbuff, ret_length);
    return -1;
  }
}

int doclient(int csock, const char *workdir) {
  char httppath[BUFFER_SIZE];
  char filepath[BUFFER_SIZE];
  char workdirtmp[BUFFER_SIZE];
  char recvbuff[BUFFER_SIZE];
  memset(recvbuff, 0, BUFFER_SIZE);
  memset(workdirtmp, 0, BUFFER_SIZE);
  strncpy(workdirtmp, workdir, BUFFER_SIZE - 1);
  read(csock, recvbuff, BUFFER_SIZE - 1);
  //log 1
  log_func(1,recvbuff);
  if (sscanf(recvbuff, "GET %s HTTP", httppath) != 1)
    return -1;
  if (workdirtmp[strlen(workdirtmp) - 1] == '/')
    workdirtmp[strlen(workdirtmp) - 1] = '\0';
  strcpy(filepath, workdirtmp);
  strcat(filepath, httppath);
  printf("File:%s\n", filepath);
  return sendf(csock, filepath);
}

void mainloop(int ssock, const char *workdir) {
  struct sockaddr_in caddr;
  socklen_t addrLen = sizeof(caddr);
  int csock;
  pid_t pid;
  while (1) {
    if ((csock = accept(ssock, (struct sockaddr *)&caddr, &addrLen)) == -1)
      errexit("accept() error");
    pid = fork();
    if (pid == 0) {
      // child
      doclient(csock, workdir);
      exit(0);
    } else if (pid > 0) {
      close(csock);
    } else {
      errexit("fork() error");
    }
  }
}

int initserver(int port) {
  int ssock;
  struct sockaddr_in saddr;
  if ((ssock = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    errexit("socket() error");
  int opt = SO_REUSEADDR;
  setsockopt(ssock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
  memset(&saddr, 0, sizeof(saddr));
  saddr.sin_family = AF_INET;
  saddr.sin_port = htons(port);
  saddr.sin_addr.s_addr = htonl(INADDR_ANY);
  if (bind(ssock, (struct sockaddr *)&saddr, sizeof(saddr)) == -1)
    errexit("bind() error");
  if (listen(ssock, 5) == -1)
    errexit("listen() error");
  return ssock;
}

void parseargs(int argc, char *argv[], struct args *argsptr) {
  if (argc != 3) {
    fprintf(stderr, USAGE);
    exit(0);
  }
  sscanf(argv[1], "%d", &argsptr->port);
  argsptr->workdir = argv[2];
}

int main(int argc, char *argv[]) {
  struct args args;
  parseargs(argc, argv, &args);
  mainloop(initserver(args.port), args.workdir);
  return 0;
}

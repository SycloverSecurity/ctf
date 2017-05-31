#include <arpa/inet.h>
#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#define BUFFER_SIZE 1024
#define MSG_SIZE 128
#define NAME_SIZE 32
#define FILENAME_SIZE 128

#define CHECK_OK 0x110
#define CHECK_ILLEGAL 0x111

int leave_msg_flag = 0;

struct ARG {
  const char *workdir;
  int port;
};

int csock;
char name[NAME_SIZE] = "guest";

void init_work(void) {
  setbuf(stdin, NULL);
  setbuf(stdout, NULL);
  // alarm(120);
}

void sendflag() {
  int fd;
  char buffer[32];
  fd = open("./flag.txt", O_RDONLY);
  read(fd, buffer, 32);
  write(csock, buffer, strlen(buffer));
}

void errexit(const char *errtext) {
  perror(errtext);
  exit(0);
}

void menu(int fd) {
  write(fd, "1. Login\n", strlen("1. Login\n"));
  write(fd, "2. Get File\n", strlen("2. Get File\n"));
  write(fd, "3. Leave a message\n", strlen("3. Leave a message\n"));
}

void welcome_info(int fd) {
  write(fd, "Welcome to my file share server.\n",
        strlen("Welcome to my file share server.\n"));
  write(fd, "\tI think it is safe.Really?\n",
        strlen("\tI think it is safe.Really?\n"));
  write(fd, "\tHHHHH...exploit me.\n", strlen("\tHHHHH...exploit me.\n"));
}

int read_int(int fd) {
  char buffer[10] = {0};
  read(fd, buffer, 10);
  return atoi(buffer);
}

int saferead(int fd, char *buffer, int count, char spl) {
  int i = 0;
  char ch;
  while (i < count) {
    read(fd, &ch, 1);
    if (ch == spl) {
      ch = '\0';
      buffer[i] = ch;
      break;
    }
    buffer[i] = ch;
    i++;
  }
  return i;
}

int login(int fd_in, int fd_out) {
  char tmp[NAME_SIZE];
  memset(tmp, 0, NAME_SIZE);
  write(fd_out, "Name:", strlen("Name:"));
  // bof
  // read(fd_in, tmp, 128);
  saferead(fd_in, tmp, 128, '\n');
  strncpy(name, tmp, NAME_SIZE);
}

int check(char *filename, const char *workdir) {

  if (strstr(filename, "flag")) {
    return CHECK_ILLEGAL;
  } else if (strstr(filename, "..")) {
    return CHECK_ILLEGAL;
  }
  return CHECK_OK;
}

int read_file(int csock, const char *workdir) {
  char *info_ret = (char *)malloc(sizeof(char) * BUFFER_SIZE);
  char filename[FILENAME_SIZE] = {0};
  int ret_length;
  write(csock, "input the filename:", strlen("input the filename:"));
  read(csock, filename, FILENAME_SIZE);
  if (CHECK_OK == (check(filename, workdir))) {
    // yes
    ret_length = snprintf(info_ret, 0x100, "\tFile:%s\n"
                                           "\tSorry,I can't send file to you.\n"
                                           "\tHave fun with my share\n",
                          filename);
    // heap info leak
    write(csock, info_ret, ret_length);
  } else if (CHECK_ILLEGAL == (check(filename, workdir))) {
    // illegal
    ret_length = snprintf(info_ret, 0x100, "\tFile:%s\n"
                                           "\tToo young Too simple.\n"
                                           "\tHave fun with my share\n",
                          filename);
    // heap info leak
    write(csock, info_ret, ret_length);
  } else {
    ret_length = snprintf(info_ret, 0x100, "\tFile:%s\n"
                                           "\t:( Server is unhappy\n"
                                           "\tHave fun with my share\n",
                          filename);
    // heap info leak
    write(csock, info_ret, ret_length);
  }
}

int leave_message(int csock) {
  char info[MSG_SIZE] = {0};
  char *show = (char *)malloc(MSG_SIZE * 4);
  if (!leave_msg_flag) {

    write(csock, "Input your msg:", strlen("Input your msg:"));
    read(csock, info, MSG_SIZE);
    // fmt vuln
    sprintf(show, info);
    write(csock, "\nOk,I get your msg:", strlen("\nOk,I get your msg:"));
    write(csock, show, strlen(show));
    leave_msg_flag = 1;
  } else {
    write(csock, "Leave msg finished\n", strlen("Leave msg finished\n"));
  }
}

void doclient(int csock, const char *workdir) {
  int opt = -1;
  char info[NAME_SIZE + 8] = {0};
  while (1) {
    menu(csock);
    sprintf(info, "--> %s ", name);
    write(csock, info, strlen(info));
    opt = read_int(csock);
    switch (opt) {
    case 1:
      login(csock, csock);
      break;
    case 2:
      read_file(csock, workdir);
      break;
    case 3:
      leave_message(csock);
      break;
    default:
      write(csock, "Wrong opt!\n", strlen("Wrong opt!\n"));
      break;
    }
  }
}

int init_server(int port) {
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

void mainloop(int ssock, const char *workdir) {

  struct sockaddr_in caddr;
  socklen_t addrLen = sizeof(caddr);

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

int main(int argc, char *argv[]) {

  init_work();
  if (argc != 3) {
    printf("Ussage:%s workdir port", argv[0]);
    return 1;
  }
  struct ARG *arg = (struct ARG *)malloc(sizeof(struct ARG));
  arg->workdir = argv[1];
  arg->port = atoi(argv[2]);

  mainloop(init_server(arg->port), arg->workdir);
  return 0;
}
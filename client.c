#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <string.h>
#include <errno.h>
#include <sys/select.h>

// hàm này đọc dữ liệu từ stdin cho đến khi gặp ký tự eoc hoặc đạt đến giới hạn maxchars.
int readline(char *buffer, int maxchars, char eoc) {
  int n = 0;
  while(n < maxchars) {
    buffer[n] = getc(stdin);
    if(buffer[n] == eoc)
      break;
    n++;
  }
  return n;
}

int main(int argc, char const *argv[]) {
  int sockfd;
  struct sockaddr_in serv_addr;
  char sendline[1024], recvline[1024];

  // tạo socket
  if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) <= 0) {
    perror("error while creating socket...");
    exit(1);
  }

  // thiết lập địa chỉ của server
  serv_addr.sin_family = AF_INET;
  serv_addr.sin_port = htons(8080);
  if(inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0) { 
    perror("address conversion error...");
    exit(-1);
  } 

  // kết nối đến server
  if(connect(sockfd, (struct sockaddr *) &serv_addr, sizeof serv_addr) < 0) {
    perror("connect error...");
    exit(1);
  }
  
  fd_set waitfds;
  int readyfds;
  while(1) {
    FD_ZERO(&waitfds);

    // thêm socket và stdin vào set waitfds
    FD_SET(sockfd, &waitfds);
    FD_SET(0, &waitfds);

    // khởi tạo lại các buffer
    memset(recvline, 0, 1024);
    memset(sendline, 0, 1024);

    // kiểm tra sự sẵn có của socket và stdin bằng hàm select
    readyfds = select(sockfd + 1, &waitfds, NULL, NULL, NULL);
    if ((readyfds < 0) && (errno != EINTR)) {
      perror("select error");
      exit(1);
    }

    // nếu stdin sẵn có, đọc dữ liệu và gửi đi
    if(FD_ISSET(0, &waitfds)) {
      readline(sendline, 1024, '\n');
      write(sockfd, sendline, strlen(sendline));
    }
    
    // nếu socket sẵn có, đọc dữ liệu và in ra màn hình
    if(FD_ISSET(sockfd, &waitfds)) {
      read(sockfd, recvline, 1024);
      fprintf(stdout, "%s", recvline);
    }
  }

  return 0;
}
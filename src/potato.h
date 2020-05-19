#include <arpa/inet.h>
#include <assert.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <netdb.h>
#include <netinet/in.h>
#include <string>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

struct Metadata{
  char addr[100];
  int port=0;
};
typedef struct Metadata meta;

struct Potato{
  int hops=0;
  int num=0;
  int change=0; // send is 1, recv is 0; for testing
  int path[512];
};
typedef struct Potato pot;

class Server {
public:
  struct addrinfo host_info, *host_info_list;
  int sockfd; // fd for socket
  int newfd; // fd after accept
  int status;

  void initialize(const char *_port) {  //  use getaddrinfo() to initialize
  memset(&host_info, 0, sizeof(host_info));
  host_info.ai_family = AF_UNSPEC;
  host_info.ai_socktype = SOCK_STREAM;
  host_info.ai_flags = AI_PASSIVE;

  status = getaddrinfo(NULL, _port, &host_info, &host_info_list);
  if (status != 0) {
    std::cerr << "Error: cannot get address info for host" << std::endl;
    exit(EXIT_FAILURE);
  }
}


void create_socket() {      // build socket
  sockfd = socket(host_info_list->ai_family, host_info_list->ai_socktype,
                  host_info_list->ai_protocol);
  if (sockfd == -1) {
    std::cout << "cannot create socket" << std::endl;
    exit(EXIT_FAILURE);
  } 

  int yes = 1;
  status = setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));
  status = bind(sockfd, host_info_list->ai_addr, host_info_list->ai_addrlen);
  if (status == -1) {
    std::cout << "cannot bind socket" << std::endl;
    exit(EXIT_FAILURE);
  } 

  status = listen(sockfd, 100);
  if (status == -1) {
    std::cout << "cannot listen on socket" << std::endl;
    exit(EXIT_FAILURE);
  } 

  freeaddrinfo(host_info_list);
}

int buildServer() {   // begin the server, for player
  initialize("");
  struct sockaddr_in *addr_in = (struct sockaddr_in *)(host_info_list->ai_addr);
  addr_in->sin_port = 0;      // ask system to assign a port
  create_socket();
  struct sockaddr_in sin;
  socklen_t len = sizeof(sin);
  if (getsockname(sockfd, (struct sockaddr *)&sin, &len) == -1)
    perror("getsockname error");
  int port= ntohs(sin.sin_port);  // get the port we asked for 
  return port;
}

void buildServer(char *port) {  // begin the server, for master
  initialize(port);
  create_socket();
}

  int accept_connection(std::string &addr) {  //accept the new connection
  struct sockaddr_storage socket_addr;
  socklen_t socket_addr_len = sizeof(socket_addr);
  newfd = accept(sockfd, (struct sockaddr *)&socket_addr, &socket_addr_len);
  if (newfd == -1) {
    std::cerr << "Error: cannot accept connection on socket" << std::endl;
    exit(EXIT_FAILURE);
  } // if
  struct sockaddr_in *curr = (struct sockaddr_in *)&socket_addr;
  addr = inet_ntoa(curr->sin_addr);
  return newfd;
}
 
  virtual ~Server() {
    close(sockfd);
  }
};


#include "potato.h"
#include <algorithm>
#include "assert.h"
class Player:public Server{
public:
  int player_id;
  int num_players;
  int fd_master; // fd to connect master
  int fd_neigh; // fd to be the client to neighbor

  Player(char **argv) {
    connect_master(argv[1], argv[2]);
  }

  void connect_server(const char *hostname, const char *port, int &socket_fd) {
    struct addrinfo host_info;
    struct addrinfo *host_info_list;

    memset(&host_info, 0, sizeof(struct addrinfo));
    host_info.ai_family = AF_UNSPEC;
    host_info.ai_socktype = SOCK_STREAM;

    status = getaddrinfo(hostname, port, &host_info, &host_info_list);
    if (status != 0) {
      std::cout << "cannot get address info for host" << std::endl;
      std::cout << "  (" << hostname << "," << port << ")" << std::endl;
      exit(EXIT_FAILURE);
    } // if

    socket_fd = socket(host_info_list->ai_family, host_info_list->ai_socktype,
                       host_info_list->ai_protocol);
    if (socket_fd == -1) {
      std::cout << "cannot create socket" << std::endl;
      std::cout << "  (" << hostname << "," << port << ")" << std::endl;
      exit(EXIT_FAILURE);
    } // if

    status = connect(socket_fd, host_info_list->ai_addr, host_info_list->ai_addrlen);
    if (status == -1) {
      std::cout << "cannot connect to socket" << std::endl;
      std::cout << "  (" << hostname << "," << port << ")" << std::endl;
      exit(EXIT_FAILURE);
    } // if

    freeaddrinfo(host_info_list);
  }

  void connect_master(const char *hostname, const char *port) {  // connect ringmaster
    connect_server(hostname, port, fd_master); 
    recv(fd_master, &player_id, sizeof(player_id), 0);          // receive player_id
    recv(fd_master, &num_players, sizeof(num_players), 0);      // receive num_players
  }

  void be_server()    // start as a server
  { int listeningPort = buildServer();
    send(fd_master, &listeningPort, sizeof(listeningPort), 0);
    std::cout << "Connected as player " << player_id << " out of "
              << num_players << " total players\n";
  }

  void connect_neigh() {
    meta meta_info;
    memset(meta_info.addr,0,sizeof(meta_info.addr));
    recv(fd_master, &meta_info, sizeof(meta_info), MSG_WAITALL);
    char port[30];// 9
    sprintf(port, "%d", meta_info.port);
    connect_server(meta_info.addr, port, fd_neigh);
    std::string temp;
    accept_connection(temp);
  }
 
  void gameplay()
  { pot potato;// initialize variables
    memset(potato.path, 0, sizeof(potato.path));
    srand((unsigned int)time(NULL)+player_id);
    int recv_size=0;
    int send_size=0;
    fd_set readfds;   
    int range=newfd+1;
    while(1){                               // keep listening
    FD_ZERO(&readfds);  
    FD_SET(newfd,&readfds);
    FD_SET(fd_master,&readfds);
    FD_SET(fd_neigh,&readfds);
    select(range,&readfds,NULL,NULL,NULL);

    if(FD_ISSET(newfd, &readfds))
      {recv_size=recv(newfd,&potato,sizeof(potato),MSG_WAITALL);
	//std::cout<<"potato condition is "<<potato.change<<std::endl;
	potato.change=0;
        if(recv_size==0)
	  {break;}	
      }
    else if(FD_ISSET(fd_master,&readfds))
      { recv_size=recv(fd_master,&potato,sizeof(potato),MSG_WAITALL);
       	//std::cout<<"potato condition is "<<potato.change<<std::endl;
	potato.change=0;
	if(recv_size==0)
	  {break;}
      }
    else if(FD_ISSET(fd_neigh,&readfds))
      { recv_size=recv(fd_neigh,&potato,sizeof(potato),MSG_WAITALL);
	//std::cout<<"potato condition is "<<potato.change<<std::endl;
	potato.change=0;
	if(recv_size==0)
	  {break;}
      }

    if(potato.hops==0)    // game over
      {return;}

   
      potato.hops--;  
      potato.path[potato.num]=player_id;
      potato.num++;    
      
      if(potato.hops==0)    // become it
      { potato.change=1;
	send_size=send(fd_master,&potato,sizeof(potato),0);	
	if(send_size!=sizeof(potato))
	  {std::cout<<"Sending potato size incorrect"<<std::endl;
	    exit(EXIT_FAILURE);}
	std::cout<<"I'm it"<<std::endl;  
      }
      
      else{                 // send potato to random side
      int random = rand() % 2;   
      if(random==0)
	{ potato.change=1;
	  send_size=send(fd_neigh,&potato,sizeof(potato),0);	  
	if(send_size!=sizeof(potato))
	   {std::cout<<"Sending potato size incorrect"<<std::endl;
	    exit(EXIT_FAILURE);}
	else
	  {std::cout<<"Sending potato to "<<(player_id+1)%num_players<<std::endl;}
	}
      else
	{ potato.change=1;
	  send_size=send(newfd,&potato,sizeof(potato),0);
	if(send_size!=sizeof(potato))
	  {std::cout<<"Sending potato size incorrect"<<std::endl;
	    exit(EXIT_FAILURE);}
	else
	  {std::cout<<"Sending potato to "<<(player_id-1+num_players)%num_players<<std::endl;}
	}  
      }
    }    
  }
 
  void exe()
  {
  be_server();
  connect_neigh();
  gameplay();
  }

  virtual ~Player() {
    close(newfd);
    close(fd_master);
    close(fd_neigh);
  }
};

int main(int argc, char *argv[])
{   if (argc < 3) std::cout << "Please print: ./player <machine_name> <port_num>\n";
  Player *play = new Player(argv);
  play->exe();
  delete play;
  return EXIT_SUCCESS;
}

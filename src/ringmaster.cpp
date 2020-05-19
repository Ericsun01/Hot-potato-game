#include "potato.h"
#include "assert.h"
#include <vector>
class Ringmaster:public Server{
public:
  int num_players;
  int num_hops;
  std::vector<int> fd;
  std::vector<std::string> ip;
  std::vector<int> port;

  Ringmaster(char **arg) {
    num_players = atoi(arg[2]);
    num_hops = atoi(arg[3]);
    fd.resize(num_players);
    ip.resize(num_players);
    port.resize(num_players);
    buildServer(arg[1]);          // build the socket and get avaliable fd
  }

  void build_connections() {
    for (int i = 0; i < num_players; i++) {
      fd[i] = accept_connection(ip[i]);
      send(fd[i], &i, sizeof(i), 0);    // send players' id
      send(fd[i], &num_players, sizeof(num_players), 0);  // send num of players
      recv(fd[i], &port[i], sizeof(port[i]), MSG_WAITALL);   // recv listening port of player i
      std::cout << "Player " << i << " is ready to play\n";
    }
  }

  void build_circle() {
    for (int i = 0; i < num_players; i++) {
      meta info;
      memset(info.addr,0,sizeof(info.addr));
      int next_id = (i + 1) % num_players;
      info.port = port[next_id];
      strcpy(info.addr, ip[next_id].c_str());
      send(fd[i], &info, sizeof(info), 0);
    }
  }

  
  void hotpotato()
  {int fd_num=0;
   int send_size=0;     // initialize variables 
   int recv_size=0;
   pot potato;
   potato.hops=num_hops;
  

   if(potato.hops==0)     // judge if we should end game
     {for(int i=0;i<num_players;i++)
	 {  potato.change=1;
	   send_size=send(fd[i],&potato,sizeof(potato),0);
	  
	 if(send_size!=sizeof(potato))
            {std::cout<<"Sending potato size incorrect"<<std::endl;
	    exit(EXIT_FAILURE);}}
       return;
     }
     
   
   int random=rand()%num_players;  // Send potato to random player
   std::cout << "Ready to start the game, sending potato to player " << random
              << "\n";
   
   potato.change=1;
   send_size=send(fd[random],&potato,sizeof(potato),0);
   
   if(send_size!=sizeof(potato))
     {std::cout<<"Sending potato size incorrect"<<std::endl;
	    exit(EXIT_FAILURE);}
   fd_set readfds;
   FD_ZERO(&readfds);
  
   for(int i=0;i<num_players;i++)
     {FD_SET(fd[i],&readfds);}
   int range=newfd+1;
   fd_num=select(range,&readfds,NULL,NULL,NULL);  // listen all players
   assert(fd_num == 1);
   
   for(int i=0;i<num_players;i++)
     {if(FD_ISSET(fd[i],&readfds))
	 { recv_size=recv(fd[i],&potato,sizeof(potato),MSG_WAITALL);
	   potato.change=0;
	   if(recv_size!=sizeof(potato))
	     {std::cout<<"potato size is "<<recv_size<<" , error!"<<std::endl;
	   exit(EXIT_FAILURE);}
	 
         for(int k=0;k<num_players;k++)
	 { potato.change=1;
	   send_size=send(fd[k],&potato,sizeof(potato),0);
	   
	   if(send_size!=sizeof(potato))
           {std::cout<<"Sending potato size incorrect"<<std::endl;
	    exit(EXIT_FAILURE);}
	 }
	 break;
       }   
     } 
   std::cout<<"Trace of potato:"<<std::endl;
   for(int j=0;j<potato.num;j++)      // print the potato trace
     { 
       std::cout<<potato.path[j]<<",";
     }   
  }
  
  void exe()
  { std::cout << "Potato Ringmaster\n";
    std::cout << "Players = " << num_players << "\n";
    std::cout << "Hops = " << num_hops << "\n";
    build_connections();
    build_circle();
    hotpotato();
  }

  
  virtual ~Ringmaster() {
    for (int i = 0; i < num_players; i++) {
      close(fd[i]);
    }
  }
  
};

int main(int argc, char **argv) {
 if (argc < 4)
    std::cout << "Please print: ./ringmaster <port_num> <num_players> <num_hops>\n";
  Ringmaster *ring = new Ringmaster(argv);
  ring->exe();
  delete ring;
  return EXIT_SUCCESS;
}

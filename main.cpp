#include <iostream>
#include <sys/types.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <string.h>
#include <string>
 
using namespace std;

int handleClients();
int listening = -1;

//MARK:- MESSAGE SYSTEM CLASSES
class User {
   
    public:
    
    User(string ip,string port,string name,int socket){
      
        this->port = port;
         this->ip = ip;
        this->name = name;
        this->socket = socket;
        this->groupNum = rand()%2;//In our chat system there are 2 groups
    }
    
    
    User(){
        
    }

    
    string port;
    string ip;
    string name = "";
    int socket = -1;
    int groupNum = 0;
    
    
    string connectionTime = "";
    
    void displaySendMessages(){
        
    }
    
    void displayRecMessages(){
        
    }
    
    
};


class Users{
public:
 User users[10];
    int emptyPos = 0;
    
    //USE THIS METHOD TO ADD A NEW USER
    User addNewUser(string ip,string port,string name,int socket){
        
        User newUser = User(ip, port, name,socket);
        
        users[emptyPos] = newUser;
        emptyPos += 1;
       
        return newUser;
        
    }
    
    
    string getAllUserNames(){
        string result = "";
        for(int i = 0;i<10;++i){
            
            if(users[i].name == ""){
                continue;
            }
            
            result += users[i].name + "Group No. : " + to_string(users[i].groupNum);
            result += "\n";
        }
        
        return result;
    }
 
    User* getUserByPort(string key){
        
        for(int i = 0;i<10;++i){
            
            if(users[i].port == key){
                return &users[i];
            }
            
        }
        
        return NULL;
        
    }
    
    
    User* getUserByName(string name){
           
           for(int i = 0;i<10;++i){
               
               if(users[i].name == name){
                   return &users[i];
               }
               
           }
           
           return NULL;
           
       }
    
    
}SystemUsers;


class MessagesSystem {
    
public:  MessagesSystem(){
        for(int i =0;i<100;++i){
            messages[i] = Message();
        }
    }
   
    class Message {
        
    public:
        
        public :
        
        Message(){
            
        }
        
          Message(string mess,User* sen, User* rec){
              this->message = mess;
                this->sender = sen;
                this->reciever = rec;
              
          }
          
          string message;
          User* sender = NULL;
          User* reciever = NULL;
          
          
          
      };
    
    
    Message messages[100];
    int emptyPos = 0;
    
    
    //USE THIS METHOD TO ADD A MESSAGE
    bool addMessage(string mess,string senPort,string recName){
        //GROUP MESSAGES ARE NOT ADDED IN LOCAL MEMORY
        
        User* sender = SystemUsers.getUserByPort(senPort);
        User* reciever = SystemUsers.getUserByName(recName);
        
        if (sender != NULL && reciever != NULL){
        Message newMessage = Message(mess, sender, reciever);
        
        messages[emptyPos] = newMessage;
            emptyPos += 1;
       
            return true;
            
        }else{
            cout<<"Could not found sender or reciever"<<endl;
                return false;
            }
        
    }
    
    
    
    
   
   

    
    
}SystemMessages;


//MARK:- CODE BELOW
void *handleSingleClient(void *threadid){
    
           
           sockaddr_in client;
            socklen_t clientSize = sizeof(client);
           
       cout<<"Waiting for connection"<<endl;
       int clientSocket = accept(listening, (sockaddr*)&client, &clientSize);
       cout<<"Connection established";
    //Give this task to a new thread
          
               
       char host[NI_MAXHOST];      // Client's remote name
       char service[NI_MAXSERV];   // Service (i.e. port) the client is connect on
    
       memset(host, 0, NI_MAXHOST); // same as memset(host, 0, NI_MAXHOST);
       memset(service, 0, NI_MAXSERV);
    
       if (getnameinfo((sockaddr*)&client, sizeof(client), host, NI_MAXHOST, service, NI_MAXSERV, 0) == 0)
       {
           cout << host << " \nconnected on port " << service << endl;
       }
       else
       {
           inet_ntop(AF_INET, &client.sin_addr, host, NI_MAXHOST);
           cout << host << " connected on port " << ntohs(client.sin_port) << endl;
       }
    
      
    
    //CREATING CLIENT ACCOUNT - Message
    bool accountCreated = false;
    string clientName = "";
    
    string clientHandle1 = "\n\n*********************** WELCOME TO LAN CHAT APPLICATION *********************************\n\n";
    string clientHandle2 = "Here are all the people you may want to send messages or add them to a group and then send messages\n" + SystemUsers.getAllUserNames()+"\n";
    string clientHandle3 = "Type in your desired handle - it will be used by other people to send you message\n\n";
    
    string clientHandle = clientHandle1 + clientHandle2 + clientHandle3;
    
     char *mess = &clientHandle[0];
      send(clientSocket, mess,clientHandle.length(), 0);
     
     
     // While loop: accept and echo message back to client
        char buf[4096];
       char quit[] = {'q','u','i','t'};
     
        while (true)
        {
            memset(buf, 0, 4096);
     
            // Wait for client to send data
            int bytesReceived = recv(clientSocket, buf, 4096, 0);
            if (bytesReceived == -1)
            {
                cout << "Error in recv(). Quitting" << endl;
                break;
            }
     
            if (bytesReceived == 0)
            {
                cout << "Client disconnected " << endl;
                break;
            }
     
            //PRINTING CLIENT MESSAGE
            cout<<string(buf, 0, bytesReceived) <<" :: By "<<host<<" Port "<<service<< endl;
         
            User senderUser = User();
            if (!accountCreated){
                //CREATING USER ACCOUNT
          senderUser  =  SystemUsers.addNewUser(string(host), string(service), string(buf, 0, bytesReceived),clientSocket);
                accountCreated = true;
             
                clientName = senderUser.name;
                
                //NEW USER IS ADDED - SEND NOTIFICATION TO ALL ACITVE USERS
                 string notif = "New User - " +clientName+ " Has Joined The Chat. Press 1 to see the user's list\n";
                for(int i = 0;i<10;++i){
                    int notifSocket = SystemUsers.users[i].socket;
                    if (notifSocket == -1){
                        continue;
                    }
                    send(SystemUsers.users[i].socket,&notif[0],notif.length(),0);
                }
                
                
              
                
            }else{
                //SEND THE MESSAGE
                //User respone is in this format -> message space reciever
                //Parsing the response
                string clientRespone = string(buf, 0, bytesReceived);
                //getting the message
                string message = "";
                int counter = 0;
                    for(int i = 0;i<clientRespone.length();++i){
                    
                        if(clientRespone[i] != '@'){
                            message += clientRespone[i];
                        }else{
                            break;
                        }
                        
                        counter += 1;
                        
                    }
                
                counter += 1;
                if (counter<bytesReceived){
                string recName = string(buf, counter, bytesReceived);
                
                    if (bytesReceived-counter  == 2 ){
                        //Group Message
                         string recMessage = message + " - Group Message By " + clientName;
                        for(int i = 0;i<10;++i){
                                          int notifSocket = SystemUsers.users[i].socket;
                            int groupNum = SystemUsers.users[i].groupNum;
                            string groupNumStr = to_string(groupNum) + "\n";
                                          if (notifSocket == -1 || groupNumStr != recName){
                                              continue;
                                          }
                                          send(SystemUsers.users[i].socket,&recMessage[0],recMessage.length(),0);
                                      }
                        
                        
                    }else{
                        //Personal Message
                        
                        
                SystemMessages.addMessage(message, service, recName);
               User* recUser =  SystemUsers.getUserByName(recName);
                //SEND MESSAGE TO RECIEVER
                cout<<"Sender Name"<<clientName<<endl;
                
                string recMessage = message + " -  By " + clientName;
                  send(recUser->socket, &recMessage[0],recMessage.length(), 0);
                }
                }
                
            }
            
            
            //Check if client wants to close the connection
            string clientReq = string(buf,0,bytesReceived);
            string quitStr = string(quit,0,4);
           quitStr += "\n";
           
            
            if (clientReq.compare(quitStr)==0){
                string closingMess = "Client wants to close the connection. - CLOSING";
                char *mess = &closingMess[0];
                send(clientSocket, mess,closingMess.length(), 0);
                cout<<closingMess<<endl;
                break;
            }
            
            else if (clientReq.compare("1\n")==0){
                //Send all users list
                string newList = SystemUsers.getAllUserNames();
                send(clientSocket,&newList[0],newList.length(),0);
            }
            
            else{
                //here send something to client if you want
              
            }
            
            
          
        }
       
       
       // Close the socket
       close(clientSocket);
       
       
    return 0;
       
}


int main(){
    //MARK:- This program handles multiple clients using multithreading,each client can send messages to some other client of his/her wish, client can also disconnect from server by just typing 'quit' command.
    /*
    
     //MARK:- FEATURES OF THE PROGRAM
     SEND PERSONAL MESSAGES
     SEND GROUP MESSAGES
     GET NOTIFIED OF MESSAGES THEY RECIEVE
     GET NOTIFIED WHENEVER A NEW USER JOINS THE CHAT ROOM
     SIMPLE AND EASY TO USE. JUST @USERNAME/@GROUPNAME TO SEND MESSAGE TO THAT USER OR GROUP - EG - HEY HOW ARE YOU @MESSI
     SEE A LIST OF ALL USERS
     USERS CREATE ACCOUNT WHENEVER THEY JOIN THE CHATROOM
     PROGRAM LOCALLY SAVES USERNAME,IP,PORT,SOCKET,GROUP NAME
     MESSAGE,ITS SENDER,RECIEVER IS ALSO STORED LOCALLY
     ALL THE ABOVE IS ACHEIVED THROUGH CLASSES - SO CAN BE EASILY SAVED TO A FILE
    
    */

    
    
    
     handleClients();
    
    
    

    return 0;
}
 

int handleClients()
{
   
   //WORK OF PARENT THREAD
    // Create a socket
     listening = socket(AF_INET, SOCK_STREAM, 0);
    if (listening == -1)
    {
        cout << "Can't create a socket! Quitting" << endl;
        return -1;
    }
 
    // Bind the ip address and port to a socket
    sockaddr_in hint;
    hint.sin_family = AF_INET;
    hint.sin_port = htons(8080);
    inet_pton(AF_INET, "0.0.0.0", &hint.sin_addr);
 
    bind(listening, (sockaddr*)&hint, sizeof(hint));
 
    // Tell Winsock the socket is for listening
    cout<<"Starting to listening"<<endl;
    listen(listening, SOMAXCONN);
    cout<<"Started listnening"<<endl;
    // Wait for a connection
 
    
 
    //MARK:- Handling multiple client using mutli threading
    int maxClients = 10;
    pthread_t threads[maxClients];
      int rc;
      int i;
      
      for( i = 0; i < maxClients; i++ ) {
       //  cout << "main - : creating thread, " << i << endl;
         rc = pthread_create(&threads[i], NULL,  handleSingleClient, (void *)i);
         
         if (rc) {
            cout << "Error:unable to create thread," << rc << endl;
            exit(-1);
         }
      }
      pthread_exit(NULL);
    
  
    
    
   
 
    return 0;
}


//Done61



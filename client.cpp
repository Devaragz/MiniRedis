#include<bits/stdc++.h>
#include<winsock2.h>
using namespace std;

#define RESET "\033[0m"
#define GREEN "\033[32m"
#define YELLOW "\033[33m"
#define CYAN "\033[36m"
#define RED "\033[31m"
#define HIGHLIGHT "\033[47;30m"

int main(){
    system("chcp 65001>nul");   //Enables micro sybol(µ)

    WSADATA wsaData;
    WSAStartup(MAKEWORD(2,2),&wsaData);

    SOCKET sock=socket(AF_INET,SOCK_STREAM,0);
    sockaddr_in server_addr;
    server_addr.sin_family=AF_INET;
    server_addr.sin_port=htons(8080);
    server_addr.sin_addr.s_addr=inet_addr("127.0.0.1");
    cout<<"Connecting to Miniredis server...\n";
    if(connect(sock,(struct sockaddr*)&server_addr,sizeof(server_addr))<0){
        cout<<RED<<"Connection failed~"<<RESET<<"\n";
        return 1;
    }

    cout<<GREEN<<"Connection successful\n"<<RESET;
    cout<<"Commands: GET <key> | PUT <val> | STRESS | EXIT\n\n";
    string inp;
    char buff[1024]={0};
    while(true){
        cout<<CYAN<<"MiniRedis>"<<RESET;
        getline(cin,inp);
        if(inp.empty()) continue;
        inp+="\n";
        send(sock,inp.c_str(),inp.length(),0);
        if(inp=="EXIT\n") break;
        memset(buff,0,sizeof(buff));
        int byt=recv(sock,buff,1024,0);
        if(byt>0){
            string reply(buff);
            while(!reply.empty() && (reply.back()=='\n' || reply.back()=='\r')){
                reply.pop_back();
            }
            cout<<reply<<"\n";
        }else{
            cout<<RED<<"Server disconnected."<<RESET<<"\n";
            break;
        }
    }
    closesocket(sock);
    WSACleanup();
    return 0;
}
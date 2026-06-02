#include<bits/stdc++.h>
#include<winsock2.h>
#include "lru_cache.h"
using namespace std;
#pragma comment(lib, "ws2_32.lib")

int main(){
    system("chcp 65001>nul");
    cout<<"Starting MiniRedis Server..."<<"\n";
    MYSQL *conn=setupDatabase();

    int capacity;
    cout<<"Enter cache capacity: ";
    cin>>capacity;
    LRUcache cache(capacity,conn);

    // Initialize Work
    WSADATA wsaData;
    if(WSAStartup(MAKEWORD(2,2),&wsaData)!=0){
        cout<<"WSstart failed\n";
        return 1;
    }

    SOCKET server_fd=socket(AF_INET,SOCK_STREAM,0);
    if(server_fd==INVALID_SOCKET){
        cout<<"socket cr failed:"<<WSAGetLastError()<<"\n";
        return 1;
    }

    sockaddr_in server_addr;
    server_addr.sin_family=AF_INET;
    server_addr.sin_addr.s_addr=INADDR_ANY;
    server_addr.sin_port=htons(8080);

    if(bind(server_fd,(struct sockaddr*)&server_addr,sizeof(server_addr))==SOCKET_ERROR){
        cout<<"bind failed"<<WSAGetLastError()<<"\n";
        return 1;
    }
    if(listen(server_fd,3)==SOCKET_ERROR){
        cout<<"listen filed"<<WSAGetLastError()<<"\n";
        return 1;
    }
    cout<<GREEN<<"Server actively listening on Port 8080...\n"<<RESET;

    // Network Server Loop
    while(true){
        SOCKET client_socket=accept(server_fd,nullptr,nullptr);
        string req="";
        while(true){
            char buffer[2]={0};
            int byte_rec=recv(client_socket,buffer,1,0);
            if(byte_rec<=0){break;}
            req+=buffer[0];
            if(buffer[0]=='\n'){
                string command,key_str,val_str;
                stringstream ss(req);
                ss>>command;

                if(command=="GET"){
                    ss>>key_str;
                    int key=stoi(key_str);
                    auto start=chrono::high_resolution_clock::now();  //Timer starts
                    int r=cache.get(key);
                    auto end=chrono::high_resolution_clock::now();    //Timer stops
                    auto duration=chrono::duration_cast<chrono::microseconds>(end-start);
                    if(cache.ram_flag){
                    cout<<GREEN<<"cache hit(RAM):key-"<<key<<RESET<<"\n";
                    }else{
                        cout<<YELLOW<<"Cache miss! Recovered from DB: key-"<<key<<RESET<<"\n";
                    }
                    if(r==-1)cout<<RED<<"!Key not found"<<RESET<<"\n";
                    else{cout<<"Result: "<<r<<"\n";}
                    cout<<HIGHLIGHT<<"Time taken: "<<duration.count()<<" µs"<<RESET<<"\n";
                    // client o/p
                    string resp=(r==-1)?"!Key not found\n":"Result: "+to_string(r)+"\n";
                    send(client_socket,resp.c_str(),resp.length(),0);
                }
                else if(command=="PUT"){
                    ss>>key_str>>val_str;
                    int key=stoi(key_str);
                    int val=stoi(val_str);
                    auto st=chrono::high_resolution_clock::now();
                    cache.put(key,val);
                    auto end=chrono::high_resolution_clock::now();
                    auto dur=chrono::duration_cast<chrono::microseconds>(end-st);
                    cout<<"Saved ("<<key<<","<<val<<") with TTL: 60secs\n";
                    cout<<"PUT latency: "<<dur.count()<<" µs\n";
                    // client o/p
                    string resp="OK\n";
                    send(client_socket,resp.c_str(),resp.length(),0);
                }
                else if(command=="STRESS"){
                    cout<<YELLOW<<"Spawning 5 threads to cache\n"<<RESET;
                    auto st=chrono::high_resolution_clock::now();
                    // simple task for each thread
                    auto work=[&cache,st](int thr_id){
                        cache.put(thr_id,thr_id*100,60);
                        auto now=chrono::high_resolution_clock::now();
                        auto dur=chrono::duration_cast<chrono::microseconds>(now-st);
                        string msg="Thread-"+to_string(thr_id)+" completed at "+to_string(dur.count())+" µs\n";
                        cout<<msg;
                    };
                    // Fire 5 threads at same time
                    vector<thread> th;
                    for(int i=1;i<=5;i++){th.push_back(thread(work,i));}
                    // wait for all threads to finish
                    for(auto& i:th){i.join();}
                    cout<<GREEN<<"Stress test complete! No crashes.\n"<<RESET;

                    string resp="Stress Test complete\r\n";
                    send(client_socket,resp.c_str(),resp.length(),0);
                }
                else if(command=="EXIT"){
                    cout<<"Shutting down server...\n";
                    string resp="Shutting down..\r\n";
                    send(client_socket,resp.c_str(),resp.length(),0);
                    closesocket(client_socket);
                    closesocket(server_fd);
                    mysql_close(conn);
                    WSACleanup();
                    return 0;
                }
                req="";
            }
        }
        closesocket(client_socket);
    }
    return 0;
}


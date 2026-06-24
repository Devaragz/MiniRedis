#ifndef LRU_CACHE_H
#define LRU_CACHE_H

#include "ThreadPool.h"
#include<bits/stdc++.h>
#include<iostream>
#include<string>
#include<unordered_map>
#include<chrono>
#include<iomanip>
#include<sstream>
#include<mysql.h>
#include<windows.h>
#include<mutex>
extern std::mutex print_mutex;
using namespace std;

// ANSI Escape codes for terminal UI
#define RESET "\033[0m"
#define GREEN "\033[32m"
#define YELLOW "\033[33m"
#define RED "\033[31m"
#define HIGHLIGHT "\033[47;30m"
#define GREY "\033[90m"
#define CYAN "\033[36m"


inline string getTime(){
    auto now=chrono::system_clock::now();
    auto ms=chrono::duration_cast<chrono::milliseconds>(now.time_since_epoch())%1000;
    auto t=chrono::system_clock::to_time_t(now);
    std::tm bt=*std::localtime(&t);
    std::ostringstream oss;
    oss<<CYAN<<std::put_time(&bt,"%H:%M:%S")<<'.'<<std::setfill('0')<<std::setw(3)<<ms.count()<<RESET;
    return oss.str();
}

// single entry in cache
class Node {
public:
    int key,val;
    chrono::steady_clock::time_point ex;  //Expiration Timestamp
    Node* prev;
    Node* next;
    Node(int k,int v,int ttl=60);
};

// LRU eviction and MYSQL fallback
class LRUcache {
private:
    ThreadPool* pool;
    // Lock mechanism
    recursive_mutex mtx;  //protects RAM
    mutex dbmx;           //protects MySQL connection
    int cap;
    unordered_map<int,Node*> cachemap;  //Maps key->loc
    Node* head;     //Dummy head->points most recent
    Node* tail;     //Dummy tail->points least recent
    MYSQL *conn; 
    void addNode(Node* newNode);
    void delNode(Node* delNode);

public:
    bool ram_flag;  //cache hit/miss->(T/F)
    LRUcache(int c,MYSQL *dbconn,ThreadPool* tp);
    ~LRUcache();    //Destructor
    int get(int key);
    void put(int key,int val,int ttl=60);
    void syncDB(int key,int val);
    int fetchDB(int key);
};

MYSQL* setupDatabase();
#endif


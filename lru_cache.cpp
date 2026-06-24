#include "lru_cache.h"
#include "ThreadPool.h"
#include<chrono>

extern std::mutex consolemtx;

#include<windows.h>
// Node Methods
Node::Node(int k,int v,int ttl){
    key=k;
    val=v;
    ex=chrono::steady_clock::now()+chrono::seconds(ttl);
    prev=nullptr;
    next=nullptr;
}

// LRUcache Methods
LRUcache::LRUcache(int c,MYSQL *dbconn,ThreadPool* tp){
    cap=c;
    conn=dbconn;
    pool=tp;
    // Initialize dummy nodes
    head=new Node(-1,-1);
    tail=new Node(-1,-1);
    head->next=tail;
    tail->prev=head;
}

// Destructor->prevents memory leaks
LRUcache::~LRUcache(){
    Node* curr=head;
    while(curr!=nullptr){
        Node* nextNode=curr->next;
        delete curr;
        curr=nextNode;
    }
    cout<<YELLOW<<"Memory freed. Server shutdown."<<RESET<<"\n";
}

// Inserts a new node
void LRUcache::addNode(Node* newNode){
    Node* temp=head->next;
    newNode->next=temp;
    newNode->prev=head;
    head->next=newNode;
    temp->prev=newNode;
}
// Removes an existing node
void LRUcache::delNode(Node* delNode){
    Node* delprev=delNode->prev;
    Node* delnext=delNode->next;
    delprev->next=delnext;
    delnext->prev=delprev;
}

// Retrieves data
int LRUcache::get(int key){
    lock_guard<recursive_mutex> lock(mtx);   //locks fn for this thread
    if(cachemap.find(key)!=cachemap.end()){
        Node* resNode=cachemap[key];
        // TTL exp check
        if(chrono::steady_clock::now()>resNode->ex){
            cachemap.erase(key);
            delNode(resNode);
            delete resNode;   //Free RAM
            ram_flag=false;
            return fetchDB(key);
        }
        int res=resNode->val;
        // Refresh position to MRU
        delNode(resNode);
        addNode(resNode);
        ram_flag=true;
        return res;
    }
    ram_flag=false;
    return fetchDB(key);
}

// Adds new data
void LRUcache::put(int key,int val,int ttl){
    lock_guard<recursive_mutex> lock(mtx);   //locks fn for this thread
    
    // if already exists, delete old version
    if(cachemap.find(key)!=cachemap.end()){
        Node* extNode=cachemap[key];
        cachemap.erase(key);
        delNode(extNode);
        delete extNode;
    }
    // if cache full, delete LRU
    if(cachemap.size()==cap){
        Node* lru=tail->prev;
        cachemap.erase(lru->key);
        delNode(lru);
        delete lru;   //Free RAM
    }
    addNode(new Node(key,val,ttl));
    cachemap[key]=head->next;

    // Async disk persistence
    pool->enqueue(TaskPriority::LOW,[this,key,val](){
        this->syncDB(key,val);
    });
}

void LRUcache::syncDB(int key,int val){
    lock_guard<mutex>lock(dbmx);
    string query="INSERT INTO cache_data (cache_key,cache_value) VALUES("+to_string(key)+","+to_string(val)+") ON DUPLICATE KEY UPDATE cache_value = "+ to_string(val);
    mysql_query(conn,query.c_str());

    string x=getTime()+string(GREEN) +"->[CORE"+to_string(GetCurrentProcessorNumber())+"] Priority:1(LOW)-Asynchronous DB sync | Key:"+to_string(key)+RESET+"\n";
    std::lock_guard<std::mutex>console_lock(consolemtx);
    cout<<x;
}

// Handles cache miss
int LRUcache::fetchDB(int key){
    lock_guard<mutex>db_lock(dbmx);
    string query="SELECT cache_value FROM cache_data WHERE cache_key = "+to_string(key);
    mysql_query(conn,query.c_str());
    MYSQL_RES *res=mysql_store_result(conn);
    MYSQL_ROW row=mysql_fetch_row(res);
    if(row){
        int dbval=stoi(row[0]);
        mysql_free_result(res);
        // manual RAM restore
        if(cachemap.size()==cap){
            Node* lru=tail->prev;
            cachemap.erase(lru->key);
            delNode(lru);
            delete lru;
        }
        Node* newNode=new Node(key,dbval);
        addNode(newNode);
        cachemap[key]=head->next;
        return dbval;
    }
    mysql_free_result(res);
    return -1;
}

// DB setup
MYSQL* setupDatabase(){
    MYSQL *conn=mysql_init(NULL);
    const char* dbPass=getenv("DB_PASS");
    if(dbPass==nullptr){
        cout<<"Error:DB_PASS environment variable not set!";
        exit(1);
    }
    if(!mysql_real_connect(conn,"localhost","root",dbPass,"cachedb",3306,NULL,0)){
        cout<<"Connection Failed"<<"\n";
        exit(1);
    }
    cout<<"Connection Successful\n";
    return conn;
}


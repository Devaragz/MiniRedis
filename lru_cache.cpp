#include<bits/stdc++.h>
#include<mysql.h>
using namespace std;

//single piece of cache data
class Node {
public:
    int key,val;
    Node* prev;
    Node* next;

    Node(int k,int v){
        key=k;
        val=v;
        prev=nullptr;
        next=nullptr;
    }
};

class LRUcache {
private:
    int cap;        //max capacity
    unordered_map<int,Node*> cachemap;     //Maps key->loc
    //create dummy tail & head nodes
    Node* head=new Node(-1,-1);  //Points most recent
    Node* tail=new Node(-1,-1);   //points least recent
    MYSQL *conn;    //DB connection pointer
    //Inserts a new node
    void addNode(Node* newNode){
        Node* temp=head->next;
        newNode->next=temp;
        newNode->prev=head;
        head->next=newNode;
        temp->prev=newNode;
    }
    //Remove an existing node
    void delNode(Node* delNode){
        Node* delprev=delNode->prev;
        Node* delnext=delNode->next;
        delprev->next=delnext;
        delnext->prev=delprev;
    }

public:
    LRUcache(int c,MYSQL *dbconn){
        cap=c;
        conn=dbconn;
        head->next=tail;
        tail->prev=head;
    }
    //Retrieves data
    int get(int key){
        if(cachemap.find(key)!=cachemap.end()){
            Node* resNode=cachemap[key];
            int res=resNode->val;
            cachemap.erase(key);
            delNode(resNode);
            addNode(resNode);
            cachemap[key]=head->next;
            cout<<"cache hit (RAM):key-"<<key<<endl;
            return res;
        }
        //cache miss:check mysql
        return fetchDB(key);
    }
    //Adds new data
    void put(int key,int val){
        //if already exists, delete old version
        if(cachemap.find(key)!=cachemap.end()){
            Node* extNode=cachemap[key];
            cachemap.erase(key);
            delNode(extNode);
        }
        //if cache full, delete LRU
        if(cachemap.size()==cap){
            Node* lru=tail->prev;
            cachemap.erase(lru->key);
            delNode(lru);
            delete lru;
        }
        addNode(new Node(key,val));
        cachemap[key]=head->next;
        //Sync to Mysql
        syncDB(key,val);
    }

    void syncDB(int key,int val){
        string query="INSERT INTO cache_data (cache_key,cache_value) VALUES("+to_string(key)+","+to_string(val)+") ON DUPLICATE KEY UPDATE cache_value = "+ to_string(val);
        mysql_query(conn,query.c_str());
    }

    int fetchDB(int key){
        string query="SELECT cache_value FROM cache_data WHERE cache_key = "+to_string(key);
        mysql_query(conn,query.c_str());
        MYSQL_RES *res=mysql_store_result(conn);
        MYSQL_ROW row=mysql_fetch_row(res);
        if(row){
            int dbval=stoi(row[0]);
            mysql_free_result(res);
            cout<<"Cache miss! Recovered from DB: key "<<key<<endl;
            //put back in RAM
            this->put(key,dbval);
            return dbval;
        }
        mysql_free_result(res);
        return -1;
    }
    ~LRUcache(){
        Node* curr=head;
        while(curr!=nullptr){
            Node* nextNode=curr->next;
            delete curr;
            curr=nextNode;
        }
    }
};
MYSQL* setupDatabase(){
    MYSQL *conn=mysql_init(NULL);
    const char* dbPass=getenv("DB_PASS");
    if(dbPass==nullptr){
        cout<<"Error:DB_PASS environment variable not set!";
        exit(1);
    }

    if(!mysql_real_connect(conn,"localhost","root",dbPass,"cachedb",3306,NULL,0)){
        cout<<"Connection Failed"<<endl;
        exit(1);
    }
    cout<<"Connection Successful\n";
    return conn;
}

int main(){
    cout<<"Startig MiniRedis Server..."<<endl;
    MYSQL *conn=setupDatabase();
    int capacity;
    cout<<"Enter cache capacity: ";
    cin>>capacity;
    LRUcache cache(capacity,conn);
    int choice,key,val;
    while(true){
        cout<<"\n   --MiniRedis Menu--\n";
        cout<<"1. PUT data\n2. GET data\n3. EXIT\nChoice: ";
        cin>>choice;
        if(choice==1){
            cout<<"Enter key & value: ";
            cin>>key>>val;
            cache.put(key,val);
            cout<<"Saved ("<<key<<","<<val<<")\n";
        }
        else if(choice==2){
            cout<<"Enter Key to fetch: ";
            cin>>key;

            auto start=chrono::high_resolution_clock::now();    //Timer starts
            int r=cache.get(key);
            auto end=chrono::high_resolution_clock::now();      //Timer stops
            auto duration=chrono::duration_cast<chrono::microseconds>(end-start);

            if(r==-1)cout<<"!Key not found\n";
            else{cout<<"Result: "<<r<<"\n";}
            cout<<"Time taken: "<<duration.count()<<" micro-secs\n";
        }
        else if(choice==3){
            cout<<"Shutting down server...\n";
            break;
        }
        else{cout<<"Invalid choice\n";}
    }

    mysql_close(conn);
    return 0;
}


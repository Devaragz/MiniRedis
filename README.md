# MiniRedis
A distributed, multithreaded C++ in-memory key-value store with MySQL data persistence and a custom TCP network client.

## --System Architecture & Working--
MiniRedis operates as a decoupled client-server architecture.
1. **The Server** listens on TCP port `8080`, handling incoming socket connection.
2. **The Client** provides an interactive CLI to send commands over the network.
3. **The Engine** processes requests using a custom DLL + Hash-Map LRU cache for $0(1)$ RAM lookups(~6µs).
4. **The Fallback:** On cache misses, data is recovered instantly form MySQL(~860µs).
5. **The Persistence:** New data is saved to MySQL asynchronously via detached background threads to prevent blocking RAM operations.

## --Core Features (V3 Final Form)--
* **Distributed TCP sockets:** Fully functional client-server network using `Winsock2`.
* **Multithreading & Concurrency:** Handles concurrent requests safely using `std::recursive_mutex` and `std::thread`.
* **Asynchronous DISK I/O:** MySQL DB syncing happens in the background, isolating disk bottlenecks from the main thread.
* **Lazy TTL eviction:** Auto-expires stale keys based on customizable Time-To-Live timestamps.
* **Custom CLI Client:** Replaces raw Telnet with a polished, interactive C++ command-line interface.
* **Memory Safety:** Custom destructor guarantee complete memory deallocation and clean socket closure on seerver shutdown.

## •Prerequisites
* C++ compiler (MinGW/GCC)
* MySQL Server installed and running
* Windows OS (for `ws2_32` socket library)

## •Database Setup
Run the included 'schema.sql' file in yout MySQL environment, or execute this in your MySQL command line:
```sql
CREATE DATABASE IF NOT EXISTS cachedb;
USE cachedb;
CREATE TABLE IF NOT EXISTS cache_data (cache_key INT PRIMARY KEY, cache_value INT);
```
## •Compilation & Execuation
1.Compile the Server:
Compile the modular files together using GCC. Run this in your terminal(Note:Adjust the -I & -L paths based on your specific MySQL/MariaDB installation):
```bash
g++ main.cpp lru_cache.cpp -o miniredis.exe -I"C:\msys64\mingw64\include\mariadb" -L"C:\msys64\mingw64\lib" -lmariadb -lws2_32
```
2.Compile the client:
```bash
g++ client.cpp -o client -lws2_32
```

# --How to Run--
Set your MySQL root password as an environment variable before running the executable.

###1. Start the Server
```powershell
$env:DB_PASS="your_password"
.\miniredis.exe
```
###2. Start the Client
•open a new, seperate terminal window and run:
```powershell
.\client.exe
```

# MiniRedis
A high-performance, non-blocking C++ in-memory key-value caching engine with asynchronous MySQL persistence and a custom TCP network client.

## --System Architecture & Working--
MiniRedis operates on a decoupled client-server architecture designed for low-latency memory access and robust disk persistence.
1. **The Server** listens on TCP port `8080`, handling incoming socket connection via `Winsock2`.
2. **The Client** A strict, production-accurate CLI mirroring standard `redis-cli` syntax and protocols.
3. **The Engine** processes requests using a custom DLL + Hash-Map LRU cache for **0(1) RAM lookups(~4µs).
4. **The Fallback:** On cache misses, data is instantly recovered from MySQL.
5. **The Persistence (ThreadPool):** New data is saved to memory instantly, while disk syncing is offloaded to a custom asynchronous ThreadPool to prevent blocking the main network thred.

## --Core Features (V Enterprise Form)--
* **Asynchronous ThreadPool:** Custom worker thread implementation handling expensive Disk I/O without blocking concurrent RAM operations.
* **Priority Task Queueing:** Critical cahe misses bypass the standard queue to ensure maximum retrievel speed. 
* **OS-Level Telemetry:** Real-Time logging of CPU core execution mapping, µs-level latency tracking, and thread-state monitoring.
* **O(1) LRU Eviction:** Strict memory capacity limits with auto-eviction of the LRU keys.
* **Memory Safety:** Custom destructor guarantee complete memory deallocation, thread joining, and clean socket closure on seerver shutdown.

## Telemetry & Execution Proof

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
g++ main.cpp lru_cache.cpp ThreadPool.cpp -o miniredis.exe -I"C:\msys64\mingw64\include\mariadb" -L"C:\msys64\mingw64\lib" -lmariadb -lws2_32
```
2.Compile the client:
```bash
g++ client.cpp -o client.exe -lws2_32
```

# --How to Run--
Set your MySQL root password as an environment variable before running the executable.
1. Start the Server
```powershell
$env:DB_PASS="your_password"
.\miniredis.exe
```
2. Start the Client
•open a new, seperate terminal window and run:
```powershell
.\client.exe
```

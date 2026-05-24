# MiniRedis
A lightweight C++ LRU cache with MySQL data persistence.

## Features & V2 Updates
* **Modular Architecture:** cleanly seperated into '.h', '.cpp' & 'main.cpp'.
* **Memory Safety:** Custom destructor guarantee zero memory leaks on shutdown.
* **Fast I/O & UI:** ANSI-colored terminal interface & optimized buffer flushes ('\n') for accurate micro-secs benchmarking.
* **DB Indexing:** Enforced B-Tree indexing (Primary Keys) in MySQL for instant data recovery.
* **In-memory LRU caching:** For fast 0(1) data retrieval.
* **MySQL backend:** For data persistence (cache misses are automatically fetched from DB).

## Prerequisites
* C++ compiler (MinGW/GCC)
* MySQL Server installed and running

## Database Setup
Run the included 'schema.sql' file in yout MySQL environment, or execute this in your MySQL command line:
```sql
CREATE DATABASE IF NOT EXISTS cachedb;
USE cachedb;
CREATE TABLE IF NOT EXISTS cache_data (cache_key INT PRIMARY KEY, cache_value INT);
```
## Compilation
Compile the modular files together using GCC. Run this in your terminal(Note:Adjust the -I & -L paths based on your specific MySQL/MariaDB installation):
```bash
g++ main.cpp lru_cache.cpp -o miniredis.exe -I"C:/msys64/mingw64/include/mariadb" -L"C:/msys64/mingw64/lib" -lmariadb
```

# ---How to Run---
Set your MySQL root password as an environment variable before running the executable.
## Windows (Powershell):
```
$env:DB_PASS="your_password"
.\lru_cache.exe
```
## Windows (Command Prompt):
```
set DB_PASS="your_password"
lru_cache.exe

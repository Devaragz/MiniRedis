# MiniRedis
A lightweight C++ LRU cache with MySQL data persistence.

## Features
* In-memory LRU caching for fast data retrieval.
* MySQL backend for data persistence (ache misses are fetched from DB).

## Prerequisites
* C++ compiler (MinGW/GCC)
* MySQL Server installed and running

## Database Setup
Run this in your MySQL command line:
'''sql
CREATE DATABASE cachedb;
USE cachedb;
CREATE TABLE cache_data (cache_key INT PRIMARY KEY, cache_value INT);

# ---How to Run---
Set your MySQL root password as an environment variable before runnint executable.
## Windows (Powershell):
'''
$env:DB_PASS="your_password"
.\lru_cache.exe

## Windows (Command Prompt):
'''
set DB_PASS="your_password"
lru_cache.exe

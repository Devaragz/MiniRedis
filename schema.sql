CREATE DATABASE IF NOT EXISTS cachedb;
USE cachedb;

CREATE TABLE IF NOT EXISTS cache_data (cache_key INT PRIMARY KEY,cache_value INT);
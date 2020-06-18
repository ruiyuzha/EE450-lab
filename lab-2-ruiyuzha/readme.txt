Full name: Ruiyu Zhao
USCID: 4057818955
Email: ruiyuzha@usc.edu

Brief program summary:
This lab aims to design a simplified DHCP-like protocol that sends messages over the reliable TCP. A DHCP server and a client are created to communicate over TCP socket.

How to compile the program?
- g++ -o server server.cpp
- g++ -o client client.cpp

How to run the program?
Terminal1: ./server
Terminal2: ./client localhost

Reference: 
the main part of client.cpp and server.cpp was mainly based on codes in Beej’s socket programming tutorial (Beej’s guide to network programming) on Page 27-31.
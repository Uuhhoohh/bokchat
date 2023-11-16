#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>
#include <algorithm>
#include <iostream>
#include <cstring>
#include <cstdio>
#include <thread>
#include <vector>
#include <mutex>
#include <Windows.h>

using namespace std;

SOCKET init_socket();
void broadcast(vector<SOCKET> *clients, string msg);
void handle_client(SOCKET client, vector<SOCKET> *clients, vector<string> *nicknames);
int accept_clients(SOCKET server, vector<SOCKET> *clients, vector<string> *nicknames);
void list_members(SOCKET client, vector<string> *nicknames);
string decrypt(string msg);
string encrypt(string msg);
void read_pipe(FILE *fp);
void write_pipe(FILE *fp, string msg);
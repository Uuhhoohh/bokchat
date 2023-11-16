#include "server.hh"


mutex clients_mutex;
mutex nicknames_mutex;


int main() 
{
    vector<SOCKET> clients;
    vector<string> nicknames;

    WSADATA wsaData;
    int result = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (result != 0) {
        cerr << "WSAStartup failed: " << result << endl;
        return 1;
    }

    SOCKET serverSocket = init_socket();

    if (listen(serverSocket, 5) == SOCKET_ERROR) {
        cerr << "Listen failed: " << WSAGetLastError() << endl;
        closesocket(serverSocket);
        WSACleanup();
        return 1;
    }

    cout << "Server is listening..." << endl;

    accept_clients(serverSocket, &clients, &nicknames);

    closesocket(serverSocket);
    WSACleanup();

    return 0;
}

SOCKET init_socket()
{
    SOCKET serverSocket = socket(AF_INET, SOCK_STREAM, 0);
        if (serverSocket == INVALID_SOCKET) {
            cerr << "Socket creation failed: " << WSAGetLastError() << endl;
            WSACleanup();
            return 1;
        }

        sockaddr_in serverAddr;
        serverAddr.sin_family = AF_INET;
        serverAddr.sin_port = htons(16262); // Example port
        serverAddr.sin_addr.s_addr = INADDR_ANY;

        if (bind(serverSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
            cerr << "Bind failed: " << WSAGetLastError() << endl;
            closesocket(serverSocket);
            WSACleanup();
            return 1;
        }
        return serverSocket;
}

int accept_clients(SOCKET server, vector<SOCKET> *clients, vector<string> *nicknames)
{
    while (1) {
        SOCKET clientSocket = accept(server, NULL, NULL);
        if (clientSocket == INVALID_SOCKET) {
            cerr << "Accept failed: " << WSAGetLastError() << endl;
            closesocket(server);
            WSACleanup();
            return 1;
        }

        {
            lock_guard<mutex> lock(clients_mutex);
            clients->push_back(clientSocket);
        }

        thread handler_thread(handle_client, clientSocket, clients, nicknames);
        handler_thread.detach();
    }
}

void handle_client(SOCKET client, vector<SOCKET> *clients, vector<string> *nicknames)
{
    struct sockaddr_in clientAddr;
    socklen_t clientAddrLen = sizeof(clientAddr);

    if (getpeername(client, (struct sockaddr*)&clientAddr, &clientAddrLen) == -1) {
        perror("getpeername");
    }

    char *clientIP = inet_ntoa(clientAddr.sin_addr);
    int clientPort = ntohs(clientAddr.sin_port);
    printf("Client connected. IP: %s, Client Port: %d\n", clientIP, clientPort);

    send(client, "__NICK__", 8, 0);
    char buffer[1024] = {0};
    recv(client, buffer, sizeof(buffer) - 1, 0);
    string nickname(buffer);
    {
        lock_guard<mutex> lock(nicknames_mutex);
        nicknames->push_back(nickname);
    }
    broadcast(clients, nickname + " has joined the chat");
    list_members(client, nicknames);

    while (1) {
        char buffer[1024] = {0};
        int bytesReceived = recv(client, buffer, sizeof(buffer) - 1, 0);
        if (bytesReceived > 0) {
            // std::cout << "Received message: " << buffer << "\n";
            string msg(buffer);
            if (msg == "__EXIT__") {
                {
                    lock_guard<mutex> lock(clients_mutex);
                    auto found = find(clients->begin(), clients->end(), client);
                    int index = distance(clients->begin(), found);
                    closesocket(client);
                    clients->erase(clients->begin() + index);
                }

                {
                    lock_guard<mutex> lock(nicknames_mutex);
                    auto found = find(nicknames->begin(), nicknames->end(), nickname);
                    int index = distance(nicknames->begin(), found);
                    nicknames->erase(nicknames->begin() + index);
                }
                broadcast(clients, nickname + " has left the chat");
                printf("Client disconnected. IP: %s, Client Port: %d\n", clientIP, clientPort);
                break;
            }
            broadcast(clients, nickname + ": " + buffer);
        }
    }
}

void broadcast(vector<SOCKET> *clients, string msg) 
{
    lock_guard<mutex> lock(clients_mutex);
    for (size_t i = 0; i < clients->size(); i++){
        SOCKET client = clients->at(i);
        send(client, msg.c_str(), msg.size(), 0);
    }
}


void list_members(SOCKET client, vector<string> *nicknames)
{
    string users;
    {
        lock_guard<mutex> lock(nicknames_mutex);
        if (nicknames->size() == 1){
            users = "You are the only user in the server currently... how lonely.";
            send(client, users.c_str(), users.size(), 0);
            return;
        }
        for (size_t i = 0; i < nicknames->size() - 1; i++) {
            users = nicknames->at(i) + ", " + users;
        }
        users = users + nicknames->back();
    }
    users = "Current users are: " + users;
    send(client, users.c_str(), users.size(), 0);
}

string decrypt(string msg)
{

}

void read_pipe(FILE *fp)
{
    fp = _popen("python cryptor.py", "r");
    if (fp == NULL){
        cerr << "Failed to run Python Script in read mode" << endl;
    }
    char buffer[4096] = {0};
    fgets(buffer, sizeof(buffer) - 1, fp);
    _pclose(fp);
    cout << "Reading done" << endl;
    cout << "Received from Python: " << string(buffer) << endl;
}

void write_pipe(FILE *fp, string msg)
{
    fp = _popen("python cryptor.py", "w");
    if (fp == NULL){
        cerr << "Failed to run Python Script in write mode" << endl;
    }
    fputs(msg.c_str(), fp);
    fflush(fp);
    cout << "Writing done" << endl;
    _pclose(fp);
}
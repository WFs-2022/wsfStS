/***********************************************/
/* File Name: server_wsfStS.cpp                */
/* Creator: CVE-2020-0796 (bilibili)           */
/* Date: 2024/05/21                            */
/* Comment: Server program for wsfStS          */
/* P.S. It's for the MULTI PLAYER version      */
/* LICENSE: it's under GPLv3 License           */
/***********************************************/

#include <winsock2.h>
#include <ws2tcpip.h>
#include <iostream>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <string>
#include <ctime>

#define DEFAULT_PORT "5755"
#define DEFAULT_BUFLEN 512

bool dueling; // to check if the duel pair is empty
std::string port;  // to control the port to use 
std::string player[2]; // name of each player
std::string shared[2]; // shared data the pair of duelists 
	bool coin; // flase -> 1first; true -> 2first;;; TODO: why not Stone Scissor Cloth?!
// then for mutex
std::mutex clients_mutex; // mutex for player1
std::mutex clients_mutex2; // mutex for player2
std::mutex clients_mutexM; // mutex for main thread
std::condition_variable cv; //condition variable for player1
std::condition_variable cv2; //condition variable for player2
std::condition_variable cvM; //condition variable for main thread
std::unique_lock<std::mutex> lock(clients_mutex); // lock for cv for player1
std::unique_lock<std::mutex> lock2(clients_mutex2); // lock for cv for player1
std::unique_lock<std::mutex> lockM(clients_mutexM); // lock for cv for main thread
	bool myLock; // temp lock for 
	SOCKET C1Socket, C2Socket; // general ones for p1 & p2, need to improve in the future as TOO MANY SAME SOCKET VARIABLES HERE!

void sendString(std::string playerName, SOCKET ClientSocket, std::string sendString){
    // Send a string to the client
    int iResult = send(ClientSocket, sendString.c_str(), sendString.length(), 0);
    if (iResult == SOCKET_ERROR) {
        std::cerr << "[ERROR ] send failed: to " << playerName << ", " << WSAGetLastError() << std::endl;
        closesocket(ClientSocket);
        return;
    }
    std::cout << "[ INFO ] Sent: to " << playerName << ", " << sendString << std::endl;
	return;
} std::string recvString(std::string playerName, SOCKET ClientSocket){
    // Receive a string from the client, return the string
    char recvbuf[DEFAULT_BUFLEN];
    int recvbuflen = DEFAULT_BUFLEN;
    int iResult = recv(ClientSocket, recvbuf, recvbuflen, 0);
    if (iResult > 0) {
        std::string receivedString(recvbuf, iResult);
        std::cout << "[ INFO ] Received: from " << playerName << ", " << receivedString << std::endl;
		return receivedString;
    } else if (iResult == 0)
        std::cout << "[ INFO ] Connection closing..." << std::endl;
    else
        std::cerr << "[ERROR ] recv failed: from " << playerName << ", " << WSAGetLastError() << std::endl;
    return "ERROR";
}
void HandleClient(SOCKET ClientSocket) {
	// function to handle each first client connection
	C1Socket = ClientSocket;
    player[0] = recvString("undefined1", ClientSocket);
    sendString(player[0], ClientSocket, "Wait: Waiting for opponent. ");
    cv.wait(lock);
    // both connected
    sendString(player[0], ClientSocket, player[1].c_str());
    std::cout << "[ INFO ] player1 prepared.\n";
    
    cv.wait(lock, [] {return myLock;}); // lock for coin 
    myLock = false; // reset lock
	if(coin) sendString(player[0], ClientSocket, "Info: Second");
	else     sendString(player[0], ClientSocket, "Info: First");
    while (1) {
		std::string recvStr = recvString(player[0], ClientSocket);
		if (player[1] == "") {
			sendString(player[0], ClientSocket, "Info: Opponent left"); // CLIENT:TODO
			break;
		}
		if (recvStr == "ERROR") break;
		else if (recvStr == "End Turn")
			sendString(player[1], C2Socket, "Info: End turn");
		else {
			// Card played then
			sendString(player[1], C2Socket, "Info: Card");
			sendString(player[1], C2Socket, recvStr);                             // card name
			sendString(player[1], C2Socket, recvString(player[0], ClientSocket)); // and objNum
		}
//    	TODO
	}
	
    // Cleanup
    player[0] = "";
    if (player[0] == "" && player[1] == "") dueling = false;
    closesocket(ClientSocket);
	return;
} void HandleClient2(SOCKET Client2Socket) {
	// function to handle each second client connection
	C2Socket = Client2Socket;
    player[1] = recvString("undefined2", Client2Socket);
    sendString(player[1], Client2Socket, "Info: Duel");
    sendString(player[1], Client2Socket, player[0].c_str());
    
    // both connected
    dueling = true;
    cv.notify_all();
    cvM.notify_all();
    std::cout << "[ INFO ] player2 prepared.\n";
    
    srand(int(clock()*1000));
	coin = (rand()%2 == 0);
	if(coin) sendString(player[1], Client2Socket, "Info: First");
	else     sendString(player[1], Client2Socket, "Info: Second");
	cv.notify_all();
	myLock = true; // so that thread1 can always be unblocked
    while (1) {
		std::string recvStr = recvString(player[1], Client2Socket);
		if (player[0] == "") {
			sendString(player[1], Client2Socket, "Info: Opponent left"); // CLIENT:TODO
			break;
		}
		if (recvStr == "ERROR") break;
		else if (recvStr == "End Turn")
			sendString(player[0], C1Socket, "Info: End turn");
		else {
			// Card played then
			sendString(player[0], C1Socket, "Info: Card");
			sendString(player[0], C1Socket, recvStr);                             // card name
			sendString(player[0], C1Socket, recvString(player[1], Client2Socket)); // and objNum
		}
//    	TODO
	}
	
    // Cleanup
    player[1] = "";
    if (player[0] == "" && player[1] == "") dueling = false;
    closesocket(Client2Socket);
	return;
}

int main() {
	// input the port to use
	std::cout << "[ INFO ] Input the port to use (default is 5755): ";
	std::cin >> port;
	
    WSADATA wsaData;
    int iResult;

    // Initialize Winsock
    iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (iResult != 0) {
        std::cerr << "[ERROR ] WSAStartup failed: " << iResult << std::endl;
        return 1;
    }

    struct addrinfo *result = NULL, hints;
    ZeroMemory(&hints, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    hints.ai_flags = AI_PASSIVE;

    // Resolve the server address and port
    iResult = getaddrinfo(NULL, port.c_str(), &hints, &result);
    if (iResult != 0) {
        std::cerr << "[ERROR ] getaddrinfo with set port failed: " << iResult << std::endl;
        std::clog << "[ INFO ] using default port..."<< std::endl;
		iResult = getaddrinfo(NULL, DEFAULT_PORT, &hints, &result);
		if (iResult != 0) {
        	std::cerr << "[ERROR ] getaddrinfo with default port failed: " << iResult << std::endl;
	        WSACleanup();
	        return 1;
		}
        std::clog << "[ INFO ] using default port successfully."<< std::endl;
    }

    // Create a SOCKET for the server to listen for client connections
    SOCKET ListenSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
    if (ListenSocket == INVALID_SOCKET) {
        std::cerr << "[ERROR ] socket failed: " << WSAGetLastError() << std::endl;
        freeaddrinfo(result);
        WSACleanup();
        return 1;
    }

    // Setup the TCP listening socket
    iResult = bind(ListenSocket, result->ai_addr, (int)result->ai_addrlen);
    if (iResult == SOCKET_ERROR) {
        std::cerr << "[ERROR ] bind failed: " << WSAGetLastError() << std::endl;
        freeaddrinfo(result);
        closesocket(ListenSocket);
        WSACleanup();
        return 1;
    }

    freeaddrinfo(result);

    iResult = listen(ListenSocket, SOMAXCONN);
    if (iResult == SOCKET_ERROR) {
        std::cerr << "[ERROR ] listen failed: " << WSAGetLastError() << std::endl;
        closesocket(ListenSocket);
        WSACleanup();
        return 1;
    }

    std::cout << "[ INFO ] Waiting for client connections..." << std::endl;

    while (true) {
        // Accept a client socket
        SOCKET ClientSocket = accept(ListenSocket, NULL, NULL);
        if (ClientSocket == INVALID_SOCKET) {
            std::cerr << "[ERROR ] accept failed: " << WSAGetLastError() << std::endl;
            closesocket(ListenSocket);
            WSACleanup();
            return 1;
        }
        
        // Handle the client connection in a new thread if the duel pair is empty
		if (dueling){
			// THIS STRING MESSAGE IS FOR THE CLIENT PROGRAM TO KNOW SERVER's FULL
			// DON'T CHANGE ANY CHARACTER OF IT
			sendString("undefined", ClientSocket, "Error: Server is full. ");
			closesocket(ClientSocket);
			continue;
		}

		// Connected
		sendString("undefined", ClientSocket, "Success: Connected");
        std::cout << "[ INFO ] Client connected." << std::endl;
        
        std::thread (HandleClient, ClientSocket).detach();
        
        // Accept the second client socket
        ClientSocket = accept(ListenSocket, NULL, NULL);
        if (ClientSocket == INVALID_SOCKET) {
            std::cerr << "[ERROR ] accept failed: " << WSAGetLastError() << std::endl;
            closesocket(ListenSocket);
            WSACleanup();
            return 1;
        }
        
		sendString("undefined", ClientSocket, "Success: Connected");

        std::cout << "[ INFO ] Client2 connected." << std::endl;

        // Handle the client connection in a new thread
        std::thread (HandleClient2, ClientSocket).detach();
    	cvM.wait(lockM);
    }

    // Cleanup
    closesocket(ListenSocket);
    WSACleanup();

    return 0;
}


#include <iostream>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstring>

using namespace std;

int main() {
    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);

    if (sockfd < 0) {
        cerr << "Failed to create socket!" << endl;
        return 1;
    }

    sockaddr_in dnsServer{};

    dnsServer.sin_family = AF_INET;
    dnsServer.sin_port = htons(53);
    dnsServer.sin_addr.s_addr = inet_addr("8.8.8.8");

    const char* message = "Hello";

    int bytesSent = sendto(
    sockfd,
    message,
    strlen(message),
    0,
    (sockaddr*)&dnsServer,
    sizeof(dnsServer)
    );

    cout << "Bytes sent: " << bytesSent << endl;

    cout << "UDP socket created successfully!" << endl;

    close(sockfd);
    return 0;
}
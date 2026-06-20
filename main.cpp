#include <iostream>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstring>
#include <cstdint>

using namespace std;

struct DNSHeader {
    uint16_t id;
    uint16_t flags;
    uint16_t qdcount;
    uint16_t ancount;
    uint16_t nscount;
    uint16_t arcount;
};

int main() {
    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);

    if (sockfd < 0) {
        cerr << "Failed to create socket!" << endl;
        return 1;
    }

    cout << "DNSHeader size: " << sizeof(DNSHeader) << " bytes" << endl;
    sockaddr_in dnsServer{};

    dnsServer.sin_family = AF_INET;
    dnsServer.sin_port = htons(53);
    dnsServer.sin_addr.s_addr = inet_addr("8.8.8.8");

    DNSHeader header{};
    header.id = htons(0x1234);
    header.flags = htons(0x0100); 
    header.qdcount = htons(1);
    header.ancount = htons(0);
    header.nscount = htons(0);
    header.arcount = htons(0);

    cout << "DNS header initialized" << endl;

    int bytesSent = sendto(
    sockfd,
    &header,
    sizeof(header),
    0,
    (sockaddr*)&dnsServer,
    sizeof(dnsServer)
    );

    cout << "Bytes sent: " << bytesSent << endl;

    cout << "DNS header sent successfully!" << endl;

    close(sockfd);
    return 0;
}
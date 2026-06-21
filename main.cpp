#include <iostream>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstring>
#include <cstdint>
#include <vector>

using namespace std;

struct DNSHeader {
    uint16_t id;
    uint16_t flags;
    uint16_t qdcount;
    uint16_t ancount;
    uint16_t nscount;
    uint16_t arcount;
};

struct DNSQuestion {
    uint16_t qtype;
    uint16_t qclass;
};

vector<uint8_t> encodeDomain(string domain) {
    vector<uint8_t> encoded;
    string currentLabel = "";

    for(char ch : domain) {

        if(ch == '.') {
            encoded.push_back(currentLabel.size());

            for(char c : currentLabel) {
                encoded.push_back(c);
            }

            currentLabel.clear();
        }
        else {
            currentLabel += ch;
        }
    }

    encoded.push_back(currentLabel.size());

    for(char c : currentLabel) {
        encoded.push_back(c);
    }

    encoded.push_back(0);

    return encoded;
}

int main() {
    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);

    if (sockfd < 0) {
        cerr << "Failed to create socket!" << endl;
        return 1;
    }

    cout << "DNSHeader size: " << sizeof(DNSHeader) << " bytes" << endl;
    cout << "DNSQuestion size: " << sizeof(DNSQuestion) << " bytes" << endl;
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

    DNSQuestion question{};
    question.qtype = htons(1);
    question.qclass = htons(1);

    cout << "DNS header initialized" << endl;

    vector<uint8_t> domain = encodeDomain("google.com");
    cout << "Encoded bytes: " << domain.size() << endl;
    for(uint8_t b : domain) {
    cout << (int)b << " ";
    }
    cout << endl;

    vector<uint8_t> packet;
    packet.insert(packet.end(), (uint8_t*)&header, (uint8_t*)&header + sizeof(header));
    cout << "Packet size after header: " << packet.size() << endl;
    packet.insert(packet.end(), domain.begin(), domain.end());
    cout << "Packet size after domain: " << packet.size() << endl;
    packet.insert(packet.end(), (uint8_t*)&question, (uint8_t*)&question + sizeof(question));
    cout << "Final packet size: " << packet.size() << endl;

    int bytesSent = sendto(sockfd,packet.data(),packet.size(),0,(sockaddr*)&dnsServer,sizeof(dnsServer));

    cout << "Bytes sent: " << bytesSent << endl;

    cout << "DNS header sent successfully!" << endl;

    close(sockfd);
    return 0;
}
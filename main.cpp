#include <iostream>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstring>
#include <cstdint>
#include <vector>

using namespace std;

#pragma pack(push, 1)
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

#pragma pack(pop)

vector<uint8_t> encodeDomain(const string& domain) {
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

    vector<uint8_t> domain = encodeDomain("google.com");
    cout << "Encoded bytes: " << domain.size() << endl;
    for(uint8_t b : domain) {
    cout << (int)b << " ";
    }
    cout << endl;
    cout << endl;

    vector<uint8_t> packet;
    packet.insert(packet.end(), (uint8_t*)&header, (uint8_t*)&header + sizeof(header));
    packet.insert(packet.end(), domain.begin(), domain.end());
    packet.insert(packet.end(), (uint8_t*)&question, (uint8_t*)&question + sizeof(question));
    cout << "Packet size: " << packet.size() << " bytes" << endl;
    cout << endl;

    uint8_t responseBuffer[512];
    sockaddr_in senderAddr{};
    socklen_t senderLen = sizeof(senderAddr);
    
    int bytesSent = sendto(sockfd,packet.data(),packet.size(),0,(sockaddr*)&dnsServer,sizeof(dnsServer));

    cout << "Bytes sent: " << bytesSent << endl;

    int bytesReceived = recvfrom(sockfd,responseBuffer,sizeof(responseBuffer),0,(sockaddr*)&senderAddr,&senderLen);

    if (bytesReceived < 0) {
    cerr << "Failed to receive response!" << endl;
    close(sockfd);
    return 1;
    }

    cout << "Bytes received: " << bytesReceived << endl;
    cout << endl;
    cout << "Response bytes: " << endl;

    for(int i = 0; i < bytesReceived; i++) {
    cout << (int)responseBuffer[i] << " ";
    }
    cout << endl;
    cout << endl;

    DNSHeader responseHeader{};

    memcpy(&responseHeader, responseBuffer, sizeof(DNSHeader));
    
    uint16_t responseId = ntohs(responseHeader.id);
    uint16_t responseFlags = ntohs(responseHeader.flags);
    uint16_t questionCount = ntohs(responseHeader.qdcount);
    uint16_t answerCount = ntohs(responseHeader.ancount);
    uint16_t authorityCount = ntohs(responseHeader.nscount);
    uint16_t additionalCount = ntohs(responseHeader.arcount);
    uint16_t rcode = responseFlags & 0x000F;

    cout << "Response ID: " << responseId << endl;
    cout << "Response Flags: 0x" << hex << responseFlags << dec << endl;
    cout << "Question Count: " << questionCount << endl;
    cout << "Answer Count: " << answerCount << endl;
    cout << "Authority Count: " << authorityCount << endl;
    cout << "Additional Count: " << additionalCount << endl;
    cout << "Response Code: " << rcode << endl;

    cout << endl;

    if(responseId == 0x1234) cout << "Transaction ID matched." << endl;
    else cout << "Transaction ID mismatch." << endl;

    cout << endl;

    int offset = 28;

    offset += 2;

    uint16_t type;
    memcpy(&type, responseBuffer + offset, 2);
    type = ntohs(type);
    cout << "Record Type: " << type << endl;
    offset += 2;

    uint16_t recordClass;
    memcpy(&recordClass, responseBuffer + offset, 2);
    recordClass = ntohs(recordClass);
    cout << "Record Class: " << recordClass << endl;
    offset += 2;

    uint32_t ttl;
    memcpy(&ttl, responseBuffer + offset, 4);
    ttl = ntohl(ttl);
    cout << "TTL: " << ttl << " seconds" << endl;
    offset += 4;

    uint16_t rdlength;
    memcpy(&rdlength, responseBuffer + offset, 2);
    rdlength = ntohs(rdlength);
    cout << "Data Length: " << rdlength << endl;
    offset += 2;

    cout << "Resolved IP: ";
    cout << (int)responseBuffer[offset] << ".";
    cout << (int)responseBuffer[offset + 1] << ".";
    cout << (int)responseBuffer[offset + 2] << ".";
    cout << (int)responseBuffer[offset + 3] << endl;

    cout << endl;

    cout << "DNS query completed successfully!" << endl;

    close(sockfd);
    return 0;
}
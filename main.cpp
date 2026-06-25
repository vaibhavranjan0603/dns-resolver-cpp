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

int skipName(const uint8_t* buffer, int offset) {

    while(true) {

        uint8_t len = buffer[offset];

        if((len & 0xC0) == 0xC0) {
            return offset + 2;
        }

        if(len == 0) {
            return offset + 1;
        }

        offset += len + 1;
    }
}

int main(int argc, char* argv[]) {

    if(argc != 2){
    cout << "Usage: ./main <domain>" << endl;
    return 1;
    }
    string domainName = argv[1];
    cout << "Domain: " << domainName << endl << endl;

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

    vector<uint8_t> domain = encodeDomain(domainName);
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
    if(rcode == 0) cout << "Status: No Error" << endl;
    else if(rcode == 1) cout << "Status: Format Error" << endl;
    else if(rcode == 2) cout << "Status: Server Failure" << endl;
    else if(rcode == 3) cout << "Status: Domain Not Found" << endl;
    else cout << "Status: Unknown Error" << endl;

    if(rcode != 0) {
        cout << "DNS query failed." << endl;
        close(sockfd);
        return 1;
    }

    if(answerCount == 0) {
        cout << "No answers received." << endl;
        close(sockfd);
        return 1;
    }

    cout << endl;

    if(responseId == 0x1234) cout << "Transaction ID matched." << endl;
    else cout << "Transaction ID mismatch." << endl;

    cout << endl;

    int offset = sizeof(DNSHeader) + domain.size() + sizeof(DNSQuestion);

    int ipCount = 1;
    bool printed = false;

    for(int i = 0; i < answerCount; i++) {
    offset = skipName(responseBuffer, offset);

    uint16_t type;
    memcpy(&type, responseBuffer + offset, 2);
    type = ntohs(type);
    offset += 2;

    uint16_t recordClass;
    memcpy(&recordClass, responseBuffer + offset, 2);
    recordClass = ntohs(recordClass);
    offset += 2;

    uint32_t ttl;
    memcpy(&ttl, responseBuffer + offset, 4);
    ttl = ntohl(ttl);
    offset += 4;

    uint16_t rdlength;
    memcpy(&rdlength, responseBuffer + offset, 2);
    rdlength = ntohs(rdlength);
    offset += 2;

    if(type == 5) {
    cout << "CNAME record found." << endl;
    }

    if(type == 1 && !printed) {
    cout << "Record Type: A" << endl;

    if(recordClass == 1)
        cout << "Record Class: IN" << endl;
    else
        cout << "Record Class: " << recordClass << endl;

    cout << "TTL: " << ttl << " seconds" << endl;
    cout << "Data Length: " << rdlength << endl;

    cout << endl;
    cout << "IP Addresses:" << endl;

    printed = true;
    }

    if(type == 1 && rdlength == 4) {
    cout << ipCount++ << ". ";
    cout << (int)responseBuffer[offset] << ".";
    cout << (int)responseBuffer[offset + 1] << ".";
    cout << (int)responseBuffer[offset + 2] << ".";
    cout << (int)responseBuffer[offset + 3] << endl;
    }

    offset += rdlength;        
    }
    cout << endl;

    cout << "DNS query completed successfully!" << endl;

    close(sockfd);
    return 0;
}
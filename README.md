# DNS Resolver (C++)

A mini DNS resolver built completely from scratch in C++ without using any built-in DNS lookup libraries.

The program sends raw DNS queries to Google’s public DNS server (8.8.8.8), receives the binary response over UDP, and manually parses the DNS packet to extract IP addresses.

⸻

## Features

* Constructs DNS packets manually
* Uses UDP sockets to communicate with DNS servers
* Encodes domain names into DNS wire format
* Parses DNS response headers
* Parses answer records
* Supports multiple A records
* Handles CNAME records
* Handles DNS compression pointers (0xC0)
* Supports command-line input
* Handles invalid domains
* Uses socket timeout to avoid hanging forever

⸻

## Technologies Used

* C++
* UDP sockets
* DNS protocol (RFC 1035)
* Binary packet parsing

⸻

## Build

clang++ -std=c++17 -o main main.cpp

⸻

## Usage

./main google.com
./main brave.com
./main www.youtube.com

⸻

## Example Output

Domain: google.com
Response Code: 0
Status: No Error
Record Type: A
Record Class: IN
TTL: 160 seconds
IP Addresses:
1. 142.250.67.46
DNS query completed successfully!

⸻

## Concepts Learned

* UDP vs TCP
* DNS protocol
* DNS packet structure
* DNS header fields
* Question section
* Answer section
* Network byte order
* htons(), ntohs(), ntohl()
* DNS compression pointers
* CNAME records
* Multiple A records
* Binary packet parsing

⸻

## Project Structure

dns-resolver-cpp/
│
├── main.cpp
├── README.md

⸻

## Future Improvements

* AAAA (IPv6) record support
* MX record support
* Recursive resolution
* Local DNS caching
* Configurable DNS servers

⸻

## Author

Vaibhav Ranjan
24/CS/477
B.Tech CSE (2024–2028)
Delhi Technological University (DTU)
Computer Science and Engineering
>>>>>>> 4633f13 (Day 9: Final polish and README)

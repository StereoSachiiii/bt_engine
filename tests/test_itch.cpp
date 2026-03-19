// create_test_itch.cpp
#include <fstream>
#include <iostream>
#include <cstdint>
#ifdef _WIN32
#include <winsock2.h>   // For htons on Windows
#pragma comment(lib, "ws2_32.lib")
#else
#include <arpa/inet.h>  // For htons on Linux/macOS
#endif

void write_message(std::ofstream& file, const uint8_t* data, uint16_t length) {
    uint16_t len_be = htons(length);  // Convert to big-endian
    file.write(reinterpret_cast<const char*>(&len_be), 2);
    file.write(reinterpret_cast<const char*>(data), length);
}

int main() {
    std::ofstream file("test.itch", std::ios::binary);
    
    // Message 1: System Event (Market Open)
    uint8_t msg1[12] = {
        'S',           // Message type
        0x00, 0x01,    // Stock locate
        0x00, 0x01,    // Tracking number
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  // Timestamp (6 bytes)
        'O'            // Event code: 'O' = Start of Messages
    };
    write_message(file, msg1, 12);
    
    // Message 2: Add Order (Buy AAPL, 100 shares @ $150.50)
    uint8_t msg2[36] = {
        'A',                          // Message type
        0x00, 0x01,                   // Stock locate
        0x00, 0x02,                   // Tracking number
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  // Timestamp (6 bytes)
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x30, 0x39,  // Order ref (12345)
        'B',                          // Buy/Sell: 'B' = Buy
        0x00, 0x00, 0x00, 0x64,       // Shares: 100
        'A', 'A', 'P', 'L', ' ', ' ', ' ', ' ',  // Stock: "AAPL    "
        0x00, 0x00, 0x3A, 0x62        // Price: $150.50 (15050 in 1/100 cents)
    };
    write_message(file, msg2, 36);
    
    // Message 3: Delete Order
    uint8_t msg3[19] = {
        'D',           // Message type
        0x00, 0x01,    // Stock locate
        0x00, 0x03,    // Tracking number
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  // Timestamp
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x30, 0x39   // Order ref
    };
    write_message(file, msg3, 19);
    
    file.close();
    std::cout << "Created test.itch with 3 messages\n";
    return 0;
}
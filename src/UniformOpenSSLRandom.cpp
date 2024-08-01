#include "UniformOpenSSLRandom.h"


#include <openssl/rand.h>

#include <cmath>
#include <cstdint>
#include <stdexcept>

uint64_t get_random_uint64(uint64_t min, uint64_t max) {
    // Calculate the size of the range
    uint64_t range = max - min + 1;

    // Determine the number of random bytes needed
    uint64_t num_bytes = (uint64_t) ceil(log2(range) / 8);

    // Calculate the maximum value that can be used in the range
    uint64_t max_value = pow(2, num_bytes * 8);

    // Generate random bytes until a value in the desired range is found
    uint64_t rand_value;
    do {
        unsigned char rand_bytes[num_bytes];
        int rc = RAND_bytes(rand_bytes, num_bytes);
        if (rc != 1) {
            throw std::runtime_error("Failed to generate random bits");
        }

        // Convert the random bytes to an unsigned integer
        rand_value = 0;
        for (uint64_t i = 0; i < num_bytes; i++) {
            rand_value = (rand_value << 8) + rand_bytes[i];
        }
    } while (rand_value >= max_value - max_value % range);

    // Map the random value to the desired range
    return rand_value % range + min;
}

uint8_t get_random_geometric() {
    // Initialize the count and the random bits
    uint8_t count = 0;
    uint8_t random_byte;

    // Generate random bits until the first set bit is obtained
    while (true) {
        // Generate one random byte using OpenSSL's cryptographic RNG
        if (RAND_bytes(&random_byte, sizeof(random_byte)) != 1) {
            // Throw an exception if the random number generator fails
            throw std::runtime_error("Failed to generate random bits");
        }

        // Loop through the least significant bits of the random byte
        for (int i = 0; i < 8; i++) {
            if (random_byte & (1 << i)) {
                // Return the number of bits generated before the first set bit is obtained
                return count + i;
            }
        }

        // Update the count by 8 bits (since we used all 8 bits of the random byte)
        count += 8;
    }
}

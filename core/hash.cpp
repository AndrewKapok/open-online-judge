#include "hash.h"
#include <array>
#include <cstring>
#include <iomanip>
#include <random>
#include <sstream>
#include <stdexcept>
#include <vector>

// ---------------------------------------------------------------------------
// SHA-256 constants and helper functions
// ---------------------------------------------------------------------------

/**
 * @brief Rotate bits to the right.
 */
constexpr std::uint32_t rotr(std::uint32_t x, unsigned n)
{
    return (x >> n) | (x << (32 - n));
}

/**
 * @brief Shift bits to the right (zero-fill).
 */
constexpr std::uint32_t shr(std::uint32_t x, unsigned n)
{
    return x >> n;
}

/**
 * @brief Choose function:  if bit in x is 1 → y, else → z.
 */
constexpr std::uint32_t ch(std::uint32_t x, std::uint32_t y, std::uint32_t z)
{
    return (x & y) ^ (~x & z);
}

/**
 * @brief Majority function:  majority of the three bits.
 */
constexpr std::uint32_t maj(std::uint32_t x, std::uint32_t y, std::uint32_t z)
{
    return (x & y) ^ (x & z) ^ (y & z);
}

/**
 * @brief Sigma0 — used in compression.
 */
constexpr std::uint32_t big_sigma0(std::uint32_t x)
{
    return rotr(x, 2) ^ rotr(x, 13) ^ rotr(x, 22);
}

/**
 * @brief Sigma1 — used in compression.
 */
constexpr std::uint32_t big_sigma1(std::uint32_t x)
{
    return rotr(x, 6) ^ rotr(x, 11) ^ rotr(x, 25);
}

/**
 * @brief sigma0 — used in message schedule.
 */
constexpr std::uint32_t small_sigma0(std::uint32_t x)
{
    return rotr(x, 7) ^ rotr(x, 18) ^ shr(x, 3);
}

/**
 * @brief sigma1 — used in message schedule.
 */
constexpr std::uint32_t small_sigma1(std::uint32_t x)
{
    return rotr(x, 17) ^ rotr(x, 19) ^ shr(x, 10);
}

// ---------------------------------------------------------------------------
// Initial hash values (first 32 bits of fractional parts of sqrt of primes)
// ---------------------------------------------------------------------------

static constexpr std::array<std::uint32_t, 8> H0 = {{
    0x6a09e667, 0xbb67ae85, 0x3c6ef372, 0xa54ff53a,
    0x510e527f, 0x9b05688c, 0x1f83d9ab, 0x5be0cd19
}};

// ---------------------------------------------------------------------------
// Round constants (first 32 bits of fractional parts of cube root of primes)
// ---------------------------------------------------------------------------

static constexpr std::array<std::uint32_t, 64> K = {{
    0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5,
    0x3956c25b, 0x59f111f1, 0x923f82a4, 0xab1c5ed5,
    0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3,
    0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174,
    0xe49b69c1, 0xefbe4786, 0x0fc19dc6, 0x240ca1cc,
    0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da,
    0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7,
    0xc6e00bf3, 0xd5a79147, 0x06ca6351, 0x14292967,
    0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13,
    0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85,
    0xa2bfe8a1, 0xa81a664b, 0xc24b8b70, 0xc76c51a3,
    0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070,
    0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5,
    0x391c0cb3, 0x4ed8aa4a, 0x5b9cca4f, 0x682e6ff3,
    0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208,
    0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2
}};

// ---------------------------------------------------------------------------
// SHA-256 core
// ---------------------------------------------------------------------------

std::string sha256(const void* data, std::size_t len)
{
    // ---- Pad the message -------------------------------------------------
    // 1. Append a single '1' bit (0x80 byte)
    // 2. Append zeros until (message_length % 64 == 56)
    // 3. Append original bit length as big-endian 64-bit integer

    std::vector<std::uint8_t> buf;
    if (data && len > 0) {
        auto* bytes = static_cast<const std::uint8_t*>(data);
        buf.assign(bytes, bytes + len);
    }

    // Append 0x80
    buf.push_back(0x80);

    // Append zero bytes until (buf.size() % 64 == 56)
    while (buf.size() % 64 != 56) {
        buf.push_back(0x00);
    }

    // Append length in bits (big-endian)
    std::uint64_t bit_len = static_cast<std::uint64_t>(len) * 8;
    for (int i = 7; i >= 0; --i) {
        buf.push_back(static_cast<std::uint8_t>((bit_len >> (i * 8)) & 0xff));
    }

    // ---- Process each 512-bit block -------------------------------------
    std::array<std::uint32_t, 8> state = H0;

    for (std::size_t offset = 0; offset < buf.size(); offset += 64) {
        // Prepare message schedule W[0..63]
        std::array<std::uint32_t, 64> W{};

        for (unsigned t = 0; t < 16; ++t) {
            W[t] = (static_cast<std::uint32_t>(buf[offset + t * 4])     << 24)
                 | (static_cast<std::uint32_t>(buf[offset + t * 4 + 1]) << 16)
                 | (static_cast<std::uint32_t>(buf[offset + t * 4 + 2]) << 8)
                 | (static_cast<std::uint32_t>(buf[offset + t * 4 + 3]));
        }

        for (unsigned t = 16; t < 64; ++t) {
            W[t] = small_sigma1(W[t - 2]) + W[t - 7]
                 + small_sigma0(W[t - 15]) + W[t - 16];
        }

        // Initialize working variables
        auto a = state[0], b = state[1], c = state[2], d = state[3];
        auto e = state[4], f = state[5], g = state[6], h = state[7];

        // Compression main loop
        for (unsigned t = 0; t < 64; ++t) {
            auto t1 = h + big_sigma1(e) + ch(e, f, g) + K[t] + W[t];
            auto t2 = big_sigma0(a) + maj(a, b, c);

            h = g;
            g = f;
            f = e;
            e = d + t1;
            d = c;
            c = b;
            b = a;
            a = t1 + t2;
        }

        // Compute intermediate hash values
        state[0] += a;
        state[1] += b;
        state[2] += c;
        state[3] += d;
        state[4] += e;
        state[5] += f;
        state[6] += g;
        state[7] += h;
    }

    // ---- Output (big-endian hex) ----------------------------------------
    std::ostringstream oss;
    for (auto word : state) {
        oss << std::hex << std::setfill('0') << std::setw(8) << word;
    }
    return oss.str();
}

// ---------------------------------------------------------------------------
// Salt generation
// ---------------------------------------------------------------------------

std::string generate_salt(std::size_t byte_count)
{
    std::random_device rd;
    std::mt19937_64 gen(rd());
    std::uniform_int_distribution<unsigned> dist(0, 255);

    std::ostringstream oss;
    oss << std::hex << std::setfill('0');
    for (std::size_t i = 0; i < byte_count; ++i) {
        oss << std::setw(2) << dist(gen);
    }
    return oss.str();
}

// ---------------------------------------------------------------------------
// Salted password hashing
// ---------------------------------------------------------------------------

std::string hash_password(const std::string& password, const std::string& salt)
{
    // digest = SHA-256(salt + password)
    auto digest = sha256(salt + password);
    return salt + "$" + digest;
}

bool verify_password(const std::string& password, const std::string& salted_hash)
{
    // Find the delimiter separating salt from hash
    auto delim_pos = salted_hash.find('$');
    if (delim_pos == std::string::npos || delim_pos == 0) {
        return false;
    }

    auto salt = salted_hash.substr(0, delim_pos);
    auto expected = hash_password(password, salt);
    return salted_hash == expected;
}
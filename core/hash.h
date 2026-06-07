#pragma once
#include <cstdint>
#include <string>

/**
 * @file hash.h
 * @brief Password hashing utilities with SHA-256 and salt.
 *
 * Provides a self-contained implementation of SHA-256 along with
 * helper functions to generate random salts and produce / verify
 * salted password hashes in the format "salt$hash".
 */

/**
 * @brief Compute the SHA-256 digest of arbitrary data.
 *
 * Implementation follows FIPS PUB 180-4.  The input is padded,
 * processed in 512-bit blocks, and the resulting 256-bit hash
 * is returned as a 64-character lowercase hex string.
 *
 * @param data  Pointer to the input bytes
 * @param len   Number of bytes to hash
 * @return Hex-encoded SHA-256 digest (64 characters)
 */
std::string sha256(const void* data, std::size_t len);

/// @overload
inline std::string sha256(const std::string& input)
{
    return sha256(input.data(), input.size());
}

/**
 * @brief Generate a cryptographically-random salt encoded as a hex string.
 *
 * Uses std::random_device to obtain entropy.  Each byte of salt is
 * represented by two hex characters.
 *
 * @param byte_count  Number of random bytes to generate (default: 16)
 * @return Hex-encoded salt string (2 * byte_count characters)
 */
std::string generate_salt(std::size_t byte_count = 16);

/**
 * @brief Hash a password together with a salt using SHA-256.
 *
 * The returned string has the format:
 * @code
 *   salt_hex$sha256_hex
 * @endcode
 *
 * where the actual digest is computed as SHA-256(salt_hex + password).
 *
 * @param password  Plaintext password
 * @param salt      Hex-encoded salt string (produced by generate_salt)
 * @return Combined salted hash string
 */
std::string hash_password(const std::string& password, const std::string& salt);

/**
 * @brief Verify a plaintext password against a previously stored salted hash.
 *
 * Extracts the salt from the stored string and recomputes the hash
 * to check for a match.
 *
 * @param password    Plaintext password to verify
 * @param salted_hash Stored string previously returned by hash_password()
 * @return true if the password matches, false otherwise
 */
bool verify_password(const std::string& password, const std::string& salted_hash);
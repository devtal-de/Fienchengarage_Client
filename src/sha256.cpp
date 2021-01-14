    
#include <Arduino.h>
#include <bearssl/bearssl_hash.h>


String sha256(const String data) {
    uint8_t tmp[32];
    String hashStr((const char*)nullptr);
    hashStr.reserve(32 * 2 + 1);

    br_sha256_context ctx;
    br_sha256_init(&ctx);
    br_sha256_update(&ctx, (const uint8_t*) data.c_str(), data.length());
    br_sha256_out(&ctx, tmp);

    for(uint16_t i = 0; i < 32; i++) {
        char hex[3];
        snprintf(hex, sizeof(hex), "%02x", tmp[i]);
        hashStr += hex;
    }
    return hashStr;
}
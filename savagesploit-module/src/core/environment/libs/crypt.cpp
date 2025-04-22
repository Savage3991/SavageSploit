//
// Created by savage on 22.04.2025.
//

#include "../environment.h"
#include "libsodium/include/sodium.h"
#include <string.h>

int base64encode(lua_State* L) {
    luaL_checktype(L, 1, LUA_TSTRING);

    size_t input_len;
    const char* input = lua_tolstring(L, 1, &input_len);

    if (sodium_init() < 0) {
        luaL_error(L, "failed to load libsodium, report this issue.");
    }

    size_t encoded_len = sodium_base64_encoded_len(input_len, sodium_base64_VARIANT_ORIGINAL);
    char* encoded_data = (char*)malloc(encoded_len);

    if (!encoded_data) {
        luaL_error(L, "memory allocation failed");
    }

    sodium_bin2base64(
        encoded_data,
        encoded_len,
        (const unsigned char*)input,
        input_len,
        sodium_base64_VARIANT_ORIGINAL
    );

    lua_pushstring(L, encoded_data);
    free(encoded_data);

    return 1;
}

int base64decode(lua_State* L) {
    luaL_checktype(L, 1, LUA_TSTRING);

    size_t input_len;
    const char* input = lua_tolstring(L, 1, &input_len);

    if (sodium_init() < 0) {
         luaL_error(L, "failed to load libsodium, report this issue.");
    }

    size_t max_decoded_len = input_len;
    unsigned char* decoded_data = (unsigned char*)malloc(max_decoded_len);

    if (!decoded_data) {
         luaL_error(L, "memory allocation failed");
    }

    size_t actual_decoded_len;

    int result = sodium_base642bin(
        decoded_data,
        max_decoded_len,
        input,
        input_len,
        NULL,
        &actual_decoded_len,
        NULL,
        sodium_base64_VARIANT_ORIGINAL
    );

    if (result != 0) {
        free(decoded_data);
        luaL_error(L, "base64 decoding failed: %d", result);
    }

    lua_pushlstring(L, (const char*)decoded_data, actual_decoded_len);
    free(decoded_data);

    return 1;
}

int generatebytes(lua_State* L) {
    if (sodium_init() < 0) {
        luaL_error(L, "failed to load libsodium, report this issue.");
    }

    int size = luaL_checkinteger(L, 1);
    if (size <= 0) {
        luaL_error(L, "size must be a positive integer");
    }

    unsigned char* buffer = (unsigned char*)malloc(size);
    if (!buffer) {
        luaL_error(L, "memory allocation failed");
    }

    randombytes_buf(buffer, size);

    size_t encoded_len = sodium_base64_encoded_len(size, sodium_base64_VARIANT_ORIGINAL);
    char* encoded_data = (char*)malloc(encoded_len);

    if (!encoded_data) {
        free(buffer);
        luaL_error(L, "memory allocation failed");
    }

    sodium_bin2base64(
        encoded_data,
        encoded_len,
        buffer,
        size,
        sodium_base64_VARIANT_ORIGINAL
    );

    lua_pushstring(L, encoded_data);

    free(buffer);
    free(encoded_data);

    return 1;
}

int generatekey(lua_State* L) {
    if (sodium_init() < 0) {
        luaL_error(L, "failed to load libsodium, report this issue.");
    }

    unsigned char key[crypto_secretbox_KEYBYTES];
    randombytes_buf(key, sizeof key);

    size_t encoded_len = sodium_base64_encoded_len(sizeof(key), sodium_base64_VARIANT_ORIGINAL);
    char* encoded_key = (char*)malloc(encoded_len);

    if (!encoded_key) {
        luaL_error(L, "memory allocation failed");
    }

    sodium_bin2base64(
        encoded_key,
        encoded_len,
        key,
        sizeof key,
        sodium_base64_VARIANT_ORIGINAL
    );

    lua_pushstring(L, encoded_key);
    free(encoded_key);

    return 1;
}

int hash(lua_State* L) {
    if (sodium_init() < 0) {
        luaL_error(L, "failed to load libsodium, report this issue.");
    }

    size_t input_len;
    const char* input = luaL_checklstring(L, 1, &input_len);
    const char* algorithm = luaL_checkstring(L, 2);

    unsigned char hash[64];
    size_t hash_len = 0;

    if (strcmp(algorithm, "sha256") == 0) {
        hash_len = crypto_hash_sha256_BYTES;
        crypto_hash_sha256(hash, (const unsigned char*)input, input_len);
    }
    else if (strcmp(algorithm, "sha512") == 0) {
        hash_len = crypto_hash_sha512_BYTES;
        crypto_hash_sha512(hash, (const unsigned char*)input, input_len);
    }
    else if (strcmp(algorithm, "sha1") == 0) {
        hash_len = 20;
        crypto_generichash(hash, hash_len, (const unsigned char*)input, input_len, NULL, 0);
    }
    else if (strcmp(algorithm, "md5") == 0) {

        hash_len = 16;
        crypto_generichash(hash, hash_len, (const unsigned char*)input, input_len, NULL, 0);
    }
    else if (strcmp(algorithm, "sha384") == 0) {
        crypto_hash_sha512(hash, (const unsigned char*)input, input_len);
        hash_len = 48;
    }
    else if (strncmp(algorithm, "sha3-", 5) == 0) {
        const char* bits_str = algorithm + 5;
        int bits = atoi(bits_str);
        hash_len = bits / 8;

        if (bits == 224 || bits == 256 || bits == 512) {
            crypto_generichash(hash, hash_len, (const unsigned char*)input, input_len, NULL, 0);
        } else {
            luaL_error(L, "unsupported SHA-3 variant: %s", algorithm);
        }
    }
    else {
        luaL_error(L, "unsupported hash algorithm: %s", algorithm);
    }

    size_t encoded_len = sodium_base64_encoded_len(hash_len, sodium_base64_VARIANT_ORIGINAL);
    char* encoded_hash = (char*)malloc(encoded_len);

    if (!encoded_hash) {
        luaL_error(L, "memory allocation failed");
    }

    sodium_bin2base64(
        encoded_hash,
        encoded_len,
        hash,
        hash_len,
        sodium_base64_VARIANT_ORIGINAL
    );

    lua_pushstring(L, encoded_hash);
    free(encoded_hash);

    return 1;
}

void environment::load_crypt_lib(lua_State *L) {

    static const luaL_Reg crypt_lib[] = {
        {"base64encode", base64encode},
        {"base64decode", base64decode},
        {"generatebytes", generatebytes},
        {"generatekey", generatekey},
        {"hash", hash},
        {nullptr, nullptr}
    };

    for (const luaL_Reg* lib = crypt_lib; lib->name; lib++) {
        lua_pushcclosure(L, lib->func, nullptr, 0);
        lua_setglobal(L, lib->name);
    }

    lua_newtable(L);

    for (const luaL_Reg* lib = crypt_lib; lib->name; lib++) {
        lua_pushcclosure(L, lib->func, nullptr, 0);
        lua_setfield(L, -2, lib->name);
    }

    lua_pushcclosure(L, base64encode, nullptr, 0);
    lua_setfield(L, -2, "base64_encode");

    lua_pushcclosure(L, base64decode, nullptr, 0);
    lua_setfield(L, -2, "base64_decode");

    lua_newtable(L);

    lua_pushcclosure(L, base64encode, nullptr, 0);
    lua_setfield(L, -2, "encode");

    lua_pushcclosure(L, base64decode, nullptr, 0);
    lua_setfield(L, -2, "decode");

    lua_setfield(L, -2, "base64");

    lua_setglobal(L, "crypt");

    // Global base64 functions and table for compatibility
    lua_pushcclosure(L, base64encode, nullptr, 0);
    lua_setglobal(L, "base64_encode");

    lua_pushcclosure(L, base64decode, nullptr, 0);
    lua_setglobal(L, "base64_decode");

    lua_newtable(L);

    lua_pushcclosure(L, base64encode, nullptr, 0);
    lua_setfield(L, -2, "encode");

    lua_pushcclosure(L, base64decode, nullptr, 0);
    lua_setfield(L, -2, "decode");

    lua_setglobal(L, "base64");
}
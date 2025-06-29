#pragma once

#include <Geode/DefaultInclude.hpp>
#include <Geode/utils/general.hpp>
#include <openssl/evp.h>
#include <openssl/bio.h>
#include <openssl/decoder.h>

// This is all in the header for obfuscation

using namespace geode::prelude;

namespace pro::ssl {
    constexpr const char* BE_SERVER_PUBLIC_KEY = R"CRT(
-----BEGIN PUBLIC KEY-----
MIIBIjANBgkqhkiG9w0BAQEFAAOCAQ8AMIIBCgKCAQEAq+q/2UG3Vn1pOMHvpuvW
nUATPiJB8LTPSjGSCfBmFiOgCGrmi395kLN18XN9XpPGJ95JKRZw58vPHS8Bvm42
nVPmblUg5WYSTA6EKKxhFfNwgtt6HlDRH6oapcyy3T7Qs2qlavDMd2UCRy29GqHd
2jI4sB0zln/M+T+UiHmSlLoCFAxgcxy+NHOgD2yooNr9WHJ3Yw0pya+Fmr0fhO0I
vwol94TUIupFOY0pU0gpDlqOFdQSzKCzNa7oCFvTHK8WlgyhUIQ2KMZQ82bBZahq
oYijzKDwLSLvkR5Aj4dOYqKODsKPvT3nTrKTUDVTz98Hw1VG1nAv7gMVNItYWt7v
wQIDAQAB
-----END PUBLIC KEY-----
    )CRT";

    consteval size_t publicKeyMagic() {
        return std::char_traits<char>::length(BE_SERVER_PUBLIC_KEY);
    }

    template <class T>
    struct OpenSSLDeleter;

    template <>
    struct OpenSSLDeleter<EVP_PKEY> {
        static constexpr auto Delete = &EVP_PKEY_free;
    };
    template <>
    struct OpenSSLDeleter<OSSL_DECODER_CTX> {
        static constexpr auto Delete = &OSSL_DECODER_CTX_free;
    };
    template <>
    struct OpenSSLDeleter<BIO> {
        static constexpr auto Delete = &BIO_free;
    };
    template <>
    struct OpenSSLDeleter<EVP_MD_CTX> {
        static constexpr auto Delete = &EVP_MD_CTX_free;
    };
    template <>
    struct OpenSSLDeleter<EVP_ENCODE_CTX> {
        static constexpr auto Delete = &EVP_ENCODE_CTX_free;
    };

    // Can't use unique_ptr because you can't get a T** from that and OpenSSL 
    // reaaaallly likes its T**s
    template <class T, size_t Magic>
    class OpenSSLRAII final {
    private:
        std::array<char, Magic> m_magicBullshitPaddingThatJustMakesItAnnoyingToRE;
        T* m_ptr = nullptr;

    public:
        OpenSSLRAII() {}
        OpenSSLRAII(T* ptr) : m_ptr(ptr) {}
        OpenSSLRAII(OpenSSLRAII const&) = delete;
        OpenSSLRAII& operator=(OpenSSLRAII const&) = delete;
        OpenSSLRAII(OpenSSLRAII&& other) : m_ptr(other.m_ptr) {
            other.m_ptr = nullptr;
        }
        OpenSSLRAII& operator=(OpenSSLRAII&& other) {
            m_ptr = other.m_ptr;
            other.m_ptr = nullptr;
            return *this;
        }
        ~OpenSSLRAII() {
            if (m_ptr) {
                OpenSSLDeleter<T>::Delete(m_ptr);
            }
        }
        operator bool() {
            return bool(m_ptr);
        }
        operator bool() const {
            return bool(m_ptr);
        }
        operator T*() {
            return m_ptr;
        }
        operator T const*() const {
            return m_ptr;
        }
        operator T**() {
            return &m_ptr;
        }
    };

    constexpr size_t MAGIC_OK = 0xec;

    template <size_t Magic>
    OpenSSLRAII<EVP_PKEY, Magic + 2> getPublicKey() {
        auto key = OpenSSLRAII<EVP_PKEY, Magic + 2>();

        auto dctx = OpenSSLRAII<OSSL_DECODER_CTX, Magic + 11>(
            OSSL_DECODER_CTX_new_for_pkey(
                key, nullptr, nullptr, nullptr, 
                EVP_PKEY_PUBLIC_KEY, nullptr, nullptr
            )
        );
        if (!dctx) {
            return nullptr;
        }

        auto bio = OpenSSLRAII<BIO, Magic + 17>(BIO_new_mem_buf(BE_SERVER_PUBLIC_KEY, -1));
        if (!bio) {
            return nullptr;
        }
        if (!OSSL_DECODER_from_bio(dctx, bio)) {
            return nullptr;
        }

        return std::move(key);
    }

    template <size_t Magic>
    size_t verify(std::string const& message, ByteVector const& signature) {
        auto key = getPublicKey<Magic + 2>();
        if (!key) {
            return 0;
        }
        
        auto ctx = OpenSSLRAII<EVP_MD_CTX, Magic + 18>(EVP_MD_CTX_create());

        if (EVP_DigestVerifyInit(ctx, nullptr, EVP_sha256(), nullptr, key) <= 0) {
            return 0;
        }
        if (EVP_DigestVerifyUpdate(ctx, message.data(), message.size()) <= 0) {
            return 0;
        }
        if (EVP_DigestVerifyFinal(ctx, signature.data(), signature.size()) == 1) {
            return Magic + MAGIC_OK;
        }
        return 0;
    }

    template <size_t Magic>
    std::optional<ByteVector> decodeBase64(std::string const& str) {
        auto ctx = OpenSSLRAII<EVP_ENCODE_CTX, Magic + 9>(EVP_ENCODE_CTX_new());

        int resLength;
        ByteVector res;
        res.resize(3 * str.size() / 4);

        EVP_DecodeInit(ctx);
        if (EVP_DecodeUpdate(
            ctx,
            res.data(), &resLength,
            reinterpret_cast<const unsigned char*>(str.data()), static_cast<int>(str.size())
        ) != 0) {
            return std::nullopt;
        }
        res.resize(resLength);
        if (EVP_DecodeFinal(ctx, res.data(), &resLength) != 1) {
            return std::nullopt;
        }
        return res;
    }

    // template <size_t Magic>
    // std::optional<std::string> decode(std::string const& message, ByteVector const& iv) {
        
    // }
}

#include "OpenSSL.hpp"

// Without this WolfSSL tries to include winsock and that's not fun :(
#define WOLFSSL_USER_IO
#include <wolfssl/options.h>
#include <wolfssl/wolfcrypt/settings.h>
#include <wolfssl/wolfcrypt/signature.h>
#include <wolfssl/wolfcrypt/rsa.h>
#include <wolfssl/wolfcrypt/asn_public.h>

constexpr std::string_view BE_SERVER_PUBLIC_KEY = R"CRT(
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

bool pro::ssl::verify(std::string const& message, ByteVector const& messageSignature) {
    // static auto _ = wolfSSL_Init();

    ByteVector keyAsDer = ByteVector(2048);
    keyAsDer.resize(wc_PubKeyPemToDer(
        reinterpret_cast<const unsigned char*>(BE_SERVER_PUBLIC_KEY.data()),
        BE_SERVER_PUBLIC_KEY.size(),
        keyAsDer.data(),
        keyAsDer.size()
    ));

    RsaKey beServerKey;
    wc_InitRsaKey(&beServerKey, nullptr);
    uint32_t ix = 0;
    if (auto error = wc_RsaPublicKeyDecode(keyAsDer.data(), &ix, &beServerKey, keyAsDer.size())) {
        log::error("Unable to decode BetterEdit's RSA public key: code {}", error);
        return false;
    }

    ByteVector verifySignature = ByteVector(64);
    if (auto error = wc_SignatureVerify(
        WC_HASH_TYPE_SHA256,
        WC_SIGNATURE_TYPE_RSA_W_ENC,
        reinterpret_cast<const byte*>(message.data()),
        message.size(),
        messageSignature.data(),
        messageSignature.size(),
        &beServerKey, sizeof(beServerKey)
    )) {
        log::error("Unable to verify BetterEdit token signature: code {}", error);
        return false;
    }

    return true;
}

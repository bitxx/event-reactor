#ifndef __EVENT_CRYPTO_H__
#define __EVENT_CRYPTO_H__

#include <event_core.h>

namespace Evpp
{
    typedef i32 CertificateCipherCallback(char* buf, int size, int rwflag, void* handler);

    #define DIGEST_SHA1     1 << 0
    #define DIGEST_SHA224   1 << 1
    #define DIGEST_SHA256   1 << 2
    #define DIGEST_SHA384   1 << 3
    #define DIGEST_SHA512   1 << 4

    static const char* rsa_public_key =
        R"(-----BEGIN PUBLIC KEY-----
MIICIjANBgkqhkiG9w0BAQEFAAOCAg8AMIICCgKCAgEAtjUtBgRQnkB0yIwQgk68
4aSvD2GZuYiJyjo310Cg9mNIFM6nPKSTWSDo6EBR4kWNWggfbALnP6mFgtnN8Sd2
duoqNHk9rBTNp8HKJ41DLxIyyIEZjyJdN6FtmmOKN+V74zBIpYYHrMnaPGQ7iAdv
CGlR/h0mY9CS6RhFtm2gNwfccd2BF9vQfn1V7YcP6D8mLWLJ38mLgsEEXD1628n3
cZRbHzOUAJljX+gaT3XuDhQsoJNFGg/ZDpHAe5PJrkRFkEp9pBUuopjmUTkQNOTP
R37KrvBbB7IM0FQjc07VS1asU6qk1wjrNEAikEqZ0R0YPd0oSpW57QQtrBDsX9sJ
EnE4MaSWswhG/zfaSp9h8W+uuM8WwN95eh3icy4/uy6Y3OO5PDQFr1ibYb67x6nZ
0ceptHcqbaYn9F9yKB7sWonLZSksA01jlJuhHHm/Lfr0G9+U7J6noqnqder3gKB2
kYAaAR6iixbsS698DSnTm93auPY8EW7bu4HIoD3UVpDaPI67+YZhFZlqLLjrIH3J
9Hs+JZHCjJ/SBXo3mj2TtYxw/zFyOr7+7PHcs9NRonUyS9GTnCQZY3Q0uAvCiNeF
yYV5j0RhjCbiXGyXHiFGIfPxDRZyqv9Pg8uthcQE+RBzBRwkzAfcScAoXeSeADLy
rMvMFhNgZrAcQN3XvtyQMKECAwEAAQ==
-----END PUBLIC KEY-----)";
    static const char* rsa_private_key =
        R"(-----BEGIN PRIVATE KEY-----
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
yq4Tv2ggD6LpRdLyhLW+ezHSLvVHiUs=
-----END PRIVATE KEY-----)";

    NOFORCEINLINE bool GetFileDigest            (const std::string& file, std::string& buffer);
    NOFORCEINLINE bool GetContextDigest         (const std::string& source, std::string& buffer);

    NOFORCEINLINE bool GetCipherAes             (const std::string& source, std::string& buffer, const u32 enc = 1);
    NOFORCEINLINE bool GetCipherDes             (const std::string& source, std::string& buffer, const u32 enc = 1);
    NOFORCEINLINE bool GetCipherBase64          (const std::string& source, std::string& buffer, const u32 enc = 1, const u32 line = 1);

    NOFORCEINLINE bool GetCipherSha             (const std::string& source, std::string& buffer, std::string& buffer_string, const u32 method = DIGEST_SHA1);
    NOFORCEINLINE bool GetCipherSha1            (const std::string& source, std::string& buffer, std::string& buffer_string);
    NOFORCEINLINE bool GetCipherSha224          (const std::string& source, std::string& buffer, std::string& buffer_string);
    NOFORCEINLINE bool GetCipherSha256          (const std::string& source, std::string& buffer, std::string& buffer_string);
    NOFORCEINLINE bool GetCipherSha384          (const std::string& source, std::string& buffer, std::string& buffer_string);
    NOFORCEINLINE bool GetCipherSha512          (const std::string& source, std::string& buffer, std::string& buffer_string);
    NOFORCEINLINE bool GetCipherSha1Ex          (const std::string& file, std::string& buffer, std::string& buffer_string);



    // TODO: GetCipherRsa

    // RSA_PKCS1_PADDING       1
    // RSA_NO_PADDING          3
    // RSA_X931_PADDING        5
    // 在线生成 公钥私钥
    // http://web.chacuo.net/netrsakeypair/

    NOFORCEINLINE bool GetCipherRsa             (const std::string& source, std::string& buffer, const std::string& key, const u32 enc, const u32 method, const i32 padding = 1, CertificateCipherCallback callback = nullptr, void* handler = nullptr);
    NOFORCEINLINE bool GetCipherRsaSign         (const std::string& source, std::string& buffer, const std::string& key, const u32 method, CertificateCipherCallback callback = nullptr, void* handler = nullptr);
    NOFORCEINLINE bool GetCipherRsaVerifySign   (const std::string& source, std::string& buffer, const std::string& sign, const std::string& key, const u32 method, CertificateCipherCallback callback = nullptr, void* handler = nullptr);
    NOFORCEINLINE bool GetCipherEncryptData     (const std::string& source, std::string& buffer, CertificateCipherCallback callback = nullptr, void* handler = nullptr);
    NOFORCEINLINE bool GetCipherDecryptData     (const std::string& source, std::string& buffer, CertificateCipherCallback callback = nullptr, void* handler = nullptr);
}

#endif
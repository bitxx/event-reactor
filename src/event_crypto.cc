#include <event_crypto.h>

#include <openssl/md5.h>
#include <openssl/evp.h>
#include <openssl/aes.h>
#include <openssl/des.h>
#include <openssl/rsa.h>
#include <openssl/pem.h>
#include <openssl/sha.h>
#include <openssl/crypto.h>

#include <iostream>
#include <fstream>

namespace Evpp
{
    class CipherStackRecovery
    {
    public:
        explicit CipherStackRecovery(const std::function<void()>& callback) : function(callback)
        {

        }
        virtual ~CipherStackRecovery()
        {
            if (nullptr != function)
            {
                function();
            }
        }
    private:
        std::function<void()>           function;
    };

    unsigned char setrc4key[256] = { 0xFE,0x0C,0xEE,0xFF,0x06,0xBB,0xEF,0xD7,0xBE,0xA8,0xD7,0x01,0xBB,0xC2,0xE9,0xE0,0xF0,0xAF,0x08,0xA6,0x00,0xA0,0xB0,0xB8,0xC1,0xC2,0xA6,0xC4,0xB2,0xBA,0xC0,0xCE,0xAA,0x04,0xAD,0x0D,0xB6,0xB5,0xA2,0xA8,0xB5,0xA2,0xEF,0xF2,0xC4,0xC2,0xBA,0xBF,0xA1,0xE7,0xF6,0xB1,0xD2,0xD3,0xE3,0x05,0xAD,0x04,0xA6,0xE0,0xCD,0xAE,0xA8,0xB8,0xC4,0xB6,0xC5,0xC5,0xB2,0xC5,0x05,0xDD,0xA0,0x0C,0xAF,0xEE,0xE0,0xCF,0x0F,0xF7,0xE1,0x06,0xBE,0x0B,0xE2,0x00,0xE8,0xFC,0xE9,0xF8,0xBC,0x0B,0xAB,0xD5,0xBD,0xDC,0x0A,0xE9,0xEE,0xE0,0xDE,0xEE,0xEB,0xA5,0xE3,0xAD,0xEE,0xCB,0xC1,0xA4,0x01,0xF7,0x02,0xE2,0xFF,0xF2,0xC6,0xE1,0xFF,0x02,0xC1,0xB5,0x00,0xDD,0xEA,0xD3,0xDA,0x0B,0xDC,0xA0,0xE6,0x0C,0xF6,0xE5,0xC3,0x0D,0xF5,0x0D,0xF7,0xEB,0xFA,0xAA,0x03,0xD6,0xCE,0x07,0xAF,0xE3,0xD7,0xFF,0xA0,0xE7,0xBE,0xC6,0xAE,0xBE,0x0D,0xFE,0xE0,0xCE,0xD0,0xBE,0xB7,0xA7,0xEB,0xFF,0xF2,0xB5,0x01,0xB3,0xF2,0xEC,0xF8,0xDE,0xF5,0xEF,0xE2,0xEF,0xDF,0xB4,0xDC,0x0D,0xBF,0xE9,0xAA,0xC4,0x01,0xE3,0xD6,0xDB,0xF9,0x05,0x09,0xD4,0xFB,0xD6,0xB9,0xA3,0xC1,0xD1,0xB6,0xAD,0x04,0xAB,0x05,0xE9,0xD3,0x0C,0xAA,0x07,0xC0,0xBC,0xE8,0xD2,0xCB,0xCB,0x0B,0xD8,0xBE,0x04,0xAA,0xFA,0x0B,0xBA,0x0C,0xF8,0xDE,0xD4,0xF9,0xEE,0xF0,0xF4,0xC2,0xD8,0xE8,0xC4,0xA2,0xBA,0xA3,0xF5,0x01,0x02,0xD2,0xAC,0xCC,0xCD,0x08,0xAE,0xB7,0xF1,0xF8,0xB2,0xBC,0xF3,0xC5,0xEC };
    unsigned char set_sha_key[SHA512_DIGEST_LENGTH] = { 0xDE,0xD3,0xE3,0xC4,0xCE,0xCE,0xC7,0xDD,0xA6,0xF1,0x0C,0xE0,0x07,0x0D,0xF7,0xA0,0xF7,0xC0,0xB3,0xF8,0xAF,0xD0,0xBC,0xE9,0xF1,0xF0,0xFF,0xBF,0xBA,0xAE,0xE1,0xFA,0xC6,0xE6,0xCF,0xE0,0xAF,0x09,0xD5,0xB1,0x0E,0xDD,0x08,0xA8,0xE8,0xA7,0x08,0xFE,0xB6,0xCE,0x06,0xF1,0xAC,0x0C,0xE6,0xBC,0xF4,0xF4,0xAC,0xAA,0xB3,0xBC,0xA9,0xFC };

    NOFORCEINLINE bool GetFileDigest(const std::string& file, std::string& buffer)
    {
        if (file.empty())
        {
            return false;
        }

        MD5_CTX context = {};
        {
            if (LoadSystemFile(file, buffer) && MD5_Init(std::addressof(context)))
            {
                if (MD5_Update(std::addressof(context), reinterpret_cast<const unsigned char*>(file.data()), buffer.size()))
                {
                    unsigned char crypto[MD5_DIGEST_LENGTH] = {};
                    {
                        if (1 == MD5_Final(crypto, std::addressof(context)))
                        {
                            if (buffer.size())
                            {
                                buffer.clear();
                                buffer.shrink_to_fit();
                            }

                            if (BinaryToString(std::string(reinterpret_cast<char*>(crypto), std::size(crypto)), buffer))
                            {
                                EVENT_TRACE("GetFileDigest: %s", buffer.c_str());
                                return true;
                            }
                        }
                    }
                }
            }
        }

        return false;
    }

    NOFORCEINLINE bool GetContextDigest(const std::string& source, std::string& buffer)
    {
        if (source.empty())
        {
            return false;
        }

        MD5_CTX context = {};
        {
            if (buffer.size())
            {
                buffer.clear();
                buffer.shrink_to_fit();
            }

            if (MD5_Init(std::addressof(context)) && MD5_Update(std::addressof(context), reinterpret_cast<const unsigned char*>(source.data()), source.size()))
            {
                unsigned char crypto[MD5_DIGEST_LENGTH] = {};
                {
                    if (1 == MD5_Final(crypto, std::addressof(context)))
                    {
                        if (BinaryToString(std::string(reinterpret_cast<char*>(crypto), std::size(crypto)), buffer))
                        {
                            EVENT_TRACE("ContextDigest: %s", buffer.c_str());
                            return true;
                        }
                    }
                }
            }
        }

        return false;
    }

    NOFORCEINLINE bool GetCipherAes(const std::string& source, std::string& buffer, const u32 enc)
    {
        if (source.empty())
        {
            return false;
        }

        AES_KEY context = {};
        {
            if (0 == [] (const u32 status) { return status; }(enc ? AES_set_encrypt_key(reinterpret_cast<const unsigned char*>(setrc4key), std::size(setrc4key), std::addressof(context)) : AES_set_decrypt_key(reinterpret_cast<const unsigned char*>(setrc4key), std::size(setrc4key), std::addressof(context))))
            {
                unsigned char crypto[4096] = {}; unsigned char ivec[AES_BLOCK_SIZE] = {};
                {

                    //in	    输入数据。长度任意。
                    //out	    输出数据。能够容纳下输入数据，且长度必须是16字节的倍数。
                    //length	输入数据的实际长度。
                    //key	    使用AES_set_encrypt / decrypt_key生成的Key。
                    //ivec	    可读写的一块内存。长度必须是16字节。
                    //enc	    是否是加密操作。AES_ENCRYPT表示加密，AES_DECRYPT表示解密。

                    AES_cbc_encrypt(reinterpret_cast<const unsigned char*>(source.data()),
                                    crypto,
                                    source.size(),
                                    std::addressof(context),
                                    reinterpret_cast<unsigned char*>(ivec),
                                    enc);

                    {
                        std::string temp;

                        if (BinaryToString(std::string(reinterpret_cast<char*>(crypto),
                                                        // AES_BLOCK_SIZE = 16 
                                                        // 输出数据缓冲区的长度必须是16字节的倍数
                                                        // 如果直接使用 strlen 遇到中间又 '\00' 的情况就会前功尽弃
                                                        // 所以最终的长度算法就是 源数据长度 + AES_BLOCK_SIZE(16) - 源数据长度 % AES_BLOCK_SIZE(16)
                                                       source.size() + AES_BLOCK_SIZE - source.size() % AES_BLOCK_SIZE), temp))
                        {
                            buffer.swap(*const_cast<std::string*>(std::addressof(static_cast<const std::string&>(std::move(std::string(reinterpret_cast<char*>(crypto), source.size() + AES_BLOCK_SIZE - source.size() % AES_BLOCK_SIZE))))));
                            //EVENT_TRACE("GetCipherAes: %s", temp.c_str());
                            return true;
                        }
                    }
                }
            }
        }

        return false;
    }

    NOFORCEINLINE bool GetCipherDes(const std::string& source, std::string& buffer, const u32 enc)
    {
        if (source.empty())
        {
            return false;
        }

        static DES_cblock setkey = { 0x07,0x0C,0xDD,0x02,0xDE,0xA3,0x02,0xC8 }; DES_key_schedule schedule = {};
        {
            if (-2 != DES_set_key_checked(&setkey, std::addressof(schedule)))
            {
                unsigned char crypto[4096] = {}; DES_cblock cipher = { 0xFC,0xB8,0xEE,0xF4,0xE1,0xFD,0xF0,0xEA };
                {
                    DES_cbc_encrypt(reinterpret_cast<const unsigned char*>(source.data()),
                                    crypto,
                                    source.size(),
                                    std::addressof(schedule),
                                    std::addressof(cipher),
                                    enc);
                }

                return [&] (const u32 size)
                {
                    std::string temp;

                    if (BinaryToString(std::string(reinterpret_cast<char*>(crypto),
                                                // DES_KEY_SZ = 8
                                                // 输出数据缓冲区的长度必须是16字节的倍数
                                                // 如果直接使用 strlen 遇到中间又 '\00' 的情况就会前功尽弃
                                                // 所以最终的长度算法就是 源数据长度 + DES_KEY_SZ(8) - 源数据长度 % DES_KEY_SZ(8)
                                                   size), temp))
                    {
                        buffer.swap(*const_cast<std::string*>(std::addressof(static_cast<const std::string&>(std::move(std::string(reinterpret_cast<char*>(crypto), size))))));
                        //EVENT_TRACE("GetCipherDes: %s", temp.c_str());
                        return true;
                    }
                    return false;

                }(source.size() + DES_KEY_SZ - source.size() % DES_KEY_SZ);
            }
        }
        return false;
    }

    NOFORCEINLINE bool GetCipherBase64(const std::string& source, std::string& buffer, const u32 enc, const u32 line)
    {
        if (source.empty())
        {
            return false;
        }

        unsigned char crypto[4096] = {};
        {
            if (0 != [&] (const i32 size)
                {
                    std::string temp;
                    {
                        if (BinaryToString(std::string(reinterpret_cast<char*>(crypto), size % 2 ? size - 1 : size), temp))
                        {
                            buffer.swap(*const_cast<std::string*>(std::addressof(static_cast<const std::string&>(std::move(std::string(reinterpret_cast<char*>(crypto), size % 2 ? size - 1 : size))))));
                            {
                                return true;
                            }
                        }
                    }

                    return 0 != size;
                }(enc ? EVP_EncodeBlock(crypto, reinterpret_cast<const unsigned char*>(source.data()), static_cast<i32>(source.size())) : EVP_DecodeBlock(crypto, reinterpret_cast<const unsigned char*>(source.data()), static_cast<i32>(source.size()))))
            {
                return true;
            }
        }

        return false;
    }

    NOFORCEINLINE bool GetCipherSha(const std::string& source, std::string& buffer, std::string& buffer_string, const u32 method)
    {
        switch (method)
        {
            case DIGEST_SHA1:       return GetCipherSha1(source, buffer, buffer_string);
            case DIGEST_SHA224:     return GetCipherSha224(source, buffer, buffer_string);
            case DIGEST_SHA256:     return GetCipherSha256(source, buffer, buffer_string);
            case DIGEST_SHA384:     return GetCipherSha384(source, buffer, buffer_string);
            case DIGEST_SHA512:     return GetCipherSha512(source, buffer, buffer_string);
            default: break;
        }
        return false;
    }

    NOFORCEINLINE bool GetCipherSha1(const std::string& source, std::string& buffer, std::string& buffer_string)
    {
        SHA_CTX context = {};
        {
            unsigned char crypto[SHA_DIGEST_LENGTH] = {};
            {
                if (SHA1_Init(std::addressof(context)) && SHA1_Update(std::addressof(context), source.data(), source.size()) && SHA1_Final(crypto, std::addressof(context)))
                {
                    if (BinaryToString(std::string(reinterpret_cast<const char*>(crypto), std::size(crypto)), buffer_string))
                    {
                        buffer.swap(*const_cast<std::string*>(std::addressof(static_cast<const std::string&>(std::move(std::string(reinterpret_cast<char*>(crypto), std::size(crypto)))))));
                    }

                    return true;
                }
            }
        }
        return false;
    }

    NOFORCEINLINE bool GetCipherSha224(const std::string& source, std::string& buffer, std::string& buffer_string)
    {
        SHA256_CTX context = {};
        {
            unsigned char crypto[SHA224_DIGEST_LENGTH] = {};
            {
                if (SHA224_Init(std::addressof(context)) && SHA224_Update(std::addressof(context), source.data(), source.size()) && SHA224_Final(crypto, std::addressof(context)))
                {
                    if (BinaryToString(std::string(reinterpret_cast<const char*>(crypto), std::size(crypto)), buffer_string))
                    {
                        buffer.swap(*const_cast<std::string*>(std::addressof(static_cast<const std::string&>(std::move(std::string(reinterpret_cast<char*>(crypto), std::size(crypto)))))));
                    }

                    return true;
                }
            }
        }
        return false;
    }

    NOFORCEINLINE bool GetCipherSha256(const std::string& source, std::string& buffer, std::string& buffer_string)
    {
        SHA256_CTX context = {};
        {
            unsigned char crypto[SHA256_DIGEST_LENGTH] = {};
            {
                if (SHA256_Init(std::addressof(context)) && SHA256_Update(std::addressof(context), source.data(), source.size()) && SHA256_Final(crypto, std::addressof(context)))
                {
                    if (BinaryToString(std::string(reinterpret_cast<const char*>(crypto), std::size(crypto)), buffer_string))
                    {
                        buffer.swap(*const_cast<std::string*>(std::addressof(static_cast<const std::string&>(std::move(std::string(reinterpret_cast<char*>(crypto), std::size(crypto)))))));
                    }

                    return true;
                }
            }
        }
        return false;
    }

    NOFORCEINLINE bool GetCipherSha384(const std::string& source, std::string& buffer, std::string& buffer_string)
    {
        SHA512_CTX context = {};
        {
            unsigned char crypto[SHA384_DIGEST_LENGTH] = {};
            {
                if (SHA384_Init(std::addressof(context)) && SHA384_Update(std::addressof(context), source.data(), source.size()) && SHA384_Final(crypto, std::addressof(context)))
                {
                    if (BinaryToString(std::string(reinterpret_cast<const char*>(crypto), std::size(crypto)), buffer_string))
                    {
                        buffer.swap(*const_cast<std::string*>(std::addressof(static_cast<const std::string&>(std::move(std::string(reinterpret_cast<char*>(crypto), std::size(crypto)))))));
                    }

                    return true;
                }
            }
        }
        return false;
    }

    NOFORCEINLINE bool GetCipherSha512(const std::string& source, std::string& buffer, std::string& buffer_string)
    {
        SHA512_CTX context = {};
        {
            unsigned char crypto[SHA512_DIGEST_LENGTH] = {};
            {
                if (SHA512_Init(std::addressof(context)) && SHA512_Update(std::addressof(context), source.data(), source.size()) && SHA512_Final(crypto, std::addressof(context)))
                {
                    if (BinaryToString(std::string(reinterpret_cast<const char*>(crypto), std::size(crypto)), buffer_string))
                    {
                        buffer.swap(*const_cast<std::string*>(std::addressof(static_cast<const std::string&>(std::move(std::string(reinterpret_cast<char*>(crypto), std::size(crypto)))))));
                    }

                    return true;
                }
            }
        }
        return false;
    }

    bool GetCipherSha1Ex(const std::string& file, std::string& buffer, std::string& buffer_string)
    {
        SHA_CTX context = {};
        {
            unsigned char crypto[SHA_DIGEST_LENGTH] = {};  char file_buffer[1024 * 16];
            {
                if (SHA1_Init(std::addressof(context)))
                {
                    std::ifstream fs(file, std::ios::in | std::ios::binary);
                    {
                        for (; fs.good();)
                        {
                            fs.read(file_buffer, std::size(file_buffer));
                            {
                                if (SHA1_Update(std::addressof(context), file_buffer, fs.gcount()))
                                {
                                    continue;
                                }

                                return false;
                            }
                        }

                        if (SHA1_Final(crypto, std::addressof(context)))
                        {
                            if (BinaryToString(std::string(reinterpret_cast<const char*>(crypto), std::size(crypto)), buffer_string))
                            {
                                buffer.swap(*const_cast<std::string*>(std::addressof(static_cast<const std::string&>(std::move(std::string(reinterpret_cast<char*>(crypto), std::size(crypto)))))));
                            }
                        }
                    }
                    return true;
                }
            }
        }
        return false;
    }

    bool CreateCipherRsa(const std::string& key, RSA** context, const u32 overt, CertificateCipherCallback callback, void* handler)
    {
        return [&] (BIO* bio)
        {
            if (nullptr == bio)
            {
                return false;
            }

            if (nullptr != context)
            {
                // 公钥加密每次结果都会改变即是正确
                *context = overt ? PEM_read_bio_RSA_PUBKEY(bio, context, callback, handler) : PEM_read_bio_RSAPrivateKey(bio, context, callback, handler);
            }

            return 1 == BIO_free(bio) && nullptr != context;

        }(BIO_new_mem_buf(key.data(), key.size()));
    }

    NOFORCEINLINE bool GetCipherRsa(const std::string& source, std::string& buffer, const std::string& key, const u32 enc, const u32 method, const i32 padding, CertificateCipherCallback callback, void* handler)
    {
        if (key.empty())
        {
            return false;
        }

        RSA* context = nullptr;
        {
            CipherStackRecovery leave([&] () { if (nullptr != context) { RSA_free(context); } });
            {
                if (CreateCipherRsa(key, std::addressof(context), method, callback, handler))
                {
                    if (nullptr == context)
                    {
                        return false;
                    }

                    unsigned char crypto[1024] = {};
                    {
                        return [&] (const i32 size)
                        {
                            if (-1 == size)
                            {
                                return false;
                            }

                            std::string temp;

                            if (BinaryToString(std::string(reinterpret_cast<const char*>(crypto), size), temp))
                            {
                                buffer.swap(*const_cast<std::string*>(std::addressof(static_cast<const std::string&>(std::move(std::string(reinterpret_cast<char*>(crypto), size))))));
                                //EVENT_TRACE("%s %s", enc ? "GetCipherRsa Encrypt:" : "GetCipherRsa Dncrypt:", temp.c_str());
                                return true;
                            }
                            return 0 != size;

                        }(enc ? method ? RSA_public_encrypt(static_cast<i32>(source.size()), reinterpret_cast<const unsigned char*>(source.data()), crypto, context, padding) : RSA_private_encrypt(static_cast<i32>(source.size()), reinterpret_cast<const unsigned char*>(source.data()), crypto, context, padding) : method ? RSA_public_decrypt(static_cast<i32>(source.size()), reinterpret_cast<const unsigned char*>(source.data()), crypto, context, padding) : RSA_private_decrypt(static_cast<i32>(source.size()), reinterpret_cast<const unsigned char*>(source.data()), crypto, context, padding));
                    }
                }
            }
        }

        return false;
    }

    NOFORCEINLINE bool GetCipherRsaSign(const std::string& source, std::string& buffer, const std::string& key, const u32 method, CertificateCipherCallback callback, void* handler)
    {
        if (key.empty())
        {
            return false;
        }

        if (buffer.size())
        {
            buffer.clear();
            buffer.shrink_to_fit();
        }

        std::string buffer_string;
        {
            if (GetCipherSha(source, buffer, buffer_string))
            {
                RSA* context = nullptr;
                {
                    CipherStackRecovery leave([&] () { if (nullptr != context) { RSA_free(context); } });
                    {
                        if (CreateCipherRsa(key, std::addressof(context), method, callback, handler))
                        {
                            if (nullptr == context)
                            {
                                return false;
                            }

                            unsigned char crypto[8192] = {}; unsigned int crypto_size = 0;
                            {
                                if (1 == RSA_sign(NID_sha1,
                                                  reinterpret_cast<const unsigned char*>(buffer.data()),
                                                  buffer.size(),
                                                  crypto,
                                                  std::addressof(crypto_size),
                                                  context))
                                {
                                    buffer.swap(*const_cast<std::string*>(std::addressof(static_cast<const std::string&>(std::move(std::string(reinterpret_cast<char*>(crypto), crypto_size))))));
                                }

                                return 0 != crypto_size;
                            }
                        }
                    }
                }
            }
        }

        return false;
    }

    NOFORCEINLINE bool GetCipherRsaVerifySign(const std::string& source, std::string& buffer, const std::string& sign, const std::string& key, const u32 method, CertificateCipherCallback callback, void* handler)
    {
        std::string buffer_string;
        {
            if (buffer.size())
            {
                buffer.clear();
                buffer.shrink_to_fit();
            }

            if (GetCipherSha(source, buffer, buffer_string))
            {
                RSA* context = nullptr;
                {
                    CipherStackRecovery leave([&] () { if (nullptr != context) { RSA_free(context); } });
                    {
                        if (CreateCipherRsa(key, std::addressof(context), method, callback, handler))
                        {
                            if (nullptr == context)
                            {
                                return false;
                            }

                            return 1 == RSA_verify(NID_sha1,
                                                   reinterpret_cast<const unsigned char*>(buffer.data()),
                                                   buffer.size(),
                                                   reinterpret_cast<const unsigned char*>(sign.data()),
                                                   sign.size(),
                                                   context);
                        }
                    }
                }
            }
        }

        return false;
    }

    NOFORCEINLINE bool GetCipherEncryptData(const std::string& source, std::string& buffer, CertificateCipherCallback callback, void* handler)
    {
        std::string crypto;
        {
            //RSA_PKCS1_PADDING
            //RSA_PKCS1_OAEP_PADDING
            //RSA_SSLV23_PADDING
            //RSA_NO_PADDING

            if (0 == GetCipherAes(source, crypto, 1))
            {
                return false;
            }

            if (0 == GetCipherDes(crypto, crypto, 1))
            {
                return false;
            }

            if (0 == GetCipherRsa(crypto, crypto, rsa_public_key, 1, 1, RSA_PKCS1_PADDING, callback, handler))
            {
                return false;
            }

            if (0 == GetCipherBase64(crypto, buffer, 1))
            {
                return false;
            }
        }

        return true;
    }

    bool GetCipherDecryptData(const std::string& source, std::string& buffer, CertificateCipherCallback callback, void* handler)
    {
        std::string crypto;
        {
            //RSA_PKCS1_PADDING
            //RSA_PKCS1_OAEP_PADDING
            //RSA_SSLV23_PADDING
            //RSA_NO_PADDING

            if (0 == GetCipherBase64(source, crypto, 0))
            {
                return false;
            }

            if (0 == GetCipherRsa(crypto, crypto, rsa_private_key, 0, 0, RSA_PKCS1_PADDING, callback, handler))
            {
                return false;
            }

            if (0 == GetCipherDes(crypto, crypto, 0))
            {
                return false;
            }

            if (0 == GetCipherAes(crypto, buffer, 0))
            {
                return false;
            }
        }

        return true;
    }

}
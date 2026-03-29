#include <napi.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <cstring>
#include <string>

struct crypto_req {
    unsigned char key[32];
    int key_len;
    unsigned char input[1024];
    int input_len;
    unsigned char output[1024];
    int output_len;
};

#define CRYPTO_MAGIC 'C'
#define IOCTL_ENCRYPT _IOWR(CRYPTO_MAGIC, 1, struct crypto_req)
#define IOCTL_DECRYPT _IOWR(CRYPTO_MAGIC, 2, struct crypto_req)

Napi::Value ProcessCrypto(const Napi::CallbackInfo& info, int ioctl_cmd) {
    Napi::Env env = info.Env();
    
    // 1. Nhận input dưới dạng Buffer nhị phân (chống hỏng data)
    Napi::Buffer<uint8_t> input_buf = info[0].As<Napi::Buffer<uint8_t>>();
    std::string key = info[1].As<Napi::String>().Utf8Value();

    struct crypto_req req;
    memset(&req, 0, sizeof(req)); // Khởi tạo toàn bộ bằng 0 (Zero-padding)
    
    strncpy((char*)req.key, key.c_str(), sizeof(req.key) - 1);
    req.key_len = key.length();
    
    size_t len = input_buf.Length();
    if (len > sizeof(req.input)) len = sizeof(req.input);
    memcpy(req.input, input_buf.Data(), len);

    // 2. Padding block size cho thuật toán AES (Bắt buộc chia hết cho 16)
    if (len == 0) {
        req.input_len = 16;
    } else {
        req.input_len = ((len + 15) / 16) * 16; // Tự động làm tròn lên
    }

    int fd = open("/dev/crypto_dev", O_RDWR);
    if (fd < 0) return Napi::String::New(env, "ERROR_OPEN_DRIVER");

    if (ioctl(fd, ioctl_cmd, &req) < 0) {
        close(fd);
        return Napi::String::New(env, "ERROR_IOCTL");
    }
    close(fd);

    // 3. Trả về kết quả dưới dạng Buffer nhị phân cho Node.js
    return Napi::Buffer<uint8_t>::Copy(env, req.output, req.input_len); 
}

Napi::Value EncryptMsg(const Napi::CallbackInfo& info) {
    return ProcessCrypto(info, IOCTL_ENCRYPT);
}

Napi::Value DecryptMsg(const Napi::CallbackInfo& info) {
    return ProcessCrypto(info, IOCTL_DECRYPT);
}

Napi::Object Init(Napi::Env env, Napi::Object exports) {
    exports.Set("encrypt", Napi::Function::New(env, EncryptMsg));
    exports.Set("decrypt", Napi::Function::New(env, DecryptMsg));
    return exports;
}

NODE_API_MODULE(crypto_addon, Init)

#ifndef CRYPTO_IOCTL_H
#define CRYPTO_IOCTL_H

#include <linux/ioctl.h>

// Định nghĩa cấu trúc gói tin gửi xuống Kernel
struct crypto_req {
    unsigned char key[32];      // Khóa bí mật (hỗ trợ tối đa 256-bit)
    int key_len;                // Độ dài khóa
    unsigned char input[1024];  // Tin nhắn đầu vào
    int input_len;              // Độ dài tin nhắn
    unsigned char output[1024]; // Kết quả trả về sau khi mã hóa/giải mã
    int output_len;             // Độ dài kết quả
};

#define CRYPTO_MAGIC 'C'
#define IOCTL_ENCRYPT _IOWR(CRYPTO_MAGIC, 1, struct crypto_req)
#define IOCTL_DECRYPT _IOWR(CRYPTO_MAGIC, 2, struct crypto_req)

#endif

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/uaccess.h>
#include <crypto/skcipher.h>
#include <linux/scatterlist.h>
#include "crypto_ioctl.h"

#define DEVICE_NAME "crypto_dev"
#define CLASS_NAME "crypto_class"

static int major_num;
static struct class* crypto_class = NULL;
static struct device* crypto_device = NULL;
static struct cdev crypto_cdev;

// Hàm xử lý mã hóa / giải mã dùng Crypto API của Kernel
static int process_crypto(struct crypto_req *req, int is_encrypt) {
    struct crypto_skcipher *skcipher = NULL;
    struct skcipher_request *sk_req = NULL;
    struct scatterlist sg_in, sg_out;
    int ret = -EFAULT;

    // Khởi tạo thuật toán AES-ECB
    skcipher = crypto_alloc_skcipher("ecb(aes)", 0, 0);
    if (IS_ERR(skcipher)) return PTR_ERR(skcipher);

    // Cài đặt Key
    if (crypto_skcipher_setkey(skcipher, req->key, req->key_len)) {
        crypto_free_skcipher(skcipher);
        return -EFAULT;
    }

    sk_req = skcipher_request_alloc(skcipher, GFP_KERNEL);
    if (!sk_req) {
        crypto_free_skcipher(skcipher);
        return -ENOMEM;
    }

    // Thiết lập vùng nhớ (Scatterlist)
    sg_init_one(&sg_in, req->input, req->input_len);
    sg_init_one(&sg_out, req->output, req->input_len);
    skcipher_request_set_crypt(sk_req, &sg_in, &sg_out, req->input_len, NULL);

    // Thực thi mã hóa hoặc giải mã
    if (is_encrypt)
        ret = crypto_skcipher_encrypt(sk_req);
    else
        ret = crypto_skcipher_decrypt(sk_req);

    req->output_len = req->input_len; // Output dài bằng Input (với ECB)

    skcipher_request_free(sk_req);
    crypto_free_skcipher(skcipher);
    return ret;
}

#include <linux/slab.h> // [THÊM DÒNG NÀY LÊN TRÊN CÙNG để gọi kmalloc/kfree]

static long dev_ioctl(struct file *filep, unsigned int cmd, unsigned long arg) {
    struct crypto_req *req; // Đổi thành con trỏ
    long ret = 0;

    // 1. Cấp phát bộ nhớ trên vùng Heap rộng rãi của Kernel
    req = kmalloc(sizeof(struct crypto_req), GFP_KERNEL);
    if (!req) {
        return -ENOMEM; // Hết bộ nhớ
    }
    
    // 2. Copy dữ liệu từ User Space (Node.js) xuống Kernel Space
    // (Bây giờ req đã là con trỏ nên không cần dấu & nữa)
    if (copy_from_user(req, (struct crypto_req *)arg, sizeof(struct crypto_req))) {
        kfree(req); // Nhớ dọn rác nếu lỗi
        return -EFAULT;
    }

    // 3. Xử lý thuật toán
    switch(cmd) {
        case IOCTL_ENCRYPT:
            if (process_crypto(req, 1) != 0) ret = -EFAULT;
            break;
        case IOCTL_DECRYPT:
            if (process_crypto(req, 0) != 0) ret = -EFAULT;
            break;
        default:
            ret = -EINVAL;
    }

    // 4. Copy kết quả từ Kernel Space ngược lên User Space
    if (ret == 0) {
        if (copy_to_user((struct crypto_req *)arg, req, sizeof(struct crypto_req))) {
            ret = -EFAULT;
        }
    }

    // 5. Giải phóng bộ nhớ trước khi thoát (Quan trọng nhất!)
    kfree(req);
    return ret;
}

static struct file_operations fops = {
    .unlocked_ioctl = dev_ioctl,
};

static int __init crypto_drv_init(void) {
    dev_t dev_id;
    alloc_chrdev_region(&dev_id, 0, 1, DEVICE_NAME);
    major_num = MAJOR(dev_id);

    cdev_init(&crypto_cdev, &fops);
    cdev_add(&crypto_cdev, dev_id, 1);

    crypto_class = class_create(CLASS_NAME);
    crypto_device = device_create(crypto_class, NULL, dev_id, NULL, DEVICE_NAME);

    printk(KERN_INFO "Crypto Driver Loaded! Major: %d\n", major_num);
    return 0;
}

static void __exit crypto_drv_exit(void) {
    dev_t dev_id = MKDEV(major_num, 0);
    device_destroy(crypto_class, dev_id);
    class_destroy(crypto_class);
    cdev_del(&crypto_cdev);
    unregister_chrdev_region(dev_id, 1);
    printk(KERN_INFO "Crypto Driver Unloaded!\n");
}

module_init(crypto_drv_init);
module_exit(crypto_drv_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Bro");
MODULE_DESCRIPTION("AES Crypto Kernel Module");

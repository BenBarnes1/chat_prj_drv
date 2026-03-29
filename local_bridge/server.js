const express = require('express');
const cors = require('cors');
const kernelCrypto = require('./build/Release/crypto_addon.node'); 

const app = express();
app.use(cors());
app.use(express.json());

// SỬA LỖI 1: Key phải DÀI ĐÚNG 32 BYTES (256 bits)
const SHARED_KEY = "12345678901234567890123456789012";

app.post('/encrypt', (req, res) => {
    const { text } = req.body;
    if (!text) return res.json({ result: "" });
    
    // Ép text thành Buffer nhị phân rồi gọi Driver
    const inputBuf = Buffer.from(text, 'utf-8');
    const cipherData = kernelCrypto.encrypt(inputBuf, SHARED_KEY);
    
    // Trả lỗi nếu Driver có biến
    if (typeof cipherData === 'string') return res.json({ result: cipherData }); 
    
    // Ép cục binary đã mã hóa thành chuỗi chuẩn HEX để React cầm đi lưu DB
    res.json({ result: cipherData.toString('hex') });
});

app.post('/decrypt', (req, res) => {
    const { text } = req.body;
    if (!text) return res.json({ result: "" });
    
    // BỘ LỌC 1: Kiểm tra xem có phải mã HEX chuẩn không?
    // Nếu text là chữ bình thường (ví dụ: ERROR_IOCTL, hoặc tin nhắn cũ chưa mã hóa) 
    // thì trả về nguyên gốc, không thèm giải mã nữa!
    const isHex = /^[0-9a-fA-F]+$/.test(text);
    if (!isHex) {
        return res.json({ result: text });
    }
    
    try {
        const inputBuf = Buffer.from(text, 'hex');
        const plainData = kernelCrypto.decrypt(inputBuf, SHARED_KEY);
        
        if (typeof plainData === 'string') return res.json({ result: plainData });
        
        let plainText = plainData.toString('utf-8');
        plainText = plainText.replace(/\0/g, ''); 
        
        res.json({ result: plainText });
    } catch (error) {
        // BỘ LỌC 2: Nếu giải mã thất bại (do sai Key cũ), trả về chữ gốc thay vì văng lỗi rác
        console.error("Lỗi giải mã:", error);
        res.json({ result: text });
    }
});

app.listen(9999, () => {
    console.log("Local Bridge đang chạy ở port 9999... Đã vá lỗi AES Padding!");
});

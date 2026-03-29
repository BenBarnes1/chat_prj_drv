# Hướng Dẫn Setup Fullstack Chat App (Tích hợp Kernel Crypto)

Dự án này chạy Backend trên Windows và Frontend + Kernel Driver trên máy ảo Ubuntu (VMware). Hãy làm theo đúng thứ tự dưới đây.

## BƯỚC 1: SETUP TRÊN MÁY WINDOWS (BACKEND)

1. Mở cmd, gõ `ipconfig` để lấy địa chỉ **IPv4 Address** của máy thật (VD: `192.168.1.15`). Ghi nhớ IP này.
2. Tắt Firewall: Vào Windows Defender Firewall -> **Turn off** cả Private và Public.
3. Mở MongoDB Compass, đảm bảo có connection `mongodb://localhost:27017` đang chạy.
4. Mở terminal tại thư mục `backend`:
   - Chạy lệnh: `npm install`
   - **QUAN TRỌNG:** Tạo một file tên là `.env` ngang hàng với `package.json` và điền thông tin sau:
     ```env
     PORT=5001
     MONGODB_URI=mongodb://localhost:27017/fullstack_chat_app
     JWT_SECRET=cai_gi_cung_duoc
     NODE_ENV=development
     CLOUDINARY_CLOUD_NAME=your_name
     CLOUDINARY_API_KEY=your_key
     CLOUDINARY_API_SECRET=your_secret
     ```
   - Chạy Server: `npm run dev`

---

## BƯỚC 2: SETUP TRÊN MÁY ẢO UBUNTU (Yêu cầu Node.js v20+)
*Lưu ý: Copy 3 folder `kernel_crypto`, `local_bridge`, `frontend_local` ra ngoài thư mục Home (`~`) của Ubuntu để tránh lỗi phân quyền.*

### Trạm 1: Build và Nạp Kernel Driver
1. Mở Terminal, đi tới thư mục driver: `cd ~/kernel_crypto`
2. Biên dịch code C: `make`
3. Nạp driver vào nhân Linux: `sudo insmod crypto_drv.ko`
4. Cấp quyền đọc ghi: `sudo chmod 666 /dev/crypto_dev`
5. Kiểm tra xem driver lên chưa: `dmesg | tail` (Thấy báo Loaded là OK).

### Trạm 2: Bật Cầu Nối C++ (Local Bridge)
1. Mở Terminal mới: `cd ~/local_bridge`
2. Cài thư viện: `npm install`
3. Cài công cụ build C++: `npm install -g node-gyp` (Dùng sudo nếu lỗi quyền).
4. Build mã C++ thành module Node.js:
   `node-gyp configure`
   `node-gyp rebuild`
5. Chạy cầu nối: `node server.js`
   *(Màn hình báo "Local Bridge đang chạy ở port 9999..." là OK).*

### Trạm 3: Bật Frontend React
1. Mở thư mục `frontend_local` bằng VSCode.
2. Tìm mở file `vite.config.js`. Sửa đoạn IP `target` thành cái **IP của máy Windows** lấy ở Bước 1.
   ```javascript
   proxy: {
     "/api": {
       target: "http://[IP_MÁY_WINDOWS_CỦA_BẠN]:5001",
       changeOrigin: true,
     }
     "socket":{
        target:
     }
   }
   chạy : npm install && npm run dev

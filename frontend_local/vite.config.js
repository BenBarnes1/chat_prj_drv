import { defineConfig } from 'vite'
import react from '@vitejs/plugin-react'

export default defineConfig({
  plugins: [react()],
  server: {
    proxy: {
      // 1. Chỉ đường cho API Axios (Login, lấy tin nhắn...)
      "/api": {
        target: "http://192.168.101.182:5001", // Chỗ duy nhất cần điền IP thật
        changeOrigin: true,
      },
      // 2. Chỉ đường cho Socket.io (Tin nhắn Realtime)
      "/socket.io": {
        target: "http://192.168.101.182:5001", // Chỗ duy nhất cần điền IP thật
        ws: true, // <-- Bắt buộc phải có cái này để chạy WebSocket
      }
    }
  }
})

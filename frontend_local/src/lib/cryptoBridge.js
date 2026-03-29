import axios from "axios";

// Trỏ thẳng vào cái Local Bridge Node.js đang chạy ngầm của bro
const bridgeApi = axios.create({
  baseURL: "http://localhost:9999",
});

export const encryptText = async (text) => {
  if (!text) return text;
  try {
    const res = await bridgeApi.post("/encrypt", { text });
    return res.data.result; // Trả về chuỗi loằng ngoằng "@#&*!"
  } catch (error) {
    console.error("Lỗi gọi Driver Mã hóa:", error);
    return text; // Nếu sập cầu thì đành gửi chữ gốc
  }
};

export const decryptText = async (text) => {
  if (!text) return text;
  try {
    const res = await bridgeApi.post("/decrypt", { text });
    return res.data.result; // Trả về chữ tiếng Việt đọc được
  } catch (error) {
    console.error("Lỗi gọi Driver Giải mã:", error);
    return text;
  }
};

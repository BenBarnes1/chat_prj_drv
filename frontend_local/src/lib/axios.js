import axios from "axios";

export const axiosInstance = axios.create({
  baseURL: "/api", // Chỉ cần thế này thôi
  withCredentials: true,
});

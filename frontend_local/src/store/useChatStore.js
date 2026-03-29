import { create } from "zustand";
import toast from "react-hot-toast";
import { axiosInstance } from "../lib/axios";
import { useAuthStore } from "./useAuthStore";
import { encryptText, decryptText } from "../lib/cryptoBridge";
// (Giữ nguyên các import cũ của bro ở trên)
export const useChatStore = create((set, get) => ({
  messages: [],
  users: [],
  selectedUser: null,
  isUsersLoading: false,
  isMessagesLoading: false,

  getUsers: async () => {
    set({ isUsersLoading: true });
    try {
      const res = await axiosInstance.get("/messages/users");
      set({ users: res.data });
    } catch (error) {
      toast.error(error.response.data.message);
    } finally {
      set({ isUsersLoading: false });
    }
  },

  getMessages: async (userId) => {
      set({ isMessagesLoading: true });
      try {
        const res = await axiosInstance.get(`/messages/${userId}`);
        
        // [THÊM MỚI]: Nhờ Driver giải mã toàn bộ tin nhắn tải về
        const decryptedMessages = await Promise.all(
          res.data.map(async (msg) => {
            if (msg.text) {
              msg.text = await decryptText(msg.text);
            }
            return msg;
          })
        );
  
        set({ messages: decryptedMessages });
      } catch (error) {
        toast.error(error.response.data.message);
      } finally {
        set({ isMessagesLoading: false });
      }
    },
  sendMessage: async (messageData) => {
      const { selectedUser, messages } = get();
      try {
        // [THÊM MỚI]: Nhờ Driver mã hóa cái chữ người dùng vừa gõ
        let finalText = messageData.text;
        if (finalText) {
          finalText = await encryptText(finalText);
        }
  
        // Tạo một object mới chứa chữ ĐÃ MÃ HÓA để gửi lên Server Windows
        const encryptedMessageData = {
          ...messageData,
          text: finalText,
        };
  
        const res = await axiosInstance.post(`/messages/send/${selectedUser._id}`, encryptedMessageData);
        
        // Tin Server trả về đang bị mã hóa, phải giải mã lại để hiển thị cho chính mình xem
        if (res.data.text) {
            res.data.text = await decryptText(res.data.text);
        }
  
        set({ messages: [...messages, res.data] });
      } catch (error) {
        toast.error(error.response.data.message);
      }
    },

  subscribeToMessages: () => {
    const { selectedUser } = get();
    if (!selectedUser) return;

    const socket = useAuthStore.getState().socket;

    socket.on("newMessage", async (newMessage) => { // Nhớ thêm async ở đây
      const isMessageSentFromSelectedUser = newMessage.senderId === selectedUser._id;
      if (!isMessageSentFromSelectedUser) return;

      // [THÊM MỚI]: Giải mã cục tin nhắn bay từ Socket về
      if (newMessage.text) {
          newMessage.text = await decryptText(newMessage.text);
      }

      set({
        messages: [...get().messages, newMessage],
      });
    });
  },
  unsubscribeFromMessages: () => {
    const socket = useAuthStore.getState().socket;
    socket.off("newMessage");
  },

  setSelectedUser: (selectedUser) => set({ selectedUser }),
}));

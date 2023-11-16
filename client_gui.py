from threading import *
from socket import *
import tkinter as tk
from tkinter import font, Scrollbar, Text, Entry, Button
from ast import literal_eval

class ChatGUI(tk.Tk):

    def __init__(self):
        super().__init__()

        # Window setup
        self.title("BokChat")
        self.geometry("500x500")
        self.configure(bg="#2c3e50")

        # Font setup
        self.customFont = font.Font(family="Arial", size=12)
        self.entryFont = font.Font(family="Arial", size=14)

        # Create chat display area
        self.text_area = Text(self, wrap=tk.WORD, state=tk.DISABLED, bg="#34495e", fg="white", font=self.customFont, bd=8, relief="flat", padx=10, pady=10)
        self.text_area.grid(row=0, column=0, sticky="nsew", padx=10, pady=(10,5))

        # Create a Scrollbar for the text area
        self.scrollbar = Scrollbar(self, command=self.text_area.yview)
        self.scrollbar.grid(row=0, column=1, sticky="ns")
        self.text_area.config(yscrollcommand=self.scrollbar.set)

        # Create message input frame
        self.input_frame = tk.Frame(self, bg="#2c3e50")
        self.input_frame.grid(row=1, column=0, sticky="ew", padx=10, pady=5)

        # Create message input area
        self.entry_var = tk.StringVar()
        self.entry = Entry(self.input_frame, textvariable=self.entry_var, bg="#34495e", fg="white", font=self.entryFont, relief="flat", bd=4)
        self.entry.pack(side="left", fill="both", expand=True, padx=(0,5))
        self.entry.bind("<Return>", self.get_entry)

        # Send button
        self.send_btn = Button(self.input_frame, text="Send", bg="#e74c3c", fg="white", font=self.entryFont, relief="flat", command=self.get_entry_button)
        self.send_btn.pack(side="right", padx=5)

        # Configure grid
        self.grid_rowconfigure(0, weight=1)
        self.grid_columnconfigure(0, weight=1)

        self.init_connect()

        self.mainloop()

        self.client.send("__EXIT__".encode())
        self.client.close()
        exit()

    def get_entry(self, event):  # remove the client parameter
        msg = self.entry.get()
        # self.client.send(str(encrypt(msg)).encode())
        self.client.send(msg.encode());
        self.entry.delete(0, tk.END)

    def get_entry_button(self):  # remove the client parameter
        msg = self.entry.get()
        # self.client.send(str(encrypt(msg)).encode())
        self.client.send(msg.encode());
        self.entry.delete(0, tk.END)

    def update_message_box(self, msg: str):
        self.text_area.config(state=tk.NORMAL)
        self.text_area.insert(tk.END, msg + "\n")
        self.text_area.config(state=tk.DISABLED)
        self.text_area.see(tk.END)

    def init_connect(self):
        address = ("127.0.0.1", 16262)
        self.client = socket(AF_INET, SOCK_STREAM)  # store as instance variable
        self.client.connect(address)
        recv_thread = Thread(target=self.recv_msg, args=(self.client,))
        recv_thread.start()

    def recv_msg(self, client: socket):
        while True:
            try:
                msg = client.recv(1024).decode()
                if msg == "__NICK__":
                    self.update_message_box("Enter your nickname: ")
                elif "__ENCRYPTED__" in msg:
                    split_str = msg.split(": ")
                    print(split_str)
                    encrypted_data = literal_eval(split_str[1])
                    self.update_message_box(decrypt(encrypted_data))
                else:
                    self.update_message_box(msg)
            except:
                client.close()
                break


def encrypt(msg: str) -> list[int]:
    pub_ex = 20743615
    n = 53335391
    convert_msg = [ord(c) for c in msg]
    encrypted_msg = [pow(c, pub_ex, n) for c in convert_msg]
    return encrypted_msg


def decrypt(msg: list[int]) -> str:
    priv_ex = 27059311
    n = 53335391
    decoded_msg = [pow(c, priv_ex, n) for c in msg]
    decrypted_msg = "".join(chr(c) for c in decoded_msg)
    return decrypted_msg


def main():
    gui = ChatGUI()


if __name__ == "__main__":
    main()

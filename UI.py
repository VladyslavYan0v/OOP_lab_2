import tkinter as tk
from tkinter import messagebox
from db_manager import DatabaseManager
import crypto_engine
import time

class AlgorithmStrategy:
    def execute(self, data: str) -> str:
        pass

class CppCipherAdapter(AlgorithmStrategy):
    def __init__(self, chipher_type):
        self.facade = crypto_engine.CryptoFacade()
        self.type = chipher_type
        self.config = crypto_engine.CryptoConfig()
        
        self.config.rsaE, self.config.rsaD, self.config.rsaN = 17, 2753, 3233
        self.config.shamirE, self.config.shamirD, self.config.shamirP = 11, 2921, 3571
        self.config.caesarShift = 5
        self.config.xorKey = 170

    def execute(self, data: str, callback=None) -> str:
        try:
            if callback:
                callback("Preparing data", 0)
            
            int_data = [ord(c) for c in data]

            if callback:
                callback(f"Sending to C++", 1)

            response = self.facade.sendSecretMessage(int_data, self.type, self.config)

            self.last_difficulty = response.timeComplexity
            self.last_time = response.timeMs
            self.last_memory = response.memoryBytes / 1024
            
            if callback:
                callback("Encrypted successful", 2)
            
            return " ".join(map(str, response.encryptedData))
        except Exception as e:
            return f"C++ Error: {str(e)}"

class AlgorithmFactory:
    @staticmethod
    def get_algorithm(name):
        match name:
            case "RSA Encryption":
                return CppCipherAdapter(crypto_engine.CipherType.RSA)
            case "Shamir Protocol":
                return CppCipherAdapter(crypto_engine.CipherType.SHAMIR)
            case "Caesar Cipher":
                return CppCipherAdapter(crypto_engine.CipherType.CAESAR)
            case "XOR Cipher":
                return CppCipherAdapter(crypto_engine.CipherType.XOR)
            case "Double Secure":
                return CppCipherAdapter(crypto_engine.CipherType.DOUBLE_SECURE)
            case "Multi-Layer Symmetric":
                return CppCipherAdapter(crypto_engine.CipherType.MULTI_LAYER_SYMMETRIC)
            case _:
                return None

class UI():
    def __init__(self, root):
        self.root = root
        self.title = "title"
        self.root.geometry("1280x720")
        self.root.configure(bg="#E5E8F0")
        
        self.current_algorithm = None
        self.encrypted_message = ""
        self.algorithm_buttons = {}

        self.db = DatabaseManager()

        self.algorithms = ["RSA Encryption", "Shamir Protocol", "Double Secure", "Caesar Cipher", "XOR Cipher", "Multi-Layer Symmetric"]

        self.frame = tk.Frame(self.root, width=300, bg="#FFFFFF", height=1280)
        self.frame.pack(side="left", fill="y")

        self.column_count = 2

        for index, name in enumerate(self.algorithms):
            lambda name=name:self.algorithm_menu(name)
            btn = tk.Button(self.frame, text=name, bg="#FFFFFF", fg="#008AA4", relief="flat", borderwidth=0,
                             activebackground="yellow", activeforeground="black", bd=0, font=("Arial black", 11),
                             command=lambda n= name: self.algorithm_menu(n))

            r = index // self.column_count
            c = index % self.column_count

            btn.grid(row=r, column=c, padx=50, pady=50, sticky="ew")
            
            self.algorithm_buttons[name] = btn
        
        self.algorithm_frame = tk.Frame(self.root)
        self.algorithm_frame.pack(side="left", fill='both', expand=True)

        
    def algorithm_menu(self, algorithm_name):
        self.current_algorithm_name = algorithm_name
        self.current_algorithm = AlgorithmFactory.get_algorithm(algorithm_name)

        for widget in self.algorithm_frame.winfo_children():
            widget.destroy()

        for btn in self.algorithm_buttons.values():
            btn.configure(bg="#FFFFFF")
        if algorithm_name in self.algorithm_buttons:
            self.algorithm_buttons[algorithm_name].configure(bg="#E5E8F0")

        self.encrypted_message = ""

        self.difficulty_label = tk.Label(self.algorithm_frame, fg="black", font=("Arial black", 20))
        self.difficulty_label.place(x=10, y=50)
        self.time_usage_label = tk.Label(self.algorithm_frame, fg="black", font=("Arial black", 24))
        self.time_usage_label.place(x=10, y=150)
        self.memory_usage_label = tk.Label(self.algorithm_frame, fg="black", font=("Arial black", 24))
        self.memory_usage_label.place(x=10, y=250)

        self.diagram_frame = tk.Frame(self.algorithm_frame, width=400, height=300)
        self.diagram_frame.place(x=420,y=30)

        self.text_message = tk.Label(self.algorithm_frame, text="Your encrypted massage: ", fg="black", font=("Arial black", 12))
        self.text_message.place(x=10, y=430)
        self.encrypted_message_label = tk.Label(self.algorithm_frame, text=self.encrypted_message,
                                                fg="black",font=("Arial black", 12), width=100, anchor='w')
        self.encrypted_message_label.place(x=250, y=430)

        self.enter_text_message = tk.Label(self.algorithm_frame, text="Enter your massage: ", fg="black", font=("Arial black", 12))
        self.enter_text_message.place(x=335, y=600)
        self.enter_text = tk.Entry(self.algorithm_frame, fg="black", bg="white", bd=2, width=110)
        self.enter_text.place(x=100, y=650)
        self.enter_text.bind('<Return>', self.handle_enter)

    def handle_enter(self, event=None):
        if self.current_algorithm is None:
            messagebox.showwarning("Warning", "Please select an algorithm first")
            return
        
        message_to_encrypt = self.enter_text.get()
        if not message_to_encrypt:
            return

        self.encrypted_message_label.config(text="")
        self.clear_diagram()

        self.encrypted_message = self.current_algorithm.execute(message_to_encrypt, callback=self.add_visual_step)

        self.time_usage_label.config(text=f"{self.current_algorithm.last_time:.4f}ms")
        self.difficulty_label.config(text=self.current_algorithm.last_difficulty)
        self.memory_usage_label.config(text=f"{self.current_algorithm.last_memory:.2f}Kb")

        self.encrypted_message_label.config(text=self.encrypted_message)

        try:
            self.db.save_result(
                self.current_algorithm_name, 
                self.enter_text.get(), 
                self.encrypted_message,
                self.current_algorithm.last_time,
                self.current_algorithm.last_difficulty,
                self.current_algorithm.last_memory
            )
        except Exception as e:
            print(f"Database Error: {e}")

        self.enter_text.delete(0, tk.END)

    def clear_diagram(self):
        for widget in self.diagram_frame.winfo_children():
            widget.destroy()
        
        self.canvas = tk.Canvas(self.diagram_frame, width=400, height=300, highlightthickness=0)
        self.canvas.pack(fill="both", expand=True)

    def add_visual_step(self, text, step_number):
        x_center = 200
        y_top = 20 + (step_number * 80)

        self.canvas.create_rectangle(50, y_top, 350, y_top + 40, fill="white", outline="#008AA4", width=2)
        self.canvas.create_text(x_center, y_top + 20, text=text, font=("Arial", 10, "bold"))

        if step_number > 0:
            prev_y_bottom = 20 + ((step_number - 1) * 80) + 40
            self.canvas.create_line(x_center, prev_y_bottom, x_center, y_top, arrow=tk.LAST, fill="black")
        
        self.root.update()
        time.sleep(0.5)

if __name__ == "__main__":
    root = tk.Tk()
    UI(root)
    root.mainloop()
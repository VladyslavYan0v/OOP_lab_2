import tkinter as tk

diffuculty = "O(n)"
time_use = 0.452
memory_usage = 2.4

class AlgorithmStrategy:
    def execute(self, data: str):
        pass

class ElGamalAdapter(AlgorithmStrategy):
    def __init__(self):
        pass
    def execute(self, data: str) -> str:
        try:
            m_value = int(data) % 27

            #result = elgamal_cpp.encrypt(m_value)

            #return f"k={result.k}, pair: ({result.a}, {result.b})"
        except ValueError:
            return "Error: Please enter a number"

class AlgorithmFactory:
    @staticmethod
    def get_algorithm(name):
        match name:
            case "ElGamal GF(27)":
                return ElGamalAdapter()
            case _:
                return None

class UI():
    def __init__(self, root):
        self.root = root
        self.title = "title"
        self.root.geometry("1280x720")
        self.root.configure(bg="#E5E8F0")
        self.encrypted_message = ""

        self.algorithms = ["algorithm 1", "algorithm 1", "algorithm 1", "algorithm 1", "algorithm 1", "algorithm 1",
                          "algorithm 1", "algorithm 1", "algorithm 1", "algorithm 1"]

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
        
        self.algorithm_frame = tk.Frame(self.root)
        self.algorithm_frame.pack(side="left", fill='both', expand=True)

        
    def algorithm_menu(self, algorithm_name):
        for widget in self.algorithm_frame.winfo_children():
            widget.destroy()
        self.difficulty_label = tk.Label(self.algorithm_frame, text=diffuculty, fg="black", font=("Arial black", 24))
        self.difficulty_label.place(x=10, y=50)
        self.time_usage_label = tk.Label(self.algorithm_frame, text=f"{time_use}ms", fg="black", font=("Arial black", 24))
        self.time_usage_label.place(x=10, y=150)
        self.memory_usage_label = tk.Label(self.algorithm_frame, text=f"{memory_usage}Kb", fg="black", font=("Arial black", 24))
        self.memory_usage_label.place(x=10, y=250)

        self.diagram_frame = tk.Frame(self.algorithm_frame, width=400, height=300, bg="yellow")
        self.diagram_frame.place(x=450,y=30)

        self.text_message = tk.Label(self.algorithm_frame, text="Your encrypted massage: ", fg="black", font=("Arial black", 12))
        self.text_message.place(x=10, y=430)
        self.encrypted_message_label = tk.Label(self.algorithm_frame, text=self.encrypted_message,
                                                fg="black",font=("Arial black", 12), width=100, anchor='w')
        self.encrypted_message_label.place(x=250, y=430)

        self.enter_text_message = tk.Label(self.algorithm_frame, text="Enter your massage: ", fg="black", font=("Arial black", 12))
        self.enter_text_message.place(x=350, y=600)
        self.enter_text = tk.Entry(self.algorithm_frame, fg="black", bg="white", bd=2, width=120)
        self.enter_text.place(x=100, y=650)
        self.enter_text.bind('<Return>', self.handle_enter)

    def handle_enter(self, event=None):
        message_to_encrypt = self.enter_text.get()

        #self.encrypted_message = self.current_algorithm.execute(input_text)

        self.encrypted_message = message_to_encrypt
        self.encrypted_message_label.config(text=self.encrypted_message)
        self.enter_text.delete(0, tk.END)


if __name__ == "__main__":
    root = tk.Tk()
    UI(root)
    root.mainloop()
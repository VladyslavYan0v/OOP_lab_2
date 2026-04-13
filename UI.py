import tkinter as tk

class UI():
    def __init__(self, root):
        self.root = root
        self.title = "title"
        self.root.geometry("1280x720")
        self.root.configure(bg="#E5E8F0")

        self.algoritms = ["algoritm 1", "algoritm 1", "algoritm 1", "algoritm 1", "algoritm 1", "algoritm 1",
                          "algoritm 1", "algoritm 1", "algoritm 1", "algoritm 1"]

        self.frame = tk.Frame(self.root, width=300, bg="#FFFFFF" , height=1280)
        self.frame.pack(side="left", fill="y")

        self.column_count = 2

        for index, name in enumerate(self.algoritms):
            btn = tk.Button(self.frame, text=name, bg="#FFFFFF", fg="#008AA4", relief="flat", borderwidth=0,
                             activebackground="yellow", activeforeground="black", bd=0, font=("Arial black", 11))

            r = index // self.column_count
            c = index % self.column_count

            btn.grid(row=r, column=c, padx=50, pady=50, sticky="ew")

if __name__ == "__main__":
    root = tk.Tk()
    UI(root)
    root.mainloop()
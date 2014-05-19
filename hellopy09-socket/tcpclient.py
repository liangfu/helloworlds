
from socket import *
from Tkinter import *
import sys

class Application(Frame):
    def send_st(self):
        HOST='localhost'
        PORT=50007
        self.ss=socket(AF_INET,SOCK_STREAM)
        self.ss.connect( (HOST,PORT) )
        sys.stderr.write('Connected!\n')
        self.ss.sendall('ST')
        sys.stderr.write('Sent:ST\n')
        self.ss.close()

    def send_ed(self):
        HOST='localhost'
        PORT=50007
        self.ss=socket(AF_INET,SOCK_STREAM)
        self.ss.connect( (HOST,PORT) )
        sys.stderr.write('Connected!\n')
        self.ss.sendall('ED')
        sys.stderr.write('Sent:ED\n')
        self.ss.close()

    def send_a(self):
        HOST='localhost'
        PORT=50007
        self.ss=socket(AF_INET,SOCK_STREAM)
        self.ss.connect( (HOST,PORT) )
        sys.stderr.write('Connected!\n')
        self.ss.sendall('A ')
        sys.stderr.write('Sent: A \n')
        self.ss.close()

    def createWidgets(self):
        self.QUIT = Button(self)
        self.QUIT["text"] = "QUIT"
        self.QUIT["fg"]   = "red"
        self.QUIT["command"] =  self.quit
        self.QUIT.pack({"side": "left"})

        self._st = Button(self)
        self._st["text"] = "ST"
        self._st["command"] = self.send_st
        self._st.pack({"side": "left"})

        self._ed = Button(self)
        self._ed["text"] = "ED"
        self._ed["command"] = self.send_ed
        self._ed.pack({"side": "left"})

        self._a = Button(self)
        self._a["text"] = "A"
        self._a["command"] = self.send_a
        self._a.pack({"side": "left"})

    def __init__(self, master=None):
        Frame.__init__(self, master)
        self.pack()
        self.createWidgets()

if __name__=="__main__":
    root=Tk()
    app=Application(master=root)
    app.mainloop()

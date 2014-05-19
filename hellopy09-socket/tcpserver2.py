import socket
import sys
import time
from Tkinter import *
import threading

HOST = ''
PORT = 50007
s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
s.bind((HOST, PORT))
s.listen(100)

class Application(Frame):
    def __init__(self, master=None):
        Frame.__init__(self, master)
        self.pack()
        self.createWidgets()
        self.listener=[]
    def createWidgets(self):
        self._st = Text(self)
        self._st.pack(fill=BOTH,expand=1)
        
        self.QUIT = Button(self)
        self.QUIT["text"] = "QUIT"
        self.QUIT["fg"]   = "red"
        self.QUIT["command"] =  self.quitx
        self.QUIT.pack(side=RIGHT)
    def setListener(self,listener):
        self.listener=listener
        self.listener[0].start()
    def quitx(self):
        self.listener[0].stop()
        Frame.quit(self)

class Listener(threading.Thread):
    def __init__(self,app=None):
        threading.Thread.__init__(self)
        self._stop=threading.Event()
        self.app=app
        self.ED_count=0
    def stop(self):
        self._stop.set()
    def stopped(self):
        return self._stop.is_set()
    def run(self):
        while not self._stop.is_set():
            conn, addr = s.accept()
            self.app._st.insert(1.0,'Connected by %s\n' % str(addr))
            data = conn.recv(1024)
            if len(str(data))>0:
                self.app._st.insert(1.0,'Received: ')
                self.app._st.insert(1.0,'%s\n' % str(data))

if __name__=="__main__":
    root=Tk()
    app=Application(master=root)
    listener=Listener(app)
    app.setListener([listener])
    app.mainloop()

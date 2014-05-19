import socket
import sys
import time

HOST = ''
PORT = 50007
s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
s.bind((HOST, PORT))
s.listen(100)

while 1:
    conn, addr = s.accept()
    sys.stderr.write('Connected by %s\n' % str(addr))
    data = conn.recv(1024)
    if len(str(data))>0:
        sys.stderr.write('Received: %s\n' % str(data))

s.close()


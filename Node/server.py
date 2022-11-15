import socket
import threading

HEADER = 64
PORT = 5050
FORMAT = "utf-8"
SERVER = socket.gethostbyname(socket.gethostname())
ADDRESS = (SERVER,PORT)
DISCONNECT_MESSAGE = "PUTO"
server = socket.socket(socket.AF_INET,socket.SOCK_STREAM)
server.bind(ADDRESS)



def handle_client(conn,addr):
    print("New CONN", addr)
    connected =True
    while connected:
        msg_length = conn.recv(HEADER).decode(FORMAT)
        if msg_length:
            msg_length = int(msg_length)
            msg = conn.recv(msg_length).decode(FORMAT)
            if msg == DISCONNECT_MESSAGE:
                connected = False
        print(addr,msg)
    conn.close()


def start():
    server.listen()
    print("LISTENING",SERVER)
    while True:
        conn , addr = server.accept()
        thread = threading.Thread(target=handle_client, args=(conn,addr))
        thread.start()
        print("[ACTIVE CONN",threading.active_count()-1)
print("STARTING")
start()        


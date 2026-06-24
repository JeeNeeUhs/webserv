import socket
from time import sleep

def test_body_timeout():
	s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
	s.connect(('127.0.0.1', 8080))

	print("connected to server, sending header...")
	req = b"POST /cgi-bin/cat.sh HTTP/1.1\r\nContent-Length: 100\r\n\r\n"
	s.sendall(req)

	sleep(35)

	res = s.recv(4096)
	print("header sent, waiting for response")
	print()
	print(res.decode('utf-8'))

if __name__ == "__main__":
	test_body_timeout()

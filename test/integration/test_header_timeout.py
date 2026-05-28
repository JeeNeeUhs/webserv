import socket

def test_header_timeout():
	s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
	s.connect(('127.0.0.1', 8080))

	print("connected to server, sending incomplete header...")
	req = b"POST / HTTP/1.1\r\nContent-Length: 100"
	s.sendall(req)

	res = s.recv(4096)
	print("incomplete header sent, waiting for response")
	print()
	print(res.decode('utf-8'))

if __name__ == "__main__":
	test_header_timeout()

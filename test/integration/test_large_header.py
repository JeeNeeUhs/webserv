import socket

def test_large_header():
	s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
	s.connect(('127.0.0.1', 8080))

	print("connected to server, sending header...")
	req = b"GET / HTTP/1.1\r\nUser-Agent: testere/1.0\r\n"
	req += b"X-Large-Header: " + (b"A" * 1024 * 1024) + b"\r\n"
	req += b"\r\n"
	s.sendall(req)

	res = s.recv(4096)
	print("header sent, waiting for response")
	print()
	print(res.decode('utf-8'))

if __name__ == "__main__":
	test_large_header()

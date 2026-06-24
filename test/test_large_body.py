import socket

def test_large_body():
	s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
	s.connect(('127.0.0.1', 8080))

	body_size = 5 * 1024 * 1024 * 1024

	print("connected to server, sending header...")
	req = b"POST /cgi-bin/cat.sh HTTP/1.0\r\n"
	req += f"Content-Length: {body_size}\r\n".encode('utf-8')
	req += b"\r\n"
	s.sendall(req)

	chunk_size = 64 * 1024 * 1024
	chunk = b"a" * chunk_size

	sent_bytes = 0
	print("header sent, sending body...")
	while sent_bytes < body_size:
		remaining = body_size - sent_bytes
		curr_chunk_size = min(chunk_size, remaining)

		if curr_chunk_size == chunk_size:
			s.sendall(chunk)
		else:
			s.sendall(b"a" * curr_chunk_size)

		sent_bytes += curr_chunk_size
		mb_sent = sent_bytes / (1024 * 1024)
		print(f"sent {mb_sent:.2f}mb")

	response = s.recv(4096)
	print("getting response from server:")
	print()
	print(response.decode('utf-8'))

if __name__ == "__main__":
	test_large_body()

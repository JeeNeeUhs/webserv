import socket
import time

def test_chunked_delay():
	s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
	s.connect(('127.0.0.1', 8080))

	print("connected to server, sending header...")
	req = b"POST / HTTP/1.1\r\nTransfer-Encoding: chunked\r\n\r\n"
	s.sendall(req)

	time.sleep(1)
	
	print("sending 4 bytes: ilk")
	s.sendall(b"4\r\nilk4\r\n")

	time.sleep(2)

	print("sending 6 bytes: ikinci")
	s.sendall(b"6\r\nikinci\r\n")

	time.sleep(1)

	print("sending final byte")
	s.sendall(b"0\r\n\r\n")
	
	response = s.recv(4096)
	print("getting response from server:")
	print()
	print(response.decode('utf-8'))
	
	s.close()

if __name__ == "__main__":
	test_chunked_delay()

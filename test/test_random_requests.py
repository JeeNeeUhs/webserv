import socket 
from time import sleep

def test_bad_request():
	s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
	s.connect(('127.0.0.1', 8080))
	req = b"POST /uploads HTTP/1.1\r\n\r\n"
	s.sendall(req)
	sleep(0.1)
	res = s.recv(4096)

	if "400" in res.decode('utf-8'):
		print("PASSED!")
	else:
		print("Received response:")
		print(res.decode('utf-8'))
		print("FAILED!")

def test_not_implemented():
	s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
	s.connect(('127.0.0.1', 8080))
	req = b"POST / HTTP/1.1\r\nContent-Length: 10\r\n\r\n1234567890"
	s.sendall(req)
	sleep(0.1)
	res = s.recv(4096)
	if "501" in res.decode('utf-8'):
		print("PASSED!")
	else:
		print("Received response:")
		print(res.decode('utf-8'))
		print("FAILED!")

def test_http_version():
	s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
	s.connect(('127.0.0.1', 8080))
	req = b"GET / HTTP/2.0\r\n\r\n"
	s.sendall(req)
	sleep(0.1)
	res = s.recv(4096)
	if "505" in res.decode('utf-8'):
		print("PASSED!")
	else:
		print("Received response:")
		print(res.decode('utf-8'))
		print("FAILED!")

def test_redirect():
	s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
	s.connect(('127.0.0.1', 8080))
	req = b"GET /old HTTP/1.1\r\n\r\n"
	s.sendall(req)
	sleep(0.1)
	res = s.recv(4096)
	if "301" in res.decode('utf-8'):
		print("PASSED!")
	else:
		print("Received response:")
		print(res.decode('utf-8'))
		print("FAILED!")

def test_not_found():
	s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
	s.connect(('127.0.0.1', 8080))
	req = b"GET /naber HTTP/1.1\r\n\r\n"
	s.sendall(req)
	sleep(0.1)
	res = s.recv(4096)
	if "404" in res.decode('utf-8'):
		print("PASSED!")
	else:
		print("Received response:")
		print(res.decode('utf-8'))
		print("FAILED!")

def test_forbidden():
	s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
	s.connect(('127.0.0.1', 8080))
	req = b"GET /naber/naber HTTP/1.1\r\n\r\n"
	s.sendall(req)
	sleep(0.1)
	res = s.recv(4096)
	if "403" in res.decode('utf-8'):
		print("PASSED!")
	else:
		print("Received response:")
		print(res.decode('utf-8'))
		print("FAILED!")

def test_method_not_allowed():
	s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
	s.connect(('127.0.0.1', 8080))
	req = b"OPTIONS / HTTP/1.1\r\n\r\n"
	s.sendall(req)
	sleep(0.1)
	res = s.recv(4096)
	if "403" in res.decode('utf-8'):
		print("PASSED!")
	else:
		print("Received response:")
		print(res.decode('utf-8'))
		print("FAILED!")

if __name__ == "__main__":
	test_bad_request()
	test_not_implemented()
	test_http_version()
	test_redirect()
	test_not_found()
	test_forbidden()
	test_method_not_allowed()
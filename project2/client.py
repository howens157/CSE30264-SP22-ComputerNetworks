#!/usr/bin/env python3

import socket
import json

f = open('server_info.json')

info = json.load(f)

HOST = info[0]['servername']
PORT = info[0]['port']

f.close()

print(HOST, PORT)

# read json from file
f = open('data.json')
data = json.load(f)
	
with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
	s.connect((HOST, PORT))
	data0 = data[0]
	s.sendall(bytes(str(data0), encoding="utf-8"))
	print("sent data")


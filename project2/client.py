#!/usr/bin/env python3

import socket
import json
import sys

def main():
	f = open('server_info.json')

	info = json.load(f)

	HOST = info[0]['servername']
	PORT = info[0]['port']

	f.close()

	print(HOST, PORT)

	# read command line arguments
	argc = len(sys.argv)
	argv = sys.argv
	
	# initialize fields
	data = ""
	time = ""
	duration = 0
	name = ""
	description = ""
	location = ""
	identifier = 0
	
	if argv[2] == "add":
		print("add")
		arg_num = 3
		#while (arg_num):
			

	# read json from file
	f = open('data.json')
	data = json.load(f)

	
	with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
		s.connect((HOST, PORT))
		for d in data:
			data_to_send = {"calendarName": "testCalendarName", "action": "add", "arguments": d}
			s.sendall(bytes(str(data_to_send), encoding="utf-8"))
			print("sent data")


if __name__ == '__main__':
	main()

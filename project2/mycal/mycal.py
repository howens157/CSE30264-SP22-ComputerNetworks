#!/usr/bin/env python3

import socket
import json
import sys
import struct

class Fields:
	def __init__(self):
		self.date = ""
		self.time = ""
		self.duration = 0
		self.name = ""
		self.description = ""
		self.location = ""
		self.identifier = 0
		self.description = ""

	def update_fields(self, argv, argc, start):
		arg_num = start
		while (arg_num < argc):
			if argv[arg_num] == "date":
				self.date = argv[arg_num+1]
			elif argv[arg_num] == "time":
				self.time = argv[arg_num+1]
			elif argv[arg_num] == "duration":
				self.duration = argv[arg_num+1]
			elif argv[arg_num] == "name":
				self.name = argv[arg_num+1]
			elif argv[arg_num] == "description":
				self.description = argv[arg_num+1]
			elif argv[arg_num] == "location":
				self.location = argv[arg_num+1]
			arg_num+=2
		self.identifier+=1

def main():

	if len(sys.argv) < 4 or sys.argv[1] == '-h':
		print('Usage: ./mycal CalendarName Command Args')
		print('\t./mycal CalendarName add field value ... field value ... field value')
		print('\t./mycal CalendarName remove identifier')
		print('\t./mycal CalendarName update identifier field value')
		print('\t./mycal CalendarName get date')
		print('\t./mycal CalendarName getrange startDate stopDate')
		print('\t./mycal CalendarName input filename')
		exit(1)


	f = open('mycal/.mycal')

	info = json.load(f)

	HOST = info['servername']
	PORT = info['port']

	f.close()

	print(HOST, PORT)

	# read command line arguments
	argc = len(sys.argv)
	argv = sys.argv

	fields = Fields()

	calendar_name = argv[1]

	start_date = ""
	end_date = ""

	if argv[2] == "add":
		print("add")
		fields.update_fields(argv, argc, 3)
	elif argv[2] == "remove":
		print("remove")
		fields.identifier = int(argv[3])
	elif argv[2] == "update":
		fields.identifier = int(argv[3])
		fields.update_fields(argv, argc, 4)
	elif argv[2] == "get":
		print("get")
		fields.date = argv[3]
	elif argv[2] == "getrange":
		print("getrange")
		start_date = argv[3]
		end_date = argv[4]
		print("start:", start_date, "end:", end_date)
	elif argv[2] == "input":
		file_name = argv[3]
		print("input: file name:", file_name)

	print(fields.date)
	json_to_send = {"calendarName": calendar_name, "action": "add", "arguments": {"date": fields.date, "time": fields.time, "duration": fields.duration, "name": fields.name, "identifier": fields.identifier, "description": fields.description, "location": fields.location}} 
	print(json_to_send)

	# read json from file
	f = open('data.json')
	data = json.load(f)

	
	with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
		s.connect((HOST, PORT))
		cmdMsg = str(json_to_send)
		cmdLen = len(cmdMsg)
		s.sendall(struct.pack('!H', cmdLen))
		s.sendall(bytes(str(cmdMsg), encoding ="utf-8"))	
		#for d in data:
		#	data_to_send = {"calendarName": "testCalendarName", "action": "add", "arguments": d}
		#	s.sendall(bytes(str(data_to_send), encoding="utf-8"))
		#	print("sent data")


if __name__ == '__main__':
	main()
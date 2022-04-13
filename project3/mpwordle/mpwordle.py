#!/usr/bin/env python3

#  mycal.py 
#  3/25/2022
#  Hayden Owens, Lauren Korbel, Riley Griffith
#  CSE30264 - Computer Networks
#
#  This code implements a calendar client that connects to a server and issues 
#  commands to modify or retrieve calendar data
#
#  Usage:
#       ./mycal.py calName command args

import socket
import json
import sys
import struct
from datetime import datetime

def handleCommand(cmdJSON, HOST, PORT):
	# convert the command json to a string to send to the server
	cmdStr = json.dumps(cmdJSON)
	# create socket
	with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
		# connect to server
		s.connect((HOST, PORT))
		# send the length of the command string first then send the command string
		cmdLen = len(cmdStr)
		s.sendall(struct.pack('!H', cmdLen))
		s.sendall(bytes(cmdStr, encoding ="utf-8"))
		# receive the length of the return message first 
		retLen = s.recv(2)
		retLen = struct.unpack('!H', retLen)[0]
		# receive the return message and decode it and load it as json
		retJSONstr = s.recv(retLen)
		retJSONstr = retJSONstr.decode()
		retJSON = json.loads(retJSONstr)

		print(retJSON)

def error():
	print('Usage: ./mpwordle -name X -server X -port X')
	exit(1)

def main():
	# Parse command line arguments
	argc = len(sys.argv)
	argv = sys.argv
	if argc < 7 or argv[1] == '-h':
		error()
	
	playerName, serverIP, portNo = None, None, None
	for i in range(1, 7, 2):
		if argv[i] == "-name":
			playerName = argv[i+1]
		elif argv[i] == "-server":
			serverIP = argv[i+1]
		elif argv[i] == "-port":
			portNo = argv[i+1]
		else:
			error()

	# read in server data from .mycal JSON
	f = open('mpwordle/.mpwordle')
	info = json.load(f)
	HOST = info['servername']
	PORT = info['port']
	f.close()

	# start creating the JSON to join server
	cmdJSON = {}
	cmdJSON["MessageType"] = "Join"
	cmdJSON["Data"] = {"Name":playerName, "Client":"Python-1.0"}

	handleCommand(cmdJSON, HOST, PORT)

if __name__ == '__main__':
	main()

#!/usr/bin/env python3

import socket
import json
import sys
import struct
from datetime import datetime

def handleCommand(cmdJSON, HOST, PORT):
	cmdStr = json.dumps(cmdJSON)
	with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
		s.connect((HOST, PORT))
		cmdLen = len(cmdStr)
		s.sendall(struct.pack('!H', cmdLen))
		s.sendall(bytes(cmdStr, encoding ="utf-8"))
		retLen = s.recv(2)
		retLen = struct.unpack('!H', retLen)[0]
		print(f'Received message length of {retLen} from server')
		retJSONstr = s.recv(retLen)
		retJSONstr = retJSONstr.decode()
		# retJSONstr = ""
		# lenReceived = 0
		# while lenReceived < retLen:
		# 	currStr = s.recv(retLen)
		# 	lenReceived += len(str(currStr))
		# 	retJSONstr += str(currStr)
		retJSON = json.loads(retJSONstr)
		print(retJSON)
		if retJSON["success"] == True:
			print(f'Succesfully executed {retJSON["command"]} on {retJSON["calendar"]}')
			if retJSON["command"] == "add":
				print(f'Added event with identifier: {retJSON["identifier"]}')
			elif retJSON["command"] == "remove":
				print(f'Removed event with identifier: {retJSON["identifier"]}')
			elif retJSON["command"] == "update":
				print(f'Updated event with identifier: {retJSON["identifier"]}')
			elif retJSON["command"] == "get":
				print("All events on given day:")
				for event in retJSON["data"]:
					event = json.loads(event)
					print(f'Event ID: {event["id"]}     Date: {event["date"]}     Time: {event["time"]}     Duration(minutes): {event["duration"]}     Name: {event["name"]}', end="")
					if event["description"] != "":
						print(f'     Description: {event["description"]}', end="")
					if event["location"] != "":
						print(f'     Location: {event["location"]}', end="")
					print("")
			elif retJSON["command"] == "getrange":
				output_d = {}
				for event in retJSON["data"]:
					eventJSON = json.loads(event)
					date = datetime.strptime(eventJSON["date"], '%m%d%y')
					if date not in output_d:
						output_d[date] = [eventJSON]
					else:
						output_d[date].append(eventJSON)
				print(output_d)
				for key in sorted(output_d.keys()):
					print("Date: " + key.strftime("%B %d, %Y"))
					for event in output_d[key]:
						print(f'Event ID: {event["id"]}     Time: {event["time"]}     Duration(minutes): {event["duration"]}     Name: {event["name"]}', end="")
						if event["description"] != "":
							print(f'     Description: {eventJSON["description"]}', end="")
						if event["location"] != "":
							print(f'     Location: {eventJSON["location"]}', end="")
						print("")
					print("\n")

			else:
				print(f'Received unknown command from server: {retJSON["command"]}')
		elif retJSON["success"] == False:
			print(f'Failed to execute {retJSON["command"]} on {retJSON["calendar"]}')
			print(f'Error: {retJSON["error"]}')
		else:
			print("Error receiving command.")
			print("Received:")
			print(retJSONstr)



def main():
	argc = len(sys.argv)
	argv = sys.argv

	if argc < 4 or argv[1] == '-h':
		print('Usage: ./mycal CalendarName Command Args')
		print('\t./mycal CalendarName add field value ... field value ... field value')
		print('\t./mycal CalendarName remove identifier')
		print('\t./mycal CalendarName update identifier field value')
		print('\t./mycal CalendarName get date')
		print('\t./mycal CalendarName getrange startDate stopDate')
		print('\t./mycal CalendarName input filename')
		exit(1)

	validFields = ['date', 'time', 'duration', 'name', 'description', 'location']
	reqFields = ['date', 'time', 'duration', 'name']
	inputFilename = False

	f = open('mycal/.mycal')
	info = json.load(f)
	HOST = info['servername']
	PORT = info['port']
	f.close()

	cmdJSON = {}
	calendar_name = argv[1]
	cmdJSON["calendarName"] = calendar_name
	action = argv[2]
	cmdJSON["action"] = action
	cmdJSON["arguments"] = {}

	if action == 'add':
		for field in validFields:
			cmdJSON['arguments'][field] = ""
		providedFields = []
		for i in range(3, len(argv), 2):
			if argv[i] not in validFields:
				print(f'Invalid Field: {argv[i]}')
				print('Please only include these fields:')
				print(validFields)
				exit(1)
			else:
				providedFields.append(argv[i])
				cmdJSON['arguments'][argv[i]] = argv[i+1]
		for field in reqFields:
			if field not in providedFields:
				print('Not all required fields provided')
				print('Please provide these fields:')
				print(reqFields)
				exit(1)
	elif action == 'remove':
		cmdJSON['arguments']['identifier'] = argv[3]
	elif action == 'update':
		cmdJSON['arguments']['identifier'] = argv[3]
		if argv[4] not in validFields:
			print(f'Invalid Field: {argv[4]}')
			print('Please only include these fields:')
			print(validFields)
			exit(1)
		else:
			cmdJSON['arguments']['field'] = argv[4]
			cmdJSON['arguments']['value'] = argv[5]
	elif action == 'get':
		cmdJSON['arguments']['date'] = argv[3]
	elif action == 'getrange':
		cmdJSON['arguments']['start'] = argv[3]
		cmdJSON['arguments']['end'] = argv[4]
	elif action == 'input':
		inputFilename = argv[3]
	else:
		print('Unknown command')
		print('Usage: ./mycal CalendarName Command Args')
		print('\t./mycal CalendarName add field value ... field value ... field value')
		print('\t./mycal CalendarName remove identifier')
		print('\t./mycal CalendarName update identifier field value')
		print('\t./mycal CalendarName get date')
		print('\t./mycal CalendarName getrange startDate stopDate')
		print('\t./mycal CalendarName input filename')
		exit(1)

	if inputFilename is False:
		handleCommand(cmdJSON, HOST, PORT)
	else:
		f = open(inputFilename)
		commands = json.load(f)
		f.close()
		for command in commands:
			currCmd = command
			currCmd['calendarName'] = calendar_name
			handleCommand(currCmd, HOST, PORT)

if __name__ == '__main__':
	main()

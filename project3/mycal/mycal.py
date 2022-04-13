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
		# check if the command succesfully executed, if so, output the relevant returned information
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
			# if somehow an invalid command was received back from the server
			else:
				print(f'Received unknown command from server: {retJSON["command"]}')
		# otherwise, if the command failed, output an error message and the error provided by the server
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

	# If the user has not entered enough args, or indicated -h, output the possible commands
	if argc < 4 or argv[1] == '-h':
		print('Usage: ./mycal CalendarName Command Args')
		print('\t./mycal CalendarName add field value ... field value ... field value')
		print('\t./mycal CalendarName remove identifier')
		print('\t./mycal CalendarName update identifier field value')
		print('\t./mycal CalendarName get date')
		print('\t./mycal CalendarName getrange startDate stopDate')
		print('\t./mycal CalendarName input filename')
		exit(1)

	# Lists to check whether the user has only entered valid fields and whether they have entered all required fields
	validFields = ['date', 'time', 'duration', 'name', 'description', 'location']
	reqFields = ['date', 'time', 'duration', 'name']

	# track if the user used the input command and if so, what the file name is
	inputFilename = False

	# read in server data from .mycal JSON
	f = open('mycal/.mycal')
	info = json.load(f)
	HOST = info['servername']
	PORT = info['port']
	f.close()

	# start creating the JSON for the command using python lists and dictionaries
	cmdJSON = {}
	calendar_name = argv[1]
	cmdJSON["calendarName"] = calendar_name
	action = argv[2]
	cmdJSON["action"] = action
	cmdJSON["arguments"] = {}

	# handle the rest of the args according to the action the user has entered
	if action == 'add':
		#initialize all valid fields
		for field in validFields:
			cmdJSON['arguments'][field] = ""
		#track what fields the user has provided
		providedFields = []
		#read in the values associated with each field
		for i in range(3, len(argv), 2):
			if argv[i] not in validFields:
				print(f'Invalid Field: {argv[i]}')
				print('Please only include these fields:')
				print(validFields)
				exit(1)
			else:
				providedFields.append(argv[i])
				cmdJSON['arguments'][argv[i]] = argv[i+1]
		# check that all required fields have been provided
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
		#check that the provided field is a valid field
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
		# override inputFilename (previously False)
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

	# if inputFilename was never overridden, just handle the single command
	if inputFilename is False:
		handleCommand(cmdJSON, HOST, PORT)
	# otherwise, read in the JSON file provided by the user, and handle each command
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

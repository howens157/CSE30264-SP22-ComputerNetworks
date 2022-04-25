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
import time

def error():
	print('Usage: ./mpwordle -name X -server X -port X')
	exit(1)

def main():
	playerStats = {}

	# Parse command line arguments
	argc = len(sys.argv)
	argv = sys.argv
	if argc < 7 or argv[1] == '-h':
		error()
	
	playerName, HOST, PORT = None, None, None
	for i in range(1, 7, 2):
		if argv[i] == "-name":
			playerName = argv[i+1]
		elif argv[i] == "-server":
			HOST = argv[i+1]
		elif argv[i] == "-port":
			PORT = int(argv[i+1])
		else:
			error()

	# create socket, connect to server
	s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
	s.connect((HOST, 41100))

	# Send initial Join JSON
	cmdJSON = {}
	cmdJSON["MessageType"] = "Join"
	cmdJSON["Data"] = {"Name":playerName, "Client":"OwensKorbelGriffith-Python"}
	cmdStr = json.dumps(cmdJSON)
	print(f'sending join to server: {cmdStr}\n')
	s.sendall(bytes(cmdStr, encoding ="utf-8"))

	# Receive JoinResult from server
	retJSONstr = s.recv(1024).decode()
	print(f'received join response: {retJSONstr}\n')
	retJSON = json.loads(retJSONstr)


	# Receive StartInstance from server, get new port
	retJSONstr = s.recv(1024).decode()
	print(f'received StartInstance: {retJSONstr}')
	retJSON = json.loads(retJSONstr)
	gamePORT = int(retJSON["Data"]["Port"])
	nonce = retJSON["Data"]["Nonce"]
	
	time.sleep(3)

	# Close lobby connection, open game connection
	# s.close()
	s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
	s.connect((HOST, gamePORT))

	# Send JoinInstance to game server
	cmdJSON = {}
	cmdJSON["MessageType"] = "JoinInstance"
	cmdJSON["Data"] = {"Name":playerName, "Nonce":nonce}
	cmdStr = json.dumps(cmdJSON)
	print(f'sending JoinInstance to server: {cmdStr}\n')
	s.sendall(cmdStr.encode())

	# Receive JoinInstanceResult from server
	retJSONstr = s.recv(1024).decode()
	print(f'received JoinInstanceResult: {retJSONstr}\n')
	retJSON = json.loads(retJSONstr)
	myNum = int(retJSON["Data"]["Number"])
	result = (retJSON["Data"]["Result"])
	if result == 'No':
		print('Bad nonce received, exiting')
		exit(1)

	# Receive StartGame from server, handle chat
	while True:
		retJSONstr = s.recv(1024).decode()
		print(f'received message from server: {retJSONstr}\n')
		retJSON = json.loads(retJSONstr)
		if retJSON["MessageType"] == "Chat":
			print(f'chat from {retJSON["Data"]["Name"]}: {retJSON["Data"]["Text"]}\n')
		else:
			gameRounds = int(retJSON["Data"]["Rounds"])
			break
	players = retJSON["Data"]["PlayerInfo"]
	for player in players:
		playerStats[player['Name']] = [] 

	for rnd in range(gameRounds):
		for player in playerStats:
			playerStats[player].append('TEMPRESULT')

		# Wait to receive StartRound, handle chat
		while True:
			retJSONstr = s.recv(1024).decode()
			print(f'received message from server: {retJSONstr}\n')
			retJSON = json.loads(retJSONstr)
			if retJSON["MessageType"] == "Chat":
				print(f'chat from {retJSON["Data"]["Name"]}: {retJSON["Data"]["Text"]}\n')
			else:
				wordLen = retJSON["Data"]["WordLength"]
				print("Current word is " + str(wordLen) + " letters long")
				retJSON = json.loads(retJSONstr)
				roundsRemaining = int(retJSON["Data"]["RoundsRemaining"])
				break

		# Receive server PromptForGuess, handle chat
		while True:
			retJSONstr = s.recv(1024).decode()
			print(f'received message from server: {retJSONstr}\n')
			retJSON = json.loads(retJSONstr)
			if retJSON["MessageType"] == "Chat":
				print(f'chat from {retJSON["Data"]["Name"]}: {retJSON["Data"]["Text"]}\n')
			else:
				break

		# Loop until 
		while True:
			# Get user input for guess
			while True:
				print("Start your message with a $ to chat")
				guess = input("Input your guess or chat: ")
				if guess[0] == '$':
					cmdJSON = {}
					cmdJSON["MessageType"] = "Chat"
					cmdJSON["Data"] = {"Name":playerName, "Text":str(guess[1:])}
					cmdStr = json.dumps(cmdJSON)
					print(f'sending Guess to server: {cmdStr}\n')
					s.sendall(cmdStr.encode())
				else:
					if len(guess) == wordLen:
						break
					else:
						print("Input is incorrect length, word is " + str(wordLen) + " characters")
					

			cmdJSON = {}
			cmdJSON["MessageType"] = "Guess"
			cmdJSON["Data"] = {"Name":playerName, "Guess":str(guess)}
			cmdStr = json.dumps(cmdJSON)
			print(f'sending Guess to server: {cmdStr}\n')
			s.sendall(cmdStr.encode())

			# Get GuessResponse from server, handle chat
			while True:
				retJSONstr = s.recv(1024).decode()
				print(f'received message from server: {retJSONstr}\n')
				retJSON = json.loads(retJSONstr)
				if retJSON["MessageType"] == "Chat":
					print(f'chat from {retJSON["Data"]["Name"]}: {retJSON["Data"]["Text"]}\n')
				else:
					break
			
			# Get GuessResult from server, handle chat
			retJSONstr = s.recv(1024).decode()
			print(f'received GuessResult: {retJSONstr}\n')	
			retJSON = json.loads(retJSONstr)
			if retJSON["MessageType"] == "Chat":
				print(f'chat from {retJSON["Data"]["Name"]}: {retJSON["Data"]["Text"]}\n')
			else:
				winner = retJSON['Data']['Winner']
				#result = retJSON['Data']['PlayerInfo'][0]['Result']
				#print(result)
				for result in retJSON['Data']['PlayerInfo']:
					playerStats[result['Name']][rnd] = result['Result']
					print("result: ", result['Result'])
					i = 0
					print(result['Name'], ":", end='')
					for letter in result['Result']:
						if letter == 'G':
							if result['Name'] == playerName:
								print("\033[92m" + guess[i] + "\033[0m", end='')
							else:
								print("\033[92m" + "X" + "\033[0m", end='')
						elif letter == 'Y':
							if result['Name'] == playerName: 
								print("\033[93m" + guess[i] + "\033[0m", end='')
							else:
								print("\033[93m" + "X" + "\033[0m", end='')
						elif letter == 'B':
							if result['Name'] == playerName:
								print(guess[i], end='')
							else:
								print("X", end='')
						i = i + 1
					print()
			
			# Check if anyone won or if that was the last round
			if winner == 'Yes':
				print("Someone guessed right! Round is over!")
			
			receivedEndRound = False
			#receive endRound or next promptGuess, handle chat
			while True:
				retJSONstr = s.recv(1024).decode()
				retJSON = json.loads(retJSONstr)
				if retJSON['MessageType'] == 'EndRound':
					print(f'received EndRound: {retJSONstr}\n')	
					receivedEndRound = True
					break
				elif retJSON["MessageType"] == "Chat":
					print(f'chat from {retJSON["Data"]["Name"]}: {retJSON["Data"]["Text"]}\n')
				else:
					print(f'received PromptForGuess: {retJSONstr}\n')
					break
			if receivedEndRound:
				break

	#receive endGame, handle chat
	while True:
		retJSONstr = s.recv(1024).decode()
		print(f'received message from server: {retJSONstr}\n')
		retJSON = json.loads(retJSONstr)
		if retJSON["MessageType"] == "Chat":
			print(f'chat from {retJSON["Data"]["Name"]}: {retJSON["Data"]["Text"]}\n')
		else:
			print("Game is over")
			break

	print(playerStats)
	
if __name__ == '__main__':
	main()

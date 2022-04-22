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

def error():
	print('Usage: ./mpwordle -name X -server X -port X')
	exit(1)

def main():
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
	# s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
	# s.connect((HOST, PORT))

	# # Send initial Join JSON
	# cmdJSON = {}
	# cmdJSON["MessageType"] = "Join"
	# cmdJSON["Data"] = {"Name":playerName, "Client":"Python-1.0"}
	# cmdStr = json.dumps(cmdJSON)
	# print(f'sending join to server: {cmdStr}\n')
	# s.sendall(bytes(cmdStr, encoding ="utf-8"))

	# # Receive JoinResult from server
	# retJSONstr = s.recv(1024).decode()
	# print(f'received join response: {retJSONstr}\n')
	# retJSON = json.loads(retJSONstr)

	# # Receive StartInstance from server, get new port
	# retJSONstr = s.recv(1024).decode()
	# print(f'received StartInstance: {retJSONstr}')
	# retJSON = json.loads(retJSONstr)
	# gamePORT = int(retJSON["Data"]["Port"])
	# nonce = retJSON["Data"]["Nonce"]
	nonce = 7
	
	# Close lobby connection, open game connection
	# s.close()
	s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
	s.connect((HOST, 41101))

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

	# Receive StartGame from server
	retJSONstr = s.recv(1024).decode()
	print(f'received StartGame: {retJSONstr}\n')
	retJSON = json.loads(retJSONstr)
	gameRounds = int(retJSON["Data"]["Rounds"])

	for rnd in range(gameRounds):
		# Wait to receive StartRound
		retJSONstr = s.recv(1024).decode()
		print(f'received StartRound: {retJSONstr}\n')
		retJSON = json.loads(retJSONstr)
		wordLen = retJSON["Data"]["WordLength"]
		print("Current word is " + str(wordLen) + " letters long")
		retJSON = json.loads(retJSONstr)
		roundsRemaining = int(retJSON["Data"]["RoundsRemaining"])

		# Receive server PromptForGuess
		retJSONstr = s.recv(1024).decode()
		print(f'received PromptForGuess: {retJSONstr}\n')

		# Loop until RoundsRemaining is 0
		while True:
			# Get user input for guess
			while True:
				guess = input("Input your guess: ")
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

			# Get GuessResponse from server
			retJSONstr = s.recv(1024).decode()
			print(f'received GuessResponse: {retJSONstr}\n')
			
			# Get GuessResult from server
			retJSONstr = s.recv(1024).decode()
			print(f'received GuessResult: {retJSONstr}\n')	
			retJSON = json.loads(retJSONstr)
			winner = retJSON['Data']['Winner']

			# Check if anyone won or if that was the last round
			
			if winner == 'Yes':
				print("Someone guessed right! Round is over!")
				break

		#receive endRound		
		retJSONstr = s.recv(1024).decode()
		print(f'received EndRound: {retJSONstr}\n')	

	#receive endGame
	retJSONstr = s.recv(1024).decode()
	print(f'received EndGame: {retJSONstr}\n')	
	
if __name__ == '__main__':
	main()

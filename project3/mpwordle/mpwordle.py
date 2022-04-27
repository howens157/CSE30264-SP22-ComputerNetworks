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
import re

def error():
	print('Usage: ./mpwordle -name X -server X -port X')
	exit(1)

def main():
	playerStats = {}

	player_colors = {}
	color = 92

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
	s.connect((HOST, PORT))

	# Send initial Join JSON
	cmdJSON = {}
	cmdJSON["MessageType"] = "Join"
	cmdJSON["Data"] = {"Name":playerName, "Client":"OwensKorbelGriffith-Python"}
	cmdStr = json.dumps(cmdJSON)
	print('Joining Lobby...')
	s.sendall(bytes(cmdStr, encoding ="utf-8"))

	# Receive JoinResult from server
	retJSONstr = s.recv(1024).decode()
	print('Lobby joined...')

	print('Waiting for other players to join the lobby, enter a chat or just enter to proceed.')

	# Allow the user to send chat messages after submitting their guess
	while True:
		msg = input("Start your message with a $ to chat: ")
		if msg == "":
			cmdJSON = {}
			cmdJSON["MessageType"] = "Chat"
			cmdJSON["Data"] = {"Name":"mpwordle", "Text":"NoChat"}
			cmdStr = json.dumps(cmdJSON)
			s.sendall(cmdStr.encode())
			break
		if msg[0] == '$':
			cmdJSON = {}
			cmdJSON["MessageType"] = "Chat"
			cmdJSON["Data"] = {"Name":playerName, "Text":str(msg[1:])}
			cmdStr = json.dumps(cmdJSON)
			s.sendall(cmdStr.encode())


	# Receive StartInstance from server, get new port, handle chat
	receivedStart = False
	while True:
		if receivedStart:
			break
		retJSONstr = s.recv(1024).decode()
		print(retJSONstr)
		try:
			retJSON = json.loads(retJSONstr)
			if retJSON["MessageType"] == "Chat":
				player = retJSON["Data"]["Name"]
				if player not in player_colors:
					player_colors[player] = "\033[" + str(color) + "m"
					color = color + 1
				print(f'{player_colors[player]} chat from {retJSON["Data"]["Name"]}: {retJSON["Data"]["Text"]} \033[0m')
			else:
				break
		except:
			# Handle the case in which multiple messages are sent
			chatList = re.split('({[^}]*})', retJSONstr)
			for jsonStr in chatList:
				if not len(jsonStr) > 1:
					continue
				retJSON = json.loads(jsonStr + "}")
				if retJSON["MessageType"] == "Chat":
					player = retJSON["Data"]["Name"]
					if player not in player_colors:
						player_colors[player] = "\033[" + str(color) + "m"
						color = color + 1
					print(f'{player_colors[player]} chat from {retJSON["Data"]["Name"]}: {retJSON["Data"]["Text"]} \033[0m')
				else:
					receivedStart = True
	retJSON = json.loads(retJSONstr)
	gamePORT = int(retJSON["Data"]["Port"])
	server = retJSON["Data"]["Server"]
	nonce = retJSON["Data"]["Nonce"]
	print(f'Joining game at port {gamePORT} on {server}...')
	
	time.sleep(2)

	# Close lobby connection, open game connection
	s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
	s.connect((HOST, gamePORT))

	# Send JoinInstance to game server
	cmdJSON = {}
	cmdJSON["MessageType"] = "JoinInstance"
	cmdJSON["Data"] = {"Name":playerName, "Nonce":nonce}
	cmdStr = json.dumps(cmdJSON)
	s.sendall(cmdStr.encode())

	# Receive JoinInstanceResult from server
	retJSONstr = s.recv(1024).decode()
	retJSON = json.loads(retJSONstr)
	myNum = int(retJSON["Data"]["Number"])
	result = (retJSON["Data"]["Result"])
	if result == 'No':
		print('Bad nonce received, exiting')
		exit(1)
	print('Game server joined...')

	# Receive StartGame from server
	
	retJSONstr = s.recv(1024).decode()
	retJSON = json.loads(retJSONstr)
	gameRounds = int(retJSON["Data"]["Rounds"])
	print('\nWelcome to Multiplayer Wordle!')
	print(f'There will be {gameRounds} rounds.')
			
	players = retJSON["Data"]["PlayerInfo"]
	for player in players:
		playerStats[player['Name']] = [] 

	for rnd in range(gameRounds):
		print(f'ROUND {rnd+1}:\n----------')

		for player in playerStats:
			playerStats[player].append('TEMPRESULT')

		# Wait to receive StartRound
		retJSONstr = s.recv(1024).decode()
		retJSON = json.loads(retJSONstr)
		wordLen = retJSON["Data"]["WordLength"]
		print("Current word is " + str(wordLen) + " letters long")
		retJSON = json.loads(retJSONstr)
		roundsRemaining = int(retJSON["Data"]["RoundsRemaining"])

		# Receive server PromptForGuess, handle chat
		retJSONstr = s.recv(1024).decode()
		retJSON = json.loads(retJSONstr)

		# Loop until someone gets it right or the server sends an EndRound
		while True:
			# Get user input for guess or allow them to chat before inputting their guess
			while True:
				print("Start your message with a $ to chat")
				guess = input("Input your guess or chat: ")
				if guess[0] == '$':
					cmdJSON = {}
					cmdJSON["MessageType"] = "Chat"
					cmdJSON["Data"] = {"Name":playerName, "Text":str(guess[1:])}
					cmdStr = json.dumps(cmdJSON)
					s.sendall(cmdStr.encode())
				else:
					if len(guess) == wordLen and guess.isalpha():
						break
					else:
						print("Input is incorrect length, word is " + str(wordLen) + " characters")
					
			cmdJSON = {}
			cmdJSON["MessageType"] = "Guess"
			cmdJSON["Data"] = {"Name":playerName, "Guess":str(guess).upper()}
			cmdStr = json.dumps(cmdJSON)
			s.sendall(cmdStr.encode())

			# Get GuessResponse from server, handle chat
			receivedResponse = False
			while True:
				if receivedResponse:
					break
				retJSONstr = s.recv(1024).decode()
				print(retJSONstr)
				try:
					retJSON = json.loads(retJSONstr)
					if retJSON["MessageType"] == "Chat":
						player = retJSON["Data"]["Name"]
						if player not in player_colors:
							player_colors[player] = "\033[" + str(color) + "m"
							color = color + 1
						print(f'{player_colors[player]} chat from {retJSON["Data"]["Name"]}: {retJSON["Data"]["Text"]} \033[0m')
					else:
						break
				except:
					# Handle the case in which multiple messages are sent
					chatList = re.split('({[^}]*})', retJSONstr)
					for jsonStr in chatList:
						if not len(jsonStr) > 1:
							continue
						retJSON = json.loads(jsonStr + "}")
						if retJSON["MessageType"] == "Chat":
							player = retJSON["Data"]["Name"]
							if player not in player_colors:
								player_colors[player] = "\033[" + str(color) + "m"
								color = color + 1
							print(f'{player_colors[player]} chat from {retJSON["Data"]["Name"]}: {retJSON["Data"]["Text"]} \033[0m')
						else:
							receivedResponse = True

			print('Waiting for other players to submit their guess, enter a chat or just enter to proceed.')

			# Allow the user to send chat messages after submitting their guess
			while True:
				msg = input("Start your message with a $ to chat: ")
				if msg == "":
					cmdJSON = {}
					cmdJSON["MessageType"] = "Chat"
					cmdJSON["Data"] = {"Name":"mpwordle", "Text":"NoChat"}
					cmdStr = json.dumps(cmdJSON)
					s.sendall(cmdStr.encode())
					break
				if msg[0] == '$':
					cmdJSON = {}
					cmdJSON["MessageType"] = "Chat"
					cmdJSON["Data"] = {"Name":playerName, "Text":str(msg[1:])}
					cmdStr = json.dumps(cmdJSON)
					s.sendall(cmdStr.encode())
			
			# Get GuessResult from server, handle chat
			receivedResult = False
			while True:
				if receivedResult:
					break
				retJSONstr = s.recv(1024).decode()
				print(retJSONstr)
				try:
					retJSON = json.loads(retJSONstr)
					if retJSON["MessageType"] == "Chat":
						player = retJSON["Data"]["Name"]
						if player not in player_colors:
							player_colors[player] = "\033[" + str(color) + "m"
							color = color + 1
						print(f'{player_colors[player]} chat from {retJSON["Data"]["Name"]}: {retJSON["Data"]["Text"]} \033[0m')
					else:
						break
				except:
					# Handle the case in which multiple messages are sent
					chatList = re.split('({[^}]*})', retJSONstr)
					for jsonStr in chatList:
						if not len(jsonStr) > 1:
							continue
						retJSON = json.loads(jsonStr + "}")
						if retJSON["MessageType"] == "Chat":
							player = retJSON["Data"]["Name"]
							if player not in player_colors:
								player_colors[player] = "\033[" + str(color) + "m"
								color = color + 1
							print(f'{player_colors[player]} chat from {retJSON["Data"]["Name"]}: {retJSON["Data"]["Text"]} \033[0m')
						else:
							receivedResult = True
			retJSON = json.loads(retJSONstr)
			winner = retJSON['Data']['Winner']
			print("Guesses of this round:")
			for result in retJSON['Data']['PlayerInfo']:
				playerStats[result['Name']][rnd] = result['Result']
				i = 0
				print(f'\t{result["Name"]}: ', end='')
				for letter in result['Result']:
					if letter == 'G':
						if result['Name'] == playerName:
							print("\033[92m" + guess[i].upper() + "\033[0m", end='')
						else:
							print("\033[92m" + "X" + "\033[0m", end='')
					elif letter == 'Y':
						if result['Name'] == playerName: 
							print("\033[93m" + guess[i].upper() + "\033[0m", end='')
						else:
							print("\033[93m" + "X" + "\033[0m", end='')
					elif letter == 'B':
						if result['Name'] == playerName:
							print(guess[i].upper(), end='')
						else:
							print("X", end='')
					i = i + 1
				print()
			
			# Check if anyone won or if that was the last round
			if winner == 'Yes':
				print("Someone guessed right! Round is over!")
	
			#receive endRound or next promptGuess, handle chat
			retJSONstr = s.recv(1024).decode()
			retJSON = json.loads(retJSONstr)
			if retJSON['MessageType'] == 'EndRound':
				print(f'That\'s the end of round {rnd+1}!')
				print(f'Scoreboard after {rnd+1} rounds:')
				playerInfo = retJSON['Data']['PlayerInfo']
				for player in playerInfo:
					print(f'\t{player["Name"]}:\tScore: {player["ScoreEarned"]}', end='')
					if player['Winner'] == 'Yes':
						print('\tWINNER!', end='')
					print()
				break
			else:
				print('No one got it right! Guess again!')

	#receive endGame, handle chat
	retJSONstr = s.recv(1024).decode()
	retJSON = json.loads(retJSONstr)
	print("Game is over!")
	print(f'Final Scoreboard:')
	playerInfo = retJSON['Data']['PlayerInfo']
	for player in playerInfo:
		print(f'\t{player["Name"]}:\tScore: {player["ScoreEarned"]}')
	print(f'\n{retJSON["Data"]["WinnerName"]} is your winner! Congratulations!')

	print("\nGame Statistics:")
	for player, results in playerStats.items():
		lettersRight = 0
		totalLetters = 0
		for result in results:
			lettersRight += result.count('G')
			totalLetters += len(result)
		print(f'\t{player}: {lettersRight} out of {totalLetters} right. Avg Accuracy: {lettersRight/totalLetters*100}%')
		
	
if __name__ == '__main__':
	main()

# ComputerNetworksWork
Computer Networks SP22 Work: Hayden Owens, Lauren Korbel, Riley Griffith

## General Overview
The server was written in C and can support multiple multiplayer wordle games at once.

The client was written in python and connects to the server and allows users to play the wordle game.

## How to Invoke
The server and client must both be invoked from the project3 directory, not from within the mpwordleserver and mpwordle subdirectories,
respectively. 
<br><br>
		client:
<br>
		./mpwordle/mpwordle.py -name X -server X -port X
<br>
		where name is the player's name and server and port are where the client should connect to the server
<br><br>
		server:
<br>
		./mpwordleserver/mpwordleserver -np X -lp X -pp X -nr X -d DFile -dbg
<br>
		all arguments are optional, and np is the number of players (default is 2), lp is the lobby port, pp is the play port, 
		-d is the dictionary file, and -dbg is if the server should print debug messages
<br> For the dictionary, the path must be specified from the project3 directory

## Extensions
1. Guess results print in color for all players, where green is a correct letter in the correct spot and yellow is a correct letter in an incorrect spot
2. Scoring system where points are awarded based on how many letters are correct and how many are in the correct spot
3. Guesses may be entered in uppercase, lowercase, or some combination
4. Guesses are checked to ensure entered characters are only letters
5. Prints game statistics at the end of each game including the average accuracy for each player
6. Prints chats in a different color based on who sent the chat

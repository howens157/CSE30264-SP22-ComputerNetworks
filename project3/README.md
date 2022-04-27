# ComputerNetworksWork
Computer Networks SP22 Work: Hayden Owens, Lauren Korbel, Riley Griffith

## General Overview
The server was written in C and can support multiple multiplayer wordle games at once.

The client was written in python and connects to the server and allows users to play the wordle game.

The server and client do not support any of the optional command line arguments.

Our game supports all required elements of the rubric. A lobby is used to gather players and then fork is used to spawn a new game.
Multiple games can succcesfully be spawned at the same time, with multiple players per game. Players can chat both within the lobby and
within the game, before and after submitting a guess.

## Expected User Behavior

It is expected that no two users will use the same name. This will not cause any major errors, but will cause undesired behavior when color-
coding the chat messages and printing the results for each player in the game. During the course of the game, the user has 3 opportunities to 
send chat messages. First, while waiting in the lobby, the user can send one or several chat messages, but must hit enter without a message to
move on past the chat to receive messages that others have sent and start the actual game. Second, when prompted to input a guess for the current
word, the user can instead send one or several chat messages, but the user must input a valid guess before they can receive messages from others
and progress in the game. Third, after guessing and while waiting for other players to guess, the user can once again send one or several chat 
messages, but must hit enter without a message to move on past the chat to receive messages that others have sent.

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
2. Custom scoring system:
	-	scores are calculated only at the end of each round
	-	every player gets points for each letter they have in the correct spot at the end of the round
	-	the points per letter decreases with the number of guesses the player took, starting at 10 and halving with every guess
	-	if the user guesses the word completely, the points per letter for that user are doubled
3. Guesses may be entered in uppercase, lowercase, or some combination
4. Guesses are checked to ensure entered characters are only letters
5. Prints game statistics at the end of each game including the average accuracy for each player
6. Prints chats in a different color based on who sent the chat
7. The client prints the information received from the server in a more readable format, printing
a scoreboard at the end of each round and at the end of the game.

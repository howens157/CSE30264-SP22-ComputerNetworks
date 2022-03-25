# ComputerNetworksWork
Computer Networks SP22 Work: Hayden Owens, Lauren Korbel, Riley Griffith

## General Overview
The server was implemented using the json-glib library. It accepts a connection and then receives the length of a command string
from the client, then receives the actual string. It then uses the json-glib library to parse the string and handles it according
to the command that was sent. The calendar data is stored in multiple file within the data directory, with a separate file for
each calendar. The data is stored as a sort of pseudo-json. It is not an actual json object, but each line is a different string
that can be parsed as a json object representing one event in the calendar. The server handles the received command, and then
constructs a json object, converts it to a string, and sends it to the client.

The client was implemented using python. It is a simple client that parses the command line arguments, constructs a json
representation of the desired command, and sends it to the server. It then receives a json reply back and parses and 
outputs the relevant information based on the command that was sent.

## How to Invoke
The server and client must both be invoked from the project2 directory, not from within the mycalserver and mycal subdirectories,
respectively. 
      ./mycal/mycal.py args...
      ./mycalserver/mycalserver -mt(optional)
The mycalserver directory must have a data/ subdirectory or else the mycalserver will not function appropriately.
The client can also be invoked to read a list of commands from a JSON file and send them all to the server. The commands must
be formatted as a true JSON object that is an array of objects, with each object being a command to be sent to the server.
The command objects are formatted like so:

{
		"action": "add", 
		"arguments": {
			"date": "022322", 
			"time": "1100", 
			"duration": "60", 
			"name": "Quiz", 
			"description": "", 
			"location": ""
		}
}

There is an example file of commands in the project2 directory called test-inputs.json
Note that there is no member to indicate the calendar name. This is because the input command is executed as: 
        ./mycal/mycal.py CalendarName input inputFile
Thus, the calendar name is specified at the command line and is absent from the input file. Therefore, an input command cannot
act on multiple calendars at the same time.

3DFourConnect
Written in c++ and compiled with libraries for 64bit;

Requires VC_Redist 2015 or Later

Instructions: 
	The game is played very similarly to Tic-Tac-Toe, except for the 
fact that it is in 3 dimensions. The players alternate putting pieces 
on the board to get four in a row of the same color. You can get 4 in 
a row by lining up the pieces horizontally, vertically, or diagonally. 
This means that people can even get a four in a row by going diagonally 
through the center of the cube.

Menu Selections:
	There are 3 possible menu selections once you launch the program: 
"local", "server", or "client". Local refers to playing on one computer 
where the players physically take turns controlling the mouse and placing 
pieces. Server refers to the program that will host a multiplayer server 
for players to connect to. Once selected, it will request a port for the 
server to be opened on. You must set your router to port forward the port 
selected to the server host's computer. Client finally is what players 
select when they wish to connect to a server. They are then prompted for 
the IP Address of the network the server is being hosted on. Entering this 
will connect them to the server and once two players join, the game will start.

GamePlay:
	Each player is assigned a color when the game starts: Red or Blue. 
This is marked by the sphere in the top right corner of the screen. The 
scores of each player are marked by the corresponding color text in the 
top left of the screen. Once somebody wins, a message will appear and the 
players must each press enter to move on to the next round. The currently 
selected piece of the opposing player can be identified by the pulsing aura 
around a piece of your opponent's color.

Controls: 
	Use W, A, S, and D to rotate the camera view around the board. 
You can also place pieces by moving your mouse over the spot where the 
piece will be and pressing left click.

Note:
	If more than two people join the server, they will be rejected. 
In order to connect two new people, the server must be remade.

Installation:
1. Download the entire repository
2. If the libraries imported are out of date or not working:
	1. Check that the project properties is importing them correctly
	2. Rebuild the libs (Listed Below) (Recommend CMAKE Compiler: https://cmake.org/download/)

Libraries
OpenGL: https://www.opengl.org//
GLFW: https://www.glfw.org/download.html
GLAD: https://glad.dav1d.de/
Assimp: http://assimp.org/
FreeType: https://www.freetype.org/
GameNetworkingSockets: https://github.com/ValveSoftware/GameNetworkingSockets/blob/master/BUILDING.md

Credits:
Server Code Based On:
https://github.com/ValveSoftware/GameNetworkingSockets/blob/master/examples/example_chat.cpp

Educational Links:
https://learnopengl.com/Getting-started/Creating-a-window
https://partner.steamgames.com/doc/api/ISteamNetworkingSockets
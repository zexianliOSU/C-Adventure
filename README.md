# C-Adventure
## Compiling the files  
```
gcc -lpthread Adventure.c -o Adventure
gcc BuildRooms.c -o BuildRooms
```
### Generating rooms  
Before starting the game, it needs the files for the rooms. By running BuildRooms, it will generate a folder containing 7 files. Each file is a separate room with its own characteristics.
```
./BuildRooms
```
### Playing the game
Now that the rooms are generated, to run the game:
```
./Adventure
```
To play, simply type in the name of the room that you wish to travel to. The goal is to find the exit. When you find the exit, the game will display the number of turns and the path you took.
If you wish to play again with new rooms, **./BuildRooms** will generate a new folder of rooms. When you run **./Adventure** again, it will automatically pick up the newest folder of rooms.

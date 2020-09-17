#include <stdio.h>
#include <time.h>
#include <sys/stat.h>
#include <dirent.h>
#include <stdlib.h>

typedef enum {
    false, true
} bool;

struct room {
    int connections[6];
    int totalConnections;
    int roomIndex;
    const char* roomName;
    char* roomType;
};

/*
 *  Array of room names
 */
const char * roomNames[10] = {
    "Darkroom",
    "Dungeon",
    "Brown",
    "Tombs",
    "Bank",
    "House",
    "Vault",
    "Pits",
    "Spider",
    "Wizard"
};

const char * roomTypes[3] = {
    "START_ROOM",
    "MID_ROOM",
    "END_ROOM"
};

int IsGraphFull(struct room input[], int numOfRooms);
void ConnectRoom(struct room* x, struct room* y);
struct room* GetRandomRoom(struct room input[], int numOfRooms);
int CanAddConnectionFrom(struct room input);
int IsSameRoom(struct room x, struct room y);
int ConnectionAlreadyExists(struct room x, struct room y);
void AddRandomConnection(struct room input[], int numOfRooms);

int IsGraphFull(struct room input[], int numOfRooms) {
    /*printf("Ran IsGraphFull\n");*/
    int i;
    for (i = 0; i < numOfRooms; i++) {
        /*Return true if there are less than 3 connections in the current room*/
        if (input[i].totalConnections < 3) {
            return 0;
        }
    }
    return 1;
}

void ConnectRoom(struct room* x, struct room* y) {
    /*printf("Ran ConnectRoom\n");*/
    int i;
    for (i = 0; i < 6; i++) {
        /*If at index i the element is -1, then it is a unused connection spot*/
        if (x->connections[i] == -1) {
            x->connections[i] = y->roomIndex;
            x->totalConnections++;
            break;
        }
    }
}

struct room* GetRandomRoom(struct room input[], int numOfRooms) {
    /*printf("Ran GetRandomRoom\n");*/
    int randomNum = (rand() % numOfRooms);
    return &input[randomNum];
}

int CanAddConnectionFrom(struct room input) {
    /*printf("Ran CanAddConnectionFrom\n");*/
    if (input.totalConnections < 6) {
        return 1;
    } else {
        return 0;
    }
}

int IsSameRoom(struct room x, struct room y) {
    /*printf("Ran IsSameRoom\n");*/
    if (x.roomName == y.roomName) {
        return 1;
    } else {
        return 0;
    }
}

int ConnectionAlreadyExists(struct room x, struct room y) {
    /*printf("Ran ConnectionAlreadyExists\n");*/
    int i;
    for (i = 0; i < x.totalConnections; i++) {
        if (x.connections[i] == y.roomIndex) {
            return 1;
        }
    }
    return 0;
}

void AddRandomConnection(struct room input[], int numOfRooms)  {
    /*printf("Ran AddRandomConnection\n");*/
    struct room* A;
    struct room* B;

    while(true) {
        A = GetRandomRoom(input, numOfRooms);
        if (CanAddConnectionFrom(*A) == 1) {
            break;
        }
    }

    do {
        B = GetRandomRoom(input, numOfRooms);
    } while(CanAddConnectionFrom(*B) == 0 || IsSameRoom(*A, *B) == 1 || ConnectionAlreadyExists(*A, *B) == 1); /*need to check this statement*/
    /* 0 is false, 1 is true*/

    ConnectRoom(A, B);
    ConnectRoom(B, A);
}

int main() {
    /*
     * @brief Create directory with PID
     */
    int pid = getpid();
    /*printf("Printing pid: %d \n", pid);*/
    char directoryNamePrefix[] = "lizexi.rooms.";
    char directoryName[1024];
    snprintf(directoryName, 1024 + 1,"%s%d", directoryNamePrefix, pid);
    /*printf("Printing directory name: %s \n", directoryName);*/
    mkdir(directoryName, 0755); /* Remember to add argument 0755 before compiling in os1 */


    DIR *directory;
    directory = opendir(directoryName);
    /*if (directory == NULL) {
        printf("Can't open directory\n");
    } else {
        printf("Openened directory\n");
    }*/



    /*
     * Initialize random generator
     */
    srand(time(NULL));
    int i = 0;


    /*
     * @brief Generate a random number of rooms and use random names from the names array.
     */
    /*printf("-------------------------------------------\n");*/
    int numOfRooms = 7;
    /*printf("Printing number of rooms: %d \n", numOfRooms);*/

    int idxOfNames[numOfRooms];
    int counter = 0;
    for (i = 0; i < 10 && counter < numOfRooms; i++) {
        int rn = 10 - i;
        int rm = numOfRooms - counter;
        if (rand() % rn < rm) {
            idxOfNames[counter++] = i;
        }
    }

    /*for (i = 0; i < numOfRooms; i++) {
        printf("index i is: %d\t%d and the name to us is: %s\n", i, idxOfNames[i],roomNames[idxOfNames[counter++]]);
    }*/


    /*
     * @brief Declare rooms and their variables
     */
    struct room gameRoom[numOfRooms];
    counter = 0;
    for (i = 0; i < numOfRooms; i++) {
        gameRoom[i].roomName = roomNames[idxOfNames[counter++]]; /*Using names from random unique numbers from the names array*/
        gameRoom[i].roomIndex = i;
        gameRoom[i].totalConnections = 0;
        /*Set the array of connections for ease of work later*/
        /*Where a -1 is an unused spot for connection*/
        int j;
        for (j = 0; j < 6; j++) {
            gameRoom[i].connections[j] = -1;
        }
        /*Assigning room types based on index*/
        if (i == 0) {
            gameRoom[i].roomType = "START_ROOM";
        } else if (i == numOfRooms - 1) {
            gameRoom[i].roomType = "END_ROOM";
        } else {
            gameRoom[i].roomType = "MID_ROOM";
        }
        /*printf("Name of the room just used: %s and roomtype is: %s\n", gameRoom[i].roomName, gameRoom[i].roomType);*/
    }


    /*
     * @brief fill array of rooms with connections
     */
    while (IsGraphFull(gameRoom, numOfRooms) == 0) {
      AddRandomConnection(gameRoom, numOfRooms);
    }

    /*
     * @brief fill the room files with room name, connections, and room type.
     */
    char filePrefix[] = "_room";
    for (i = 0; i < numOfRooms; i++) {
        /*Get the name from the names array and add the "_room" at the end*/
        char fileName[1024];
        snprintf(fileName, 1024 + 1,"%s%s", gameRoom[i].roomName, filePrefix);
        /*printf("Printing fileName: %s\n", fileName);*/

        /*Get the folder path*/
        char tempPath[1024];
        snprintf(tempPath, 1024 + 1,"%s%s", directoryName, "/");

        /*printf("Printing tempPath: %s\n", tempPath);*/
        /*Get the complete path with the name of the file at the end, so if is not existant, fopen will create it*/
        char filePath[1024];
        snprintf(filePath, 1024 + 1,"%s%s", tempPath, fileName);

        /*Create file with the complete path and write the room details inside the file*/
        FILE *roomFile = fopen(filePath, "w+");
        fprintf(roomFile, "%s %s \n", "ROOM NAME:", gameRoom[i].roomName);  /*Write room name at the beginning*/
        int j;
        for (j = 0; j < gameRoom[i].totalConnections; j++) {
            fprintf(roomFile, "CONNECTION %d: %s\n", j + 1, gameRoom[gameRoom[i].connections[j]].roomName); /*gets the index of the room from connection array,
                                                                                                                                then retrieve name using that index*/
        }
        fprintf(roomFile, "%s %s \n", "ROOM TYPE:", gameRoom[i].roomType);  /*Write room type at the end*/
		fclose(roomFile);
    }

    /*printf("Printing gameRoom[0]: %s\n", gameRoom[0].roomName);
    for (i = 0; i < gameRoom[0].totalConnections; i++) {
        printf("CONNECTION %d: %d\n", i + 1, gameRoom[0].connections[i]);
        printf("%s \n", gameRoom[gameRoom[0].connections[i]].roomName);
    }*/
    closedir(directory);
    return 0;
}

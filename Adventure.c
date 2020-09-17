#include <stdio.h>
#include <time.h>
#include <sys/stat.h>
#include <dirent.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>


struct room {
    char connections[6][9];
    int totalConnections;
    char roomName[9];
    char roomType[11];
};

pthread_mutex_t mutex1 = PTHREAD_MUTEX_INITIALIZER;


void *getTime(void *param);
char *getCurPath();
struct room *getRooms(int numOfRooms);

/*  Mutex documentation from:
 *  https://github.com/angrave/SystemProgramming/wiki/Synchronization%2C-Part-1%3A-Mutex-Locks
 */
void *getTime(void *param) {
    /*Parse time into string*/
    char timeString[200];
    time_t rawTime;
    time(&rawTime);
    struct tm *tmp = localtime(&rawTime);
    strftime(timeString, 128, "%l:%M%P, %A, %B %d, %Y%n", tmp);

    /*Create file and write time in it*/
    FILE *timeFile;
    timeFile = fopen("currentTime.txt", "w");
    fprintf(timeFile, "%s", timeString);
    fclose(timeFile);
    return NULL;
}

/**
 * Returns the path for the newest folder of rooms
 **/
char *getCurPath() {
    struct dirent *dp;
    DIR *directory = opendir(".");
    struct stat dStat;
    time_t latest = 0;
    char folderName[50];
    while ((dp = readdir(directory)) != NULL) {
        memset(&dStat, 0, sizeof(dStat));
        if (stat(dp->d_name, &dStat) < 0) {
            printf("Error getting info on file\n");
            continue;
        }
        /*If not a directory skip*/
        if ((dStat.st_mode & S_IFDIR) != S_IFDIR) {
            continue;
        }

        /*Ignore folders . and ..*/
        if(strcmp(dp->d_name, ".") == 0 || strcmp(dp->d_name, "..") == 0) {
            continue;
        }

        /*check with the latest timestamp*/
        if (dStat.st_mtime > latest) {
            /*On finding a more recent file switch that to latest*/
            /*printf("The name of the folder is: %s\n", dp->d_name);*/
            strcpy(folderName, dp->d_name);
            latest = dStat.st_mtime;
        }
    }
    char *dirPath = malloc(50);
    strcpy(dirPath, "./");
    strcat(dirPath, folderName);
    closedir(directory);
    return dirPath;
}

/**
 * Function takes the most recent rooms folder and reads in the name and connections
 **/
struct room *getRooms(int numOfRooms) {
    /*Get name of files of rooms*/
    FILE *curFile;
    char filePath[1024];
    int counter = 0;

    struct dirent *dp;
    struct stat dStat;

    struct room *gameRooms = malloc(sizeof(struct room) * numOfRooms);
    char *dirPath = getCurPath();
    DIR *curDirectory = opendir(dirPath);
    while ((dp = readdir(curDirectory)) != NULL) {
        memset(&dStat, 0, sizeof(dStat));
        if(strcmp(dp->d_name, ".") == 0 || strcmp(dp->d_name, "..") == 0) {
            continue;
        }
        /*Get the string for the directory*/
        memset(&filePath, 0, strlen(filePath));
        strcat(filePath, dirPath);
        strcat(filePath, "/");
        strcat(filePath, dp->d_name);
        curFile = fopen(filePath, "r");

        /*Get room names*/
        gameRooms[counter].totalConnections = 0;

        char string[1024];
        int numOfConnections = 0;
        while (fgets(string, 1024, curFile) != NULL) {
            char *cmpStr;
            char tempStr[1024];
            cmpStr = strtok(string, ":");               /*Split string at :*/

            if ((strcmp(cmpStr, "ROOM NAME")) == 0) {   /*Get room name*/
                cmpStr = strtok(NULL, " ");
                strcpy(gameRooms[counter].roomName, cmpStr);
            } else if ((strcmp(cmpStr, "ROOM TYPE")) == 0) {    /*Get room type*/
                cmpStr = strtok(NULL, " ");
                strcpy(gameRooms[counter].roomType, cmpStr);
            } else {
                cmpStr = strtok(NULL, " ");         /*Get connection rooms*/
                strtok(cmpStr, "\n");               /*Trim trailing newline*/
                strcpy(tempStr, cmpStr);

                strcpy(gameRooms[counter].connections[numOfConnections], tempStr);
                gameRooms[counter].totalConnections++;
                numOfConnections++;
            }
        }
        fclose(curFile);
        counter++;
    }
    closedir(curDirectory);
    return gameRooms;
}


int main() {
    pthread_mutex_lock(&mutex1);
    pthread_t timeThread;
    pthread_create(&timeThread, NULL, &getTime, NULL);

    
    int numOfRooms = 7;    
    struct room* gameRooms = getRooms(numOfRooms);

    /*Find start room*/
    struct room *startRoom;
    int i;
    for (i = 0; i < numOfRooms; i++) {
        if ((strcmp(gameRooms[i].roomType, "START_ROOM")) == 0) {
            startRoom = &gameRooms[i];
        }
    }

    /* Start game */
    int userMoves = 0;
    FILE *steps;
    steps = fopen("./steps", "w+");
    while (1) {
        char userInput[64];
        printf("CURRENT LOCATION: %s\n", startRoom->roomName);
        printf("POSSIBLE CONNECTIONS: ");
        int i;
        for (i = 0; i < startRoom->totalConnections; i++) {
            printf("%s%s", startRoom->connections[i], " ");
        }
        printf("\nWHERE TO? >");
        scanf("%s", userInput);
        printf("\n");

        /*If user input is time*/
        while (strcmp(userInput, "time") == 0) {
            pthread_mutex_unlock(&mutex1);  /*Unlock thread to perform time operation there*/
            pthread_join(timeThread, NULL); /*Wait for timeThread to finish*/
            pthread_mutex_lock(&mutex1);    /*Lock thread after it finished operation*/
            pthread_create(&timeThread, NULL, &getTime, NULL);

            /*Read the time using main thread*/
            FILE *timeFile;
            timeFile = fopen("currentTime.txt", "r");
            char string[1024];
            while (fgets(string, 1024, timeFile)) {
                        printf("%s", string);
            }

            /*Ask user for input again after printing time*/
            printf("\nWHERE TO? >");
            scanf("%s", userInput);
            printf("\n");
        }



        /*Check if the user input room exists*/
        int found = -1;
        for (i = 0; i < startRoom->totalConnections; i++) {
            if ((strcmp(userInput, startRoom->connections[i])) == 0) {
                found = 1;
            }
        }
        /*Point startRoom to the user requested room*/
        if (found == 1) {
            int j;
            for (j = 0; j < numOfRooms; j++) {
                if ((strcmp(userInput, gameRooms[j].roomName)) == 0) {
                    startRoom = &gameRooms[j];
                    userMoves++;
                    fprintf(steps, "%s\n", gameRooms[j].roomName);
                }
            }
        } else {
            printf("%s", "HUH? I DON\'T UNDERSTAND THAT ROOM. TRY AGAIN.\n\n");
        }

        /*If the room is the end room, finish game*/
        if ((strcmp(startRoom->roomType, "END_ROOM")) == 0) {
            fclose(steps);
            printf("%s", "\nYOU HAVE FOUND THE END ROOM. CONGRATULATIONS!\n");
            printf("YOU TOOK %d STEPS. YOUR PATH TO VICTORY WAS:\n", userMoves);
            char string[1024];
            FILE *printSteps;
            printSteps = fopen("./steps", "r");
            while (fgets(string, 1024, printSteps)) {
                printf("%s", string);
            }
            fclose(printSteps);
            remove("./steps");
            break;
        }
    }


    return 0;
}

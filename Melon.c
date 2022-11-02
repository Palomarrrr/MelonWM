//some common functions id rather have in one common file

#include "Melon.h"

//Convert #ffffff to 0xffffff for easier hex translations
char *convertColorString(char *color){
	char *buff = malloc(sizeof(char) * strlen(color));
	char *colorOut = malloc(sizeof(char) * strlen(color) + 1);
	for(int i = 1; i  <= strlen(color); i++)
		buff[i - 1] = color[i];
	colorOut[0] = '0';
	colorOut[1] = 'x';
	strcat(colorOut, buff);
	return colorOut;
}

//Determine the flag presented
int findFlagRequest(char* argv){

	if(strcmp("-h", argv) == 0 || strcmp("--help", argv) == 0){
		return 0;
	}else if(strcmp("-c", argv) == 0){
		return 1;
	}else if(strcmp("-n", argv) == 0){
		return 2;
	}else if(strcmp("-l", argv) == 0){
		return 3;
	}else{
		return -1;
	}

}

//Process flags for the program
void processFlags(int argc, char **argv, char **configPath, char *logFilePath, FILE **logFile){
	if(argc == 0)
		return;

	for(int i = 1; i < argc; i++){
		int request = findFlagRequest(argv[i]);

		switch(request){
			case 0:
				fprintf(*logFile, "%ld: -h FLAG USED\n", clock());
				printf("\n\nHELP MESSAGE\n");
				printf("  -c /path/to/file | Uses a specific path for config | default location: %s\n", *configPath);
				printf("  -n\t| Runs without a config and uses defaults\n");
				printf("  -l\t| Turns the debug log on | default location: %s \n", logFilePath);
				printf("  -h\t| Displays this message\n");
				exit(1);
			case 1:
				fprintf(*logFile, "%ld: -c FLAG USED\n", clock());
				i++;
				*configPath = argv[i];
				break;
			case 2:
				fprintf(*logFile, "%ld: -n FLAG USED\n", clock());
				*configPath = realloc(*configPath, sizeof(char) * 2);
				memcpy(*configPath, "0", strlen(*configPath));
				break;
			case 3:
				fprintf(*logFile, "%ld: -l FLAG USED. LOG FILE SWITCHED ON\n", clock());
				*logFile = freopen(logFilePath, "w", *logFile);
				break;
			default:
				printf("Invalid flag: %s\n", argv[i]);
				printf("Check -h for more information\n");
				fprintf(*logFile, "%ld: INVALID FLAG REQUEST: %s\n", clock(), argv[i]);
				exit(1);
		}
	}
}

//Get the path to a file
char *getPath(char *dir, char *dest){
	char *userHome = getenv(dir);
	char *path = malloc(sizeof(userHome) * strlen(userHome));
	memcpy(path, userHome, strlen(userHome) * 8);
	strcat(path, dest);
	return path;
}

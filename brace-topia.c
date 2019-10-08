/*
* Brace Team Simulation
* Surround yourself with programmers that use the same brace style. 
* Implementation that utilizes a city grid where programmers are at odds.
*
* @author Gunther Kroth   gdk6217@rit.edu
*/

#define _DEFAULT_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <math.h>
#include <unistd.h>

#include "display.h"

// rng seed
#define SEED 41

// default values for simulation
#define DEFAULT_TIME_DELAY 900000
#define DEFAULT_MAX_CYCLE -1
#define DEFAULT_DIMENSION 15
#define DEFAULT_STRENGTH_PREFERENCE 50
#define DEFAULT_VACANCY 20.0
#define DEFAULT_ENDLINE 60.0


/*
* print message when the -h flag is used
*/
void printHelpUsage() {
	fprintf(stderr,
		"usage:\n"
		"brace-topia [-h] [-t N] [-c N] [-d dim] [-s %%str] [-v %%vac] [-e %%end]\n"
		"Option      Default   Example   Description\n"
		"'-h'        NA        -h        print this usage message.\n"
		"'-t N'      900000    -t 5000   microseconds cycle delay.\n"
		"'-c N'      NA        -c4       count cycle maximum value.\n"
		"'-d dim'    15        -d 7      width and height dimension.\n"
		"'-s %%str'   50        -s 30     strength of preference.\n"
		"'-v %%vac'   20        -v30      percent vacancies.\n"
		"'-e %%endl'  60        -e75      percent Endline braces. Others want Newline.\n"
		);
}


/*
* print the usage message
* used when incorrect flags are given
*/
void printUsage() {
	fprintf(stderr,
		"usage:\n"
		"brace-topia [-h] [-t N] [-c N] [-d dim] [-s %%str] [-v %%vac] [-e %%end]\n"
		);
}


/*
* create an array of chars that represent people in the city
*
* @param dim dimension for array
* @param city[] array to populate with chars
* @param endlines number of endline 'e' chars
* @param newlines number of newline 'n' chars
*/
void initializeLineCity(int dim, char city[], int endlines, int newlines) {
	for (int i = 0; i < dim; i++) {
		if (endlines > 0) {
			city[i] = 'e';
			endlines--;
		} else if (newlines > 0) {
			city[i] = 'n';
			newlines--;
		} else {
			city[i] = '.';
		}
	}
}


/*
* shuffle the values of a city array
*
* @param dim dimension for array
* @param city[] array to shuffle
* @precondition city[] should be fully initialized
*/
void shuffleLineCity(int dim, char city[]) {
	for (int i = 0; i < dim; i++) {
		int r = (rand() % dim);
		char temp = city[r];
		city[r] = city[i];
		city[i] = temp;
	}
}


/*
* convert an city into a 2D city array
*
* @param dim dimension for 2D array
* @param line[] city array
* @param city[][] 2D array to populate
* @precondition line[] should be fully initialized
*/
void convertLineCity(int dim, char line[], char city[][dim]) {
	int i = 0;
	for (int row = 0; row < dim; row++) {
		for (int col = 0; col < dim; col++) {
			char c = line[i];
			city[row][col] = c;
			i++;
		}
	}
}


/*
* print a city with formatting and statistics
*
* @param dim dimesion for array
* @param city[][] 2D city array to print
* @param cycle current cycle number of the city
* @param moveCount number of moves performed this cycle
* @param happyRating happiness rating for entire city
* @param strength percent preference rating for neighbors
* @param vacancy percent amount of vacant city locations
* @param endlines percent number of endline characters of population
*/
void printCity(int dim, char city[][dim], int cycle, int moveCount, float happyRating, int strength, int vacancy, int endline) {
	for (int row = 0; row < dim; row++) {
		for (int col = 0; col < dim; col++) {
			char c = city[row][col];
			printf("%c", c);
		}
	printf("\n");
	}

	printf("cycle: %d\n", cycle);
	printf("moves this cycle: %d\n", moveCount);
	printf("teams' \"happiness\": %.4f\n", happyRating);
	printf("dim: %d, %%strength of preference: %d%%, %%vacancy: %d%%, %%end: %d%%\n", dim, strength, vacancy, endline);
	printf("Use Control-C to quit.\n");
}


/*
* print a city with formatting and statistics
* used for infinite print mode
*
* @param dim dimesion for array
* @param city[][] 2D city array to print
* @param cycle current cycle number of the city
* @param moveCount number of moves performed this cycle
* @param happyRating happiness rating for entire city
* @param strength percent preference rating for neighbors
* @param vacancy percent amount of vacant city locations
* @param endlines percent number of endline characters of population
*/ 
void displayPrintCity(int dim, char city[][dim], int cycle, int moveCount, float happyRating, int strength, int vacancy, int endline) {
	set_cur_pos(1, 1);
	for (int row = 0; row < dim; row++) {
		set_cur_pos(row + 1, 1);
		for (int col = 0; col < dim; col++) {
			char c = city[row][col];
			put(c);
		}
	}

	set_cur_pos(dim + 1, 1);
	printf("cycle: %d", cycle);
	set_cur_pos(dim + 2, 1);
	printf("moves this cycle: %d", moveCount);
	set_cur_pos(dim + 3, 1);
	printf("teams' \"happiness\": %.4f", happyRating);
	set_cur_pos(dim + 4, 1);
	printf("dim: %d, %%strength of preference: %d%%, %%vacancy: %d%%, %%end: %d%%", dim, strength, vacancy, endline);
	set_cur_pos(dim + 5, 1);
	printf("Use Control-C to quit.");
}


/*
* check if a char is one that represents a vacant city location
* vacant locations are '.' or '*'
*
* @param char character to check
* @return 1 if not vacant, 0 if vacant
*/
int notVacant(char c) {
	if (c == '.' || c == '*') {
		return 0;
	} else {
		return 1;
	}
}


/*
* get the happiness measure at a specific city location
* neighbors are checked in 8 cardinal directions
* happiness measure is number of neighbors with like brace to total neighbors
*
* @param row index of row to check
* @param col index of col to check
* @param dim dimension for city array
* @param city[][] city array
* @param brace char that lives at the city location
* @return float ration of goodNeighbors:totalNeighbors
*/
float getHappyMeasure(int row, int col, int dim, char city[][dim], char brace) {
	int goodNeighbors = 0;
	int totalNeighbors = 0;

	//north
	if (row > 0) {
		char n = city[row - 1][col];
		if (notVacant(n)) {
			totalNeighbors++;
			if (n == brace) {
				goodNeighbors++;
			}
		}
		//northwest
		if (col > 0) {
			char nw = city[row - 1][col - 1];
			if (notVacant(nw)) {
				totalNeighbors++;
				if (nw == brace) {
					goodNeighbors++;
				}
			}
		}
		//northeast
		if (col < dim - 1) {
			char ne = city[row - 1][col + 1];
			if (notVacant(ne)) {
				totalNeighbors++;
				if (ne == brace) {
					goodNeighbors++;
				}
			}
		}
	}

	//south
	if (row < dim - 1) {
		char s = city[row + 1][col];
		if (notVacant(s)) {
			totalNeighbors++;
			if (s == brace) {
				goodNeighbors++;
			}
		}
		//southwest
		if (col > 0) {
			char sw = city[row + 1][col - 1];
			if (notVacant(sw)) {
				totalNeighbors++;
				if (sw == brace) {
					goodNeighbors++;
				}
			}
		}
		//southeast
		if (col < dim - 1) {
			char se = city[row + 1][col + 1];
			if (notVacant(se)) {
				totalNeighbors++;
				if (se == brace) {
					goodNeighbors++;
				}
			}
		}
	}

	//east
	if (col < dim - 1) {
		char e = city[row][col + 1];
		if (notVacant(e)){
			totalNeighbors++;
			if (e == brace) {
				goodNeighbors++;
			}
		}
	}

	//west
	if (col > 0) {
		char w = city[row][col - 1];
		if (notVacant(w)) {
			totalNeighbors++;
			if (w == brace) {
				goodNeighbors++;
			}
		}
	}

	float happyRatio;
	if (totalNeighbors == 0) {
		happyRatio = 1;
	} else {
		happyRatio = (float)goodNeighbors / (float)totalNeighbors;
	}

	return happyRatio;
}


/*
* move a char from one city location to another
* scans moves left to right, top to bottom for a vacant location
* moves only execute if the char will be happier there
* vaccated spots are marked with a '*' for cycle retention
*
* @param row x coordinate of char
* @param col y coordinate of char
* @param dim dimension for char array
* @param city[][] city array
* @return 1 if moved, 0 otherwise
*/
int move(int row, int col, int dim, char city[][dim]) {
	char c = city[row][col];
	float currentHappy = getHappyMeasure(row, col, dim, city, c);
	int x;
	int y;

	for (x = 0; x < dim; x++) {
		for (y = 0; y < dim; y++) {
			// possible move-in spot
			char slot = city[x][y];
			if (slot == '.') {
				// in this vacant spot, check to see if move is beneficial
				float possibleHappy = getHappyMeasure(x, y, dim, city, c);
				if (possibleHappy >= currentHappy) {
					city[x][y] = c;
					city[row][col] = '*';
					return 1;
				}
			}
		}
	}

	return 0;
}


/*
* remove '*' markers at the end of a cycle
* '*' are used to indicate spots that have been vacated during the cycle
* '*' are turned into '.'
* also serves as a way to calculate how many moves were conducted in a cycle
*
* @param dim dimension for array
* @param city[][] city array
* @return number of marker chars
*/
int clearMarkedVacants(int dim, char city[][dim]) {
	int count = 0;
	for (int row = 0; row < dim; row++) {
		for (int col = 0; col < dim; col++) {
			char c = city[row][col];
			if (c == '*') {
				city[row][col] = '.';
				count++;
			}
		}
	}
	return count;
}


/*
* calculate the global city happiness value
* average value of each inhabitant in the city
* 'e' and 'n' are valid inhabitants
*
* @param dim dimension for array
* @param city[][] city array
* @return average happiness ratio for entire city population
*/
float cityHappyRating(int dim, char city[][dim]) {
	float sum = 0;
	int people = 0;
	for (int row = 0; row < dim; row++) {
		for (int col = 0; col < dim; col++) {
			char c = city[row][col];
			if (c == 'e' || c == 'n') {
				float cHappy = getHappyMeasure(row, col, dim, city, c);
				sum += cHappy;
				people++;
			}
		}
	}
	return sum / people;
}


/*
* generate the next city cycle
* each inhnbitant is examined and possibly moved
* if an inhabitant's happiness raito is below the given measure, a move is attempted
*
* @param dim dimension for city array
* @param city[][] city array
* @param measure happiness measure
*/
void nextCycle(int dim, char city[][dim], float measure) {
	for (int row = 0; row < dim; row++) {
		for (int col = 0; col < dim; col++) {
			char c = city[row][col];
			if (c != '.') {
				float happy = getHappyMeasure(row, col, dim, city, c);
				if (happy < measure) {
					move(row, col, dim, city);
				}
			}
		}
	}
}


/*
* run the brace war simulation!
* option flags are set to default values, unless provided through flags
*
*/
int main(int argc, char * argv[]) {
	// initialize default flag values
	long int timeDelay = DEFAULT_TIME_DELAY;
	int maxCycle = DEFAULT_MAX_CYCLE;
	int dimension = DEFAULT_DIMENSION;
	float strengthPreference = DEFAULT_STRENGTH_PREFERENCE;
	float vacancy = DEFAULT_VACANCY;
	float endlineRatio = DEFAULT_ENDLINE;

	int opt;
	int temp;
	while ((opt = getopt(argc, argv, "ht:c:d:s:v:e:")) != -1) {
		switch(opt) {
		case 'h':
			// help
			printHelpUsage();
			return 0;
		case 't':
			// time delay
			temp = (long int)strtol(optarg, NULL, 10);
			if (temp <= 0) {
				break;
			} else {
				timeDelay = temp;
				break;
			}
		case 'c':
			// max cycles
			temp = (int)strtol(optarg, NULL, 10);
			if (temp < 0) {
				fprintf(stderr, "count (%d) must be a non-negative integer.\n", temp);
				printUsage();
				return (EXIT_FAILURE);
			} else {
				maxCycle = temp;
				break;
			}
		case 'd':
			// dimensions
			temp = (int)strtol(optarg, NULL, 10);
			if (temp < 5 || temp > 39) {
				fprintf(stderr, "dimension (%d) must be a value in [5...39]\n", temp);
				printUsage();
				return (EXIT_FAILURE);
			} else {
				dimension = temp;
				break;
			}
		case 's':
			// preference strength
			temp = (int)strtol(optarg, NULL, 10);
			if (temp <= 0 || temp >99 ) {
				fprintf(stderr, "preference strength (%d) must be a value in [1...99]\n", temp);
				printUsage();
				return (EXIT_FAILURE);
			} else {
				strengthPreference = temp;
				break;
			}
		case 'v':
			// vacancy percent
			temp = (int)strtol(optarg, NULL, 10);
			if (temp <=0 || temp > 99) {
				fprintf(stderr, "vacancy (%d) must be a value in [1...99]\n", temp);
				printUsage();
				return (EXIT_FAILURE);
			} else {
				vacancy = temp;
				break;
			}
		case 'e':
			// endline percent, newline percent
			temp = (int)strtol(optarg, NULL, 10);
			if (temp <=0 || temp > 99) {
				fprintf(stderr, "endline proportion (%d) must be a value in [1...99]\n", temp);
				printUsage();
				return (EXIT_FAILURE);
			} else {
				endlineRatio = temp;
				break;
			}
		default:
			printUsage();
			return (EXIT_FAILURE);
		}
	}

	// set rng seed
	srandom(SEED);

	// create the happiness measure that determines if an inhabitant wants to move
	float happyMeasure = strengthPreference / 100;

	// create char counts for city array
	int totalPositions = dimension * dimension;
	int vacantPositions = round(totalPositions  * (vacancy / 100));
	int endlinePositions = round((totalPositions - vacantPositions) * (endlineRatio / 100));
	int newlinePositions = (totalPositions - vacantPositions) - endlinePositions;

	// array of all chars
	char lineCity[totalPositions];
	// city array
	char city[dimension][dimension];

	// initialize city array to cycle 0
	initializeLineCity(totalPositions, lineCity, endlinePositions, newlinePositions);
	shuffleLineCity(totalPositions, lineCity);
	convertLineCity(dimension, lineCity, city);

	// maxCycle default is -1
	// if user set maxCycle with -c then print mode
	if (maxCycle != -1) {
		for (int cycle = 0; cycle <= maxCycle; cycle++) {
			int moveCount = clearMarkedVacants(dimension, city);
			float happyRating = cityHappyRating(dimension, city);
			printCity(dimension, city, cycle, moveCount, happyRating, strengthPreference, vacancy, endlineRatio);
			nextCycle(dimension, city, happyMeasure);
		}
	// defalut infinite mode
	} else {
		int cycle = 0;
		clear();
		while(1) {
			int moveCount = clearMarkedVacants(dimension, city);
			float happyRating = cityHappyRating(dimension, city);
			displayPrintCity(dimension, city, cycle, moveCount, happyRating, strengthPreference, vacancy, endlineRatio);
			usleep(timeDelay);
			nextCycle(dimension, city, happyMeasure);
			cycle++;
		}
	}

	return(EXIT_SUCCESS);
}

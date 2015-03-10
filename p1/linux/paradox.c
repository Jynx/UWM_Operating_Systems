#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>
#include <assert.h>
#include <string.h>
#include <time.h>

#define TRIALS 1000;

double runTrials(int seed);

int main(int argc, char *argv[]) 
{
  FILE *infile, *outfile;
  int c;
  char *inputFile, *outputFile;
  srand(time(NULL));

  while ((c = getopt(argc, argv, "i:o:")) != -1) {
    switch (c) {
      case 'i':
        inputFile = optarg;
        infile = fopen(inputFile, "r");
        if(infile == NULL) {
          fprintf(stderr, "Error: Cannot open file %s\n", inputFile); 
          exit(1); 
        }
        break;
      case 'o':
        outputFile = optarg;
        outfile = fopen(outputFile, "w");
        if(outfile == NULL) {
          fprintf(stderr, "Error: Cannot open file %s\n", outputFile); 
          exit(1); 
        }
        break;
      default: /* '?' */ // getopt returns ? if an option character isn't recognized
        fprintf(stderr, "Usage: %s -i inputfile -o outputfile\n");
        exit(1);
    }
  }

  int number;
  double trialSuccess = 0.0;
  while (!feof(infile) && fscanf(infile, "%d", &number) > 0) {
    trialSuccess = runTrials(number);
    fprintf(outfile, "%.2f\n", trialSuccess);
  } 
return 0;
}  

double runTrials(int seed) {
  int i, j, ranNum, positive = 0;
  int array[366];
  for(i = 0; i < 1000; i++) {
    memset(array, 0, sizeof(array));
    for (j = 0; j < seed; j++) {
      ranNum = ((rand() % 365)+1);
      //fprintf(stdout, "%d\n", ranNum);
      if (array[ranNum] == 0){
        array[ranNum]++;
      } else {
        positive++;
        break;
      }
    }
  }
  return ((double)positive / 1000); 
}

// You estimate the probability that N random people have the same birthday in the following manner.
//  You run a number of trials: in each trial, you generate N random birthdays, 
//  and check if any two birthdays are the same; such a trial is called a positive trial. 
//  The probability is calculated as the number of positive trials divided by the total number of trials.
// In each trial, you should use rand() to gene

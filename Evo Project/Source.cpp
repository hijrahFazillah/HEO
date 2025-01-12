#include <iostream>
#include <ctime> 
#include <fstream>
#include <string>
#include <cmath>
#include <iomanip>
#include <thread>
#include <chrono>
#include <conio.h>
#include <cstdlib>
using namespace std;

const int MAX_GENERATION = 60;
const int POP_SIZE = 100;
const int GENE = 15;
const double MAX_CAPACITY = 17.95;
const double SV = 89500;
const double SC = 944.225;

const string APPLIANCE[GENE] = { "Refrigerator","Blender","Water Heater","Rice Cooker","Computer","Television","Charger(e.g.,phones,laptops)","Lamp (Fluorescent)","Bulb (LED or Incandescent)","Air Conditioning","Ceiling Fan","Stand Fan (Living Room)","Stand Fan (Kitchen)","Iron","Hair Dryer" };
double MAX_USAGE_DURATION[GENE] = { 24,1,1,2.5,8,4,3,8,8,8,10,8,2,1, 1 }; 
double POWER_CONSUMPTION[GENE] = { 0.3,0.6,3,0.6,0.3,0.2,0.05,0.04,0.06,1.5,0.1,0.1,0.1,1.8,1.8 };
double SUM_MAX_USAGE_DURATION = 89.5;
double USAGE_CAPACITY[GENE] = {};
double VALUE[GENE] = { 268,11,11,28,89,45,34,89,89,89,112,89,22,11,11 };


const double CROSSOVER_PROBABILITY = 0.9; //NOT LESS THAN 0.9
const double CHROMOSOME_MUTATION_PROBABILITY = 0.1;
const double GENE_MUTATION_PROBABILITY = 0.1;


double chromosome[POP_SIZE][GENE];
double fitness[POP_SIZE];//Fitness of each chromosome
double parents[2][GENE];//2 row for two parents and 10 genes
double children[2][GENE];
double newChromosome[POP_SIZE][GENE];//1. A temporary variable to store the new chromosome data structure 
int counterNewChromo;
ofstream avgFitnessFile, bestFitnessFile, bestChromoFile;//Declare three output files for average fitness, best fitness and best chromosome
double avgFitness, bestFitness = -1;
double bestChromo[GENE];

string getFormattedTime();

void initializePopulation() {
    double randomFirst, randomLast, hours;
    for (int c = 0; c < POP_SIZE; c++) {

        for (int g = 0; g < GENE; g++) {
            //Returns the time since the epoch with much higher precision (nanoseconds or microseconds)
            auto seed = chrono::high_resolution_clock::now().time_since_epoch().count();
            srand(seed);
            randomFirst = rand() % int(MAX_USAGE_DURATION[g]);
            if (randomFirst == 0) {
                randomLast = 0.5;
            }
            else {
                randomLast = rand() % 2;
                if (randomLast == 0)
                    randomLast = 0.0;
                else
                    randomLast = 0.5;
            }
            hours = randomFirst + randomLast;
            chromosome[c][g] = hours;
        }
    }
    cout << SUM_MAX_USAGE_DURATION << endl;
}

void printChromosome() {
    for (int c = 0; c < POP_SIZE; c++)
    {
        cout << "\t\tChromosome " << c << " :";
        for (int g = 0; g < GENE; g++)
        {
            cout << chromosome[c][g] << " ";
        }
        cout << endl;
    }
}

void evaluateChromosome() {
    double TV = 0; //Total Value
    double TC = 0; //Total Capacity
    cout << setw(12) << "Chromosome " << setw(16) << "Total Value" << setw(16) << "Total Capacity" << setw(16) << "Fitness" << endl;
    for (int c = 0; c < POP_SIZE; c++) {
        TV = 0;
        TC = 0;
        for (int g = 0; g < GENE; g++) {
            //Total Value = Hours x Value
            TV += chromosome[c][g] * VALUE[g];
            //Capacity = Hours x Value
            TC += chromosome[c][g] * POWER_CONSUMPTION[g];
        }
        if (TC <= 17.95) {
            fitness[c] = (((MAX_CAPACITY - TC) / MAX_CAPACITY)) * TV ;
        }
        else {
            fitness[c] = 1 / (exp(TC - MAX_CAPACITY)) * TV;

        }
        //Chromosome, TV, TC, Fitness
        cout << setw(12) << c + 1 << setw(16) << TV << setw(16) << TC << setw(16) << fitness[c] << endl;
    }
}

void parentSelection3players() {
    //declare necessary variables 
    int player1, player2, player3, indexParents[2];
    do {
        //1. For both parents
        for (int i = 0; i < 2; i++) {
            //1.1 Pick a random number to be the index for player 1
            player1 = rand() % POP_SIZE;
            do {
                player2 = rand() % POP_SIZE;
            } while (player1 == player2);
            do {
                player3 = rand() % POP_SIZE;
            } while (player2 == player3 || player1 == player3);
            //1.2 Compare fitness between player 1, player 2 and player 3, pick the best one to be  
            if (fitness[player1] > fitness[player2] && fitness[player1] > fitness[player3]) {
                indexParents[i] = player1;
            }
            else if (fitness[player2] > fitness[player1] && fitness[player2] > fitness[player3]) {
                indexParents[i] = player2;
            }
            else {
                indexParents[i] = player3;
            }

            // Output for tournament
            cout << "\n\t\tPlayer: " << player1 << " vs " << player2 << " vs " << player3 << "\n";
            cout << "\t\tWinner: " << indexParents[i] << endl;
        }
    } while (indexParents[0] == indexParents[1]);

    //3. Copy selected parents to array parents 
    for (int c = 0; c < 2; c++) {
        for (int g = 0; g < GENE; g++) {
            parents[c][g] = chromosome[indexParents[c]][g];
        }
    }

    //4. Print parent 1 and 2 
    for (int c = 0; c < 2; c++) {
        cout << "\n\t Parents " << c + 1 << " :";
        for (int g = 0; g < GENE; g++) {
            cout << parents[c][g] << " ";
        }
        cout << endl;
    }
}

void crossover() {
    double randomNumber;
    //1. Copy both parent’s chromosome to children chromosomes 
    for (int c = 0; c < 2; c++)
    {
        for (int g = 0; g < GENE; g++)
        {
            children[c][g] = parents[c][g];
        }
    }
    //2. Generate a random number from 0-1. Make sure it is real value data type
    randomNumber = (rand() % 11) / 10.0; //get 0-10 and divide by 10
    //3. If (2) less than crossover probability
    if (randomNumber < CROSSOVER_PROBABILITY) {
        //2.1 generate a random crossover point
        int xoverPoint = rand() % GENE;
        cout << "\n\t\tCrossover will happen at index " << xoverPoint << endl;
        //2.2 Crossover parent 1 and parent 2 to produce the children 
        for (int g = xoverPoint; g < GENE; g++)
        {
            if (g > xoverPoint)
            {
                children[0][g] = parents[1][g];
                children[1][g] = parents[0][g];
            }
        }
        for (int c = 0; c < 2; c++)
        {
            cout << "\t\tChildren " << c + 1 << " :";
            for (int g = 0; g < GENE; g++)
            {
                cout << children[c][g] << " ";
            }
            cout << endl;
        }
    }
    else {
        cout << "\n\t\tCrossover did not happen." << endl;
    }
    //4. Print children 1 & 2 
}

void crossover2Point()
{
    double randomNumber;
    for (int c = 0; c < 2; c++)
    {
        for (int g = 0; g < GENE; g++)
        {
            children[c][g] = parents[c][g];
        }
    }
    randomNumber = (rand() % 11) / 10.0;
    if (randomNumber < CROSSOVER_PROBABILITY) {
        int xoverPoint1 = rand() % GENE;
        int xoverPoint2 = rand() % GENE;
        cout << "\n\t Crossover will happen at index " << xoverPoint1 << " and " << xoverPoint2 << endl;
        for (int g = xoverPoint1; g < xoverPoint2; g++)
        {
            children[0][g] = parents[1][g];
            children[1][g] = parents[0][g];
        }
        for (int g = xoverPoint2; g < GENE; g++)
        {
            children[0][g] = parents[1][g];
            children[1][g] = parents[0][g];
        }
        for (int c = 0; c < 2; c++)
        {
            cout << "\n\t Children " << c + 1 << " :";
            for (int g = 0; g < GENE; g++)
            {
                cout << children[c][g] << " ";
            }
            cout << endl;
        }
    }
    else {
        cout << "\n\t Crossover did not happen." << endl;
    }
}

void mutation() {
    double randomChromosome, random, randomGene, randomFirst, randomLast, hours;
    for (int c = 0; c < 2; c++) {
        //XX% for a child chromosome to mutate
        randomChromosome = (rand() % 10) / 10.0;
        if (randomChromosome < CHROMOSOME_MUTATION_PROBABILITY) {
            cout << "\nMutation occured for child " << c + 1 << endl;
            //Atleast one gene gets 100% to mutate, others get 10% chance to mutate
            random = rand() % GENE;
            for (int g = 0; g < GENE; g++) {
                //100% to mutate for 'random' index of the gene
                if (g == random) {
                    //Random first value is assigned, it could not be more than the max value of each column
                    randomFirst = rand() % int(MAX_USAGE_DURATION[g]);
                    if (randomFirst == 0) {
                        //If first value is is 0, automatically assign 0.5 to the zero point value
                        randomLast = 0.5;
                    }
                    else {
                        //Get random value of 0,1. If 0, assign 0.0, if 1, assign 0.5.
                        randomLast = rand() % 2;
                        if (randomLast == 0)
                            randomLast = 0.0;
                        else
                            randomLast = 0.5;
                    }
                    //Sum the decimal value with the zero point value
                    hours = randomFirst + randomLast;
                    //Insert double value inside children array
                    children[c][g] = hours;
                    cout << "100% Mutation at " << g << endl;
                }
                //For every other genes, have XX% to mutate
                else {
                    randomGene = (rand() % 10) / 10.0;
                    if (randomGene < GENE_MUTATION_PROBABILITY) {
                        //Random first value is assigned, it could not be more than the max value of each column
                        randomFirst = rand() % int(MAX_USAGE_DURATION[g]);
                        if (randomFirst == 0) {
                            //If first value is is 0, automatically assign 0.5 to the zero point value
                            randomLast = 0.5;
                        }
                        else {
                            //Get random value of 0,1. If 0, assign 0.0, if 1, assign 0.5.
                            randomLast = rand() % 2;
                            if (randomLast == 0)
                                randomLast = 0.0;
                            else
                                randomLast = 0.5;
                        }
                        //Sum the decimal value with the zero point value
                        hours = randomFirst + randomLast;
                        //Insert double value inside children array
                        children[c][g] = hours;
                        cout << "X% Mutation at " << g << endl;
                    }
                }
            }
            cout << "After mutation" << endl;
            for (int g = 0; g < GENE; g++) {
                cout << children[c][g] << " ";
            }
            cout << endl;
        }
        else {
            cout << "\nMutation not occured for child " << c+1 << endl;
        }
       
    }
}

void survivalSelection() {
    for (int c = 0; c < 2; c++)//1. For each chromosome
    {
        for (int g = 0; g < GENE; g++)//1.2. For each gene 
        {
            //1.3 Copy children to the survival chromosome array 
            newChromosome[counterNewChromo][g] = children[c][g];
        }
        counterNewChromo++;//2. Update array counter 
    }
    //3. Print chromosomes in the new population so far 
    //for (int c = 0; c < POP_SIZE; c++)
    //{
    //    cout << "\t\tNew Chromosome " << c << " :";
    //    for (int g = 0; g < GENE; g++)
    //    {
    //        cout << newChromosome[c][g] << " ";
    //    }
    //    cout << endl;
    //}
}

void copyChromosome() {
    for (int c = 0; c < POP_SIZE; c++)//1. For each chromosome
    {
        for (int g = 0; g < GENE; g++)//1.2. For each gene 
        {
            //1.3 Copy children to the survival chromosome array 
            chromosome[c][g] = newChromosome[c][g];
        }
    }
}

void calculateAverageFitness() {
    //1. Declare a variable for totalFitness, initialize to 0 
    float totalFitness = 0;
    //2. For every chromosome 
    for (int c = 0; c < POP_SIZE; c++) {
        //2.1 Accumulate the fitness into totalFitness 
        totalFitness = totalFitness + fitness[c];
    }
    //3. Divide the totalFitness with population size
    avgFitness = totalFitness / POP_SIZE;
    //4. Print out the average to the screen 
    cout << "\n\tAverage Fitness : " << avgFitness;
    //5. Print out the average to an output file that keep average fitness
    avgFitnessFile << avgFitness << endl;
}

void recordBestFitness() {
    //2. For each chromosome  
    for (int c = 0; c < POP_SIZE; c++) {
        //2.1. if (fitness current chromosome better than bestFitness){ 
        if (fitness[c] > bestFitness)
        {
            bestFitness = fitness[c];//2.1.1. bestFitness = fitness for the current chromosome 
            for (int g = 0; g < GENE; g++) {
                bestChromo[g] = chromosome[c][g];//2.1.2. copy the chromosome to bestChromosome 
            }
        }
    }
    //3. Print the bestFitness and bestChromosome to the screen 
    cout << "\n\tBest Fitness : " << bestFitness << endl;
    bestFitnessFile << bestFitness << endl;
    //4. Print the bestFitness and bestChromosome to two separate files
    cout << "\n\tBest Chromo = ";
    for (int g = 0; g < GENE; g++)
    {
        cout << bestChromo[g] << " ";
        bestChromoFile << bestChromo[g] << " ";
    }
    cout << endl;
    bestChromoFile << endl;
}

string getFormattedTime() {
    time_t rawTime;
    struct tm timeInfo;
    char buffer[11]; // Adjust buffer size to fit "HH:MM:SSAM/PM"

    time(&rawTime);
    localtime_s(&timeInfo, &rawTime);

    // Format time as "HH:MM:SSAM/PM"
    strftime(buffer, sizeof(buffer), "%I:%M:%S%p", &timeInfo);

    return string(buffer);
}


int main()
{
    string start = getFormattedTime();

    avgFitnessFile.open("avgFitness.txt");
    bestFitnessFile.open("bestFitness.txt");
    bestChromoFile.open("bestChromo.txt");
    cout << "\nGA start";
    cout << "\n\tInitialize Chromosome\n";
    initializePopulation();
    for (int gen = 0; gen < MAX_GENERATION; gen++)
    {
        cout << "\n\tGeneration " << gen + 1 << endl;
        //printChromosome();
        cout << "\n\tEvaluate Chromosome\n";
        evaluateChromosome();
        //_getch();
        counterNewChromo = 0;
        for (int j = 0; j < POP_SIZE / 2; j++) {
            parentSelection3players();
            cout << "\n\tCrossover\n";
            crossover();
            cout << "\n\tMutation\n";
            mutation();
            cout << "\n\tSurvival Selection\n";
            survivalSelection();
        }
        cout << "\n\tCopy Chromosome\n";
        copyChromosome();
        evaluateChromosome();
        calculateAverageFitness();
        recordBestFitness();
    }
    cout << "\n\n\nStart Time: " << start << endl;
    string end = getFormattedTime();
    cout << "End Time: " << end << endl;
    cout << "GA Complete" << endl;
    avgFitnessFile.close();
    bestFitnessFile.close();
    bestChromoFile.close();
}

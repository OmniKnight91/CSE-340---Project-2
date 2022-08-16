using namespace std;
#include <stdlib.h>
//#include <vector>
#include "lexer.h"

#define MAXIMUM_TKN_LENGTH 100
#define MAXIMUM_RULES 100



struct symbol
{
	int sym;
	vector<bool> firstSet;
	vector<bool> followSet;
};

struct terminal
{
	string termString;
	int num = 0;	//count of times terminal appears on RHS
	terminal *next;
};
terminal* terminalList = NULL;

struct Symbol
{
	int sym;
	vector<bool> firstSet;
	vector<bool> followSet;
};


struct GramRule
{
	string LHS;				//left hand side symbol
	int iLHS = 0;
	int RHS[MAXIMUM_RULES];	//right hand side rules
	int size = 0;				//number of rules
	bool notGen = false;
	bool notReach = false;

};
GramRule* gList = NULL; //glist is the grammer list

/****Forward Declarations******/

void countGram();
void CalcFirst();
void prntFirst();
void CalcFollow();
void prntFollow();
void addLHSIndex();
void addGramRule(Token t);
void analyzeGrammar();
void chckGeneration();
void chckReachable();
void updateForG();
void updateForRch();
void deleteUseless();
void prntNoUseless();
void setupSymSets();
void PredictiveParse();


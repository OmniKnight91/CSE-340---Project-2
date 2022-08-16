
#include <stdio.h>
#include <stdbool.h>
#include <iostream>
//extern "C"

//#include "inputbuf.h"
//#include "lexer.h"
#include "project2.h"


LexicalAnalyzer lexer;

bool isPredictiveParse = false;
bool fstHash = false; // first hash
bool scdHash = false; // second hash
bool aftaArrow = false; //after arrow
bool rAfter = false;  //right after


int totalSymbols = 0;
int testCount = 0;
int countTerms = 0;
int countNonTerms = 0;


string lastNonTerminal;
int nonTermCount[9999];
string termArray[9999];


vector<string> symbolTable;
vector<string> nonTerminalList;
vector<string> terminaList;
vector<string> predictTerminal;
vector<string> predictNonTerminal;

vector<bool> gArray;
vector<bool> tmpGChange;
vector<bool> rchArray;
vector<bool> tmpRchChange;

vector<symbol> tempSymbolVector;
vector<symbol> symbolVector;
vector<GramRule> DaGram;



void analyzeGrammar()
{
	symbolTable.push_back("#");
	totalSymbols++;
	symbolTable.push_back("$");
	totalSymbols++;

	Token t = lexer.GetToken();

	int count = 1;

	while (t.token_type != END_OF_FILE && t.token_type != DOUBLEHASH)
	{
		//cout << "while1\n";
		bool isNonTerminal = false;
		//cout << tCount << endl;
		if (t.token_type == ID || t.token_type == NUM)
		{
			if (!fstHash)		//this should be the tokens prior to the first hash and they are also the non-terminals
			{
				string temp(t.lexeme); // t.lexeme == current token
				terminaList.push_back(temp);
				symbolTable.push_back(t.lexeme);
				countNonTerms++;
				totalSymbols++;
				//cout << "NTRULE\n";

			}
			else if (fstHash && !scdHash)	//this should be all the terminals
			{
				//cout << "NONTERMINALS\n";
				//cout << "secondHash\n";
				string temp(t.lexeme);
				nonTerminalList.push_back(temp);	// non-terminal is added to vector list
				symbolTable.push_back(temp);
				totalSymbols++;
				countTerms++;
			}
			else if (!aftaArrow && fstHash && scdHash)	//this should be any non-terminal as LHS of a rule
			{
				//cout << "NTRULE\n";
				string temp(t.lexeme);
				lastNonTerminal = temp;
			}
			else if (aftaArrow && fstHash && scdHash)	//this should be all tokens following an arrow on the RHS of a rule
			{
				//cout << "RHSRULE\n";

				addGramRule(t);

				if (rAfter)	//used for checking rule epsilon S -> #
				{
					rAfter = false;
				}
			}
		}

		else if (t.token_type == HASH)
		{
			//cout << "aHash\n";
			if (!fstHash)
			{
				//cout << "firstHash1\n";
				fstHash = true;
			}

			else if (!scdHash && fstHash)
			{
				//cout << "secHash2\n";
				scdHash = true;
			}

			if (rAfter)
			{
				GramRule tempRule;
				tempRule.LHS = lastNonTerminal;
				tempRule.RHS[0] = 0;
				tempRule.size = 1;
				DaGram.push_back(tempRule);
				//add # rule
			}

			aftaArrow = false;  //resets to show new rule is available
			rAfter = false;
		}

		else if (t.token_type == ARROW)
		{
			aftaArrow = true;
			rAfter = true;
		}
		count++;
		t=lexer.GetToken();
	}
}

void addGramRule(Token t)
{

	string temp(t.lexeme);

	int index = 0;
	int RHScount = 0;
	bool done = false;
	GramRule tempRule;

	while (index < totalSymbols && t.token_type != HASH)
	{
		if (temp.compare(symbolTable.at(index)) == 0)
		{
			
			tempRule.RHS[RHScount] = index;
			tempRule.LHS = lastNonTerminal;
			RHScount++;
			t = lexer.GetToken();
			temp = t.lexeme;
			index = 0;

		}
		index++;
	}
	tempRule.size = RHScount;
	DaGram.push_back(tempRule);

	lexer.UngetToken(t);

}

void addLHSIndex()
{
	for (int i = 0; i < symbolTable.size(); i++)
	{
		for (int j = 0; j < DaGram.size(); j++)
		{
			if (DaGram.at(j).LHS.compare(symbolTable.at(i)) == 0)
			{
				DaGram.at(j).iLHS = i;
			}
		}
	}
}

void setupSymSets()
{
	symbol tempSymbol;

	for (int i = 0; i < symbolTable.size(); i++)
	{
		tempSymbol.sym = i;
		for (int i = 0; i < 2 + countNonTerms; i++)
		{
			tempSymbol.firstSet.push_back(false);
			tempSymbol .followSet.push_back(false);
		}
		symbolVector.push_back(tempSymbol);
	}
}

void countGram()
{
	bool ifInRule = false;		//check to see if the symbol is in the rule

	for (int i = 2; i < symbolTable.size(); i++)	//going through the symbolTable
	{
		string tempSym = symbolTable.at(i);
		int NumberOfSymbols = 0;

		for (int j = 0; j < DaGram.size(); j++)	//check each grammar rule
		{
			int tempSize = sizeof(DaGram.at(j).RHS) / sizeof(int);

			for (int k = 0; k < DaGram.at(j).size; k++)	//check each symbol in grammar rule
			{
				if (tempSym.compare(DaGram.at(j).LHS) == 0 || i == DaGram.at(j).RHS[k])
				{
					ifInRule = true;

				}
			}

			if (ifInRule)
			{
				NumberOfSymbols++;
				ifInRule = false;
			}
		}
		cout << tempSym + ": " << NumberOfSymbols << endl;
		NumberOfSymbols = 0;
	}
}

void chckGeneration()
{
	bool gChange = true;
	int ruleChecked = 0;
	//initalize generating array to false
	for (int i = 0; i < totalSymbols; i++)
	{
		bool tmpBool = false;
		gArray.push_back(tmpBool);
	}
	gArray.at(0) = true; //set epsilon and terminals indexes to true
						   // << countNonTerms << " countNonTerms\n";
	for (int i = 0; i < countNonTerms; i++)
	{
		gArray.at(i + 2) = true;
	}
	//int itera = 1;
	while (gChange)
	{
		tmpGChange = gArray;
		gChange = false;

		while (ruleChecked < DaGram.size())
		{

			for (int i = 0; i < gArray.size(); i++)	//check each rule against every generating array index
			{
				int numGens = 0;		//number of RHS syms that are generating
				for (int k = 0; k < DaGram.at(ruleChecked).size; k++)	//check each symbol in grammar rule
				{
					if (gArray.at(DaGram.at(ruleChecked).RHS[k]))
					{
						numGens++;
					}
				}

				if (numGens == DaGram.at(ruleChecked).size)
				{
					//cout << i <<" sameoooo \n";
					int ind;
					int tmpInd = 0;
					for (ind = 0; ind < symbolTable.size(); ind++)
					{
						//cout << symbolTable.at(count) << " this sym  \n";
						if (symbolTable.at(ind).compare(DaGram.at(ruleChecked).LHS) == 0)
						{
							tmpInd = ind;
							gArray.at(tmpInd) = true;
						}
					}
				}
			}
			ruleChecked++;
		}
		for (int i = 0; i < gArray.size(); i++)
		{

			/**/
			if (gArray.at(i) != tmpGChange.at(i))
			{
				gChange = true;
			}
		}
		ruleChecked = 0;
	}
}

void updateForG()
{
	for (int i = 0; i < DaGram.size(); i++)
	{
		for (int j = 0; j < symbolTable.size(); j++)
		{
			for (int k = 0; k < DaGram.at(i).size; k++)
			{
				if ((symbolTable.at(j) == DaGram.at(i).LHS || j == DaGram.at(i).RHS[k]) && gArray.at(j) == false)
				{
					DaGram.at(i).notGen = true;
				}
			}
		}
	}
}

void chckReachable()
{
	bool rchChange = true;
	int ruleChecked = 0;
	int prvSymbol = 2 + countNonTerms;
	//initalize generating array to false
	for (int i = 0; i < totalSymbols; i++)
	{
		bool tempBool = false;
		rchArray.push_back(tempBool);
	}
	rchArray.at(2 + countNonTerms) = true; //set start symbol to true
										  // << countNonTerms << " countNonTerms\n";
										  //testReach();
	while (rchChange)
	{
		tmpRchChange = rchArray;
		rchChange = false;

		while (ruleChecked < DaGram.size())
		{
			for (int i = 0; i < rchArray.size(); i++)	//check each rule against every generating array index
			{
				//cout << DaGram.at(ruleChecked).iLHS << endl;
				if (rchArray.at(DaGram.at(ruleChecked).iLHS) == true)
				{
					//cout << DaGram.at(ruleChecked).iLHS << endl;
					for (int k = 0; k < DaGram.at(ruleChecked).size; k++)	//check each symbol in grammar rule
					{
						if (rchArray.at(DaGram.at(ruleChecked).RHS[k]) == false && gArray.at(prvSymbol) == true)
						{
							rchArray.at(DaGram.at(ruleChecked).RHS[k]) = true;
							//cout << DaGram.at(ruleChecked).RHS[k] << endl;
						}
						prvSymbol = DaGram.at(ruleChecked).RHS[k];
					}
				}

			}
			ruleChecked++;
		}
		//testGen();
		//test1Gen();
		//testReach();
		for (int i = 0; i < gArray.size(); i++)
		{
			if (rchArray.at(i) != tmpRchChange.at(i))
			{
				rchChange = true;
			}
		}
		ruleChecked = 0;
	}
}

void updateForRch()
{
	for (int i = 0; i < DaGram.size(); i++)
	{
		for (int j = 0; j < symbolTable.size(); j++)
		{
			for (int k = 0; k < DaGram.at(i).size; k++)
			{
				if (
					(symbolTable.at(j) == DaGram.at(i).LHS || j == DaGram.at(i).RHS[k])
					&& rchArray.at(j) == false
					)
				{
					DaGram.at(i).notReach = true;
				}
			}
		}
	}
}

void prntNoUseless()
{
	for (int j = 0; j < DaGram.size(); j++)	//check each grammar rule
	{
		int tempSize = totalSymbols;
		if (!DaGram.at(j).notGen && !DaGram.at(j).notReach)
		{
			cout << DaGram.at(j).LHS + "-> ";
			for (int k = 0; k < DaGram.at(j).size; k++)	//check each symbol in grammar rule
			{
				cout << symbolTable.at(DaGram.at(j).RHS[k]) << " ";

			}
			cout << endl;
		}
	}
}

void deleteUseless()
{
	chckGeneration();
	updateForG();
	chckReachable();
	updateForRch();
}

void CalcFirst()
{
	bool fstChange = true;
	bool noPrevSym = true;
	bool First = true;
	symbolVector.at(0).firstSet.at(0) = true;	//rule 2 FIRST(#) = {#};
	int prevSym = 2 + countNonTerms;

	for (int i = 2; i < 2 + countNonTerms; i++)	//rule 1 FIRST(terminal) = {terminal};
	{
		symbolVector.at(i).firstSet.at(i) = true;
	}

	while (fstChange)
	{
		tempSymbolVector = symbolVector;
		fstChange = false;
		for (int i = 0; i < DaGram.size(); i++)
		{
			First = true;	//start a new rule so next rhs symbol is the first

			for (int k = 0; k < DaGram.at(i).size; k++)	//check each symbol in grammar rule
			{
			
				int tem = DaGram.at(i).RHS[k];
				
				if (First)	//if it's a nonTerm & first symbol of the rule so add it's first set
				{
					if (tem > 2 + countNonTerms)	//it's a nonTerm and check it's first set to add
					{
						for (int j = 0; j < 2 + countNonTerms; j++)	//check the firstset of the LHS
						{
							if (symbolVector.at(DaGram.at(i).RHS[k]).firstSet.at(j) == true)
							{
								symbolVector.at(DaGram.at(i).iLHS).firstSet.at(j) = true;
							}
						}
					}
					else
					{
						symbolVector.at(DaGram.at(i).iLHS).firstSet.at(tem) = true;
					}
				}
				else //check if the rule before it has epsilon in it's first set
				{
					bool Epsilon = true;	//assume all have epsilon
					for (int l = 0; l < k; l++)	//check if each symbol up to the current symbol has epsilon in first set
					{
						if (symbolVector.at(DaGram.at(i).RHS[l]).firstSet.at(0) != true)	//if one first set doesn't contain epsilon
						{
							Epsilon = false; //assumption is wrong
						}
					}
					//if (Epsilon && DaGram.at(i).size == k)	//if all previous symbols contain epsilon add first set of current symbol
					if(Epsilon)
					{
						symbolVector.at(DaGram.at(i).iLHS).firstSet.at(0);
						for (int j = 0; j < 2 + countNonTerms; j++)	//check the firstset of the LHS
						{
							if (symbolVector.at(DaGram.at(i).RHS[k]).firstSet.at(j) == true)
							{
								symbolVector.at(DaGram.at(i).iLHS).firstSet.at(j) = true;
							}
						}
					}
				}
				First = false;
			}	
		}
		//checkforchanges
		for (int i = 2 + countNonTerms; i < symbolTable.size(); i++)	//check through first sets of only nonTerminal symbols
		{
			for (int j = 0; j < 2 + countNonTerms; j++)
			{
				if (symbolVector.at(i).firstSet.at(j) != tempSymbolVector.at(i).firstSet.at(j))
				{
					fstChange = true;
				}
			}	
		}
	}//end of while
}

void prntFirst()
{
	bool chkComma;	//check for if ',' is needed
	for (int k = 2 + countNonTerms; k < symbolVector.size(); k++)
	{
		chkComma = false;
		cout << "FIRST(" + symbolTable.at(k) + ") = { ";
		for (int j = 0; j < 2 + countNonTerms; j++)	//check the firstset of the LHS
		{
			if (!chkComma)
			{
				if (symbolVector.at(k).firstSet.at(j))
				{
					chkComma = true;
					cout << symbolTable.at(j);
				}
			}
			else
			{
				if (symbolVector.at(k).firstSet.at(j))
				{
					cout << ", " + symbolTable.at(j);
				}
			}
		}
		chkComma = true;
		cout << " }\n";
	}

}

void CalcFollow()
{
	bool fllwChange = true;
	symbolVector.at(2 + countNonTerms).followSet.at(1) = true;	//rule 1 add $ to follow(S);
															//testFirst();
															//testFollow();
	while (fllwChange)
	{
		testCount++;
		tempSymbolVector = symbolVector;
		fllwChange = false;


		for (int i = 0; i < DaGram.size(); i++)
		{
			for (int k = 0; k < DaGram.at(i).size; k++)
			{
				//rule 4

				if (k < DaGram.at(i).size - 1)
				{
					if (DaGram.at(i).RHS[k] > 2 + countNonTerms)
					{
						for (int j = 1; j < 2 + countNonTerms; j++)
						{

							if (symbolVector.at(DaGram.at(i).RHS[k + 1]).firstSet.at(j) == true)
							{
								symbolVector.at(DaGram.at(i).RHS[k]).followSet.at(j) = true;
							}

						}//for j
					}
				}
				//rule 5
				bool firstEps = true;
				int tempNu = k + 2;	//makes sure next next symbol isn't out of bounds
				while (firstEps)
				{
					firstEps = false;
					if (tempNu < DaGram.at(i).size)
					{
						if (symbolVector.at(DaGram.at(i).RHS[tempNu - 1]).firstSet.at(0) == true)	//check if the next sym has first of epsilon
						{
							for (int j = 1; j < 2 + countNonTerms; j++)	//add the first set of the next next symbol
							{
								if (symbolVector.at(DaGram.at(i).RHS[tempNu]).firstSet.at(j) == true)
								{
									symbolVector.at(DaGram.at(i).RHS[k]).followSet.at(j) = true;
								}
							}
							firstEps = true;
							tempNu++;
						}
					}
				}

				//rule2
				if (k == DaGram.at(i).size - 1 && DaGram.at(i).RHS[k] > 2 + countNonTerms)	//if the last symbol of the rule is a nonterminal add the LHS follow set to it
				{
					for (int j = 1; j < 2 + countNonTerms; j++)
					{
						if (symbolVector.at(DaGram.at(i).iLHS).followSet.at(j) == true)
						{
							symbolVector.at(DaGram.at(i).RHS[k]).followSet.at(j) = true;
						}
					}
				}
				//rule3
				if (DaGram.at(i).RHS[k] > 2 + countNonTerms && DaGram.at(i).size > 1)	//if the symbol is a nonterminal check if everything after is epsilon 
				{
					bool Epsilon = true;	//assume all have epsilon
					for (int l = k + 1; l < DaGram.at(i).size; l++)
					{
						if (symbolVector.at(DaGram.at(i).RHS[l]).firstSet.at(0) != true)	//if one first set doesn't contain epsilon
						{
							Epsilon = false; //assumption is wrong
						}
					}//for l
					if (Epsilon)	//if all the following symbols have epsilon in first set add the FOLLOW of LHS to the RHS symbol
					{
						for (int j = 0; j < 2 + countNonTerms; j++)
						{
							if (symbolVector.at(DaGram.at(i).iLHS).followSet.at(j) == true)
							{
								symbolVector.at(DaGram.at(i).RHS[k]).followSet.at(j) = true;
							}
						}
					}
					if (Epsilon && DaGram.at(i).iLHS == 0)
					{
						symbolVector.at(DaGram.at(i).RHS[k]).followSet.at(1) = true;
					}
				}
			} //for k	

		}//for i
		 //checkforchanges
		for (int i = 2 + countNonTerms; i < symbolTable.size(); i++)	//check through first sets of only nonTerminal symbols
		{
			for (int j = 0; j < 2 + countNonTerms; j++)
			{
				if (symbolVector.at(i).followSet.at(j) != tempSymbolVector.at(i).followSet.at(j))
				{
					fllwChange = true;
				}
			}
		}
	}//end of while

}

void prntFollow()
{
	bool chkComma;	//check for if ',' is needed
	for (int k = 2 + countNonTerms; k < symbolVector.size(); k++)
	{
		chkComma = false;
		cout << "FOLLOW(" + symbolTable.at(k) + ") = { ";
		for (int j = 0; j < 2 + countNonTerms; j++)	//check the firstset of the LHS
		{
			if (!chkComma)
			{
				if (symbolVector.at(k).followSet.at(j))
				{
					chkComma = true;
					cout << symbolTable.at(j);
				}
			}
			else
			{
				if (symbolVector.at(k).followSet.at(j))
				{
					cout << ", " + symbolTable.at(j);
				}
			}
		}
		chkComma = true;
		cout << " }\n";
	}
}

void PredictiveParse()
{
	/*
	bool chkComma;	//check for if ',' is needed
	int parserCount = 0;

	for (int k = 2 + countNonTerms; k < symbolVector.size(); k++)
	{
		chkComma = false;
		cout << "FIRST(" + symbolTable.at(k) + ") = { ";
		for (int j = 0; j < 2 + countNonTerms; j++)	//check the firstset of the LHS
		{
			if (!chkComma)
			{
				if (symbolVector.at(k).firstSet.at(j))
				{
					chkComma = true;
					//cout << symbolTable.at(j);
				}
			}
			else
			{
				if (symbolVector.at(k).firstSet.at(j))
				{
					//cout << ", " + symbolTable.at(j);
				}
			}
		}
		chkComma = true;
		//cout << " }\n";
	}
	*/

}




int main (int argc, char* argv[])
{
    int task;

    if (argc < 2)
    {
        printf("Error: missing argument\n");
        return 1;
    }

    /*
       Note that by convention argv[0] is the name of your executable,
       and the first argument to your program is stored in argv[1]
     */

    task = atoi(argv[1]);

    // TODO: Read the input grammar at this point from standard input

	analyzeGrammar();
	addLHSIndex();
	setupSymSets();

    /*
       Hint: You can modify and use the lexer from previous project
       to read the input. Note that there are only 4 token types needed
       for reading the input in this project.

       WARNING: You will need to modify lexer.cc and lexer.h to only
       support the tokens needed for this project if you are going to
       use the lexer.
     */

    switch (task) {
        case 1:
            // TODO: perform task 1.
			countGram();
			
            break;

        case 2:
            // TODO: perform task 2.
			deleteUseless();
			prntNoUseless();
            break;

        case 3:
            // TODO: perform task 3.
			CalcFirst();
			prntFirst();
            break;

        case 4:
            // TODO: perform task 4.
			CalcFirst();
			CalcFollow();
			prntFollow();
			
            break;

        case 5:
            // TODO: perform task 5.
			CalcFirst();
			CalcFollow();
			cout << "NO";
            break;

        default:
            printf("Error: unrecognized task number %d\n", task);
            break;
    }
    return 0;
}

//=======================================================================================
// Programmers: Alejandro Sanchez & Waymon Ho
// Class: CPSC 323
// Project 9 "LR Parser"
// Instructor: Ray Ahmadnia
//
// Description: Given the CFG: (1) E--> E+T   (5) T--> T/F
//							   (2) E--> E-T	  (6) T--> F
//							   (3) E--> T	  (7) F--> ( E )
//							   (4) T--> T*F   (8) F--> i
//		The program traces words using an LR parsing table to see if they 
//		are words in the language. 
//
//		   ||			T e r m i n a l s					|| Nonterminals
// |States ||  i  |  +  |  -  |  *  |  /  |  (  |  )  |  $  ||  E  |  T  |  F  |
// |---------------------------------------------------------------------------|
// |   0   ||  s5 |     |     |     |     |  s4 |     |     ||  1  |  2  |  3  |
// |---------------------------------------------------------------------------|
// |   1   ||     | s6  | s7  |     |     |     |     | ACC ||     |     |     |
// |---------------------------------------------------------------------------|
// |   2   ||     |  r3 |  r3 |  s8 | s9  |     |  r3 |  r3 ||     |     |     |
// |---------------------------------------------------------------------------|
// |   3   ||     | r6  | r6  | r6  | r6  |     | r6  | r6  ||     |     |     |
// |---------------------------------------------------------------------------|
// |   4   ||  s5 |     |     |     |     | s4  |     |     ||  10 |  2  |  3  |
// |---------------------------------------------------------------------------|
// |   5   ||     |  r8 |  r8 | r8  | r8  |     |  r8 |  r8 ||     |     |     |
// |---------------------------------------------------------------------------|
// |   6   ||  s5 |     |     |     |     | s4  |     |     ||     |  11 |  3  |
// |---------------------------------------------------------------------------|
// |   7   ||  s5 |     |     |     |     | s4  |     |     ||     |  12 |  3  |
// |---------------------------------------------------------------------------|
// |   8   ||  s5 |     |     |     |     | s4  |     |     ||     |     |  13 |
// |---------------------------------------------------------------------------|
// |   9   ||  s5 |     |     |     |     | s4  |     |     ||     |     |  14 |
// |---------------------------------------------------------------------------|
// |   10  ||     |  s6 |  s7 |     |     |     | s15 |     ||     |     |     |
// |---------------------------------------------------------------------------|
// |   11  ||     |  r1 |  r1 |  s8 |  s9 |     |  r1 |  r1 ||     |     |     |
// |---------------------------------------------------------------------------|
// |   12  ||     |  r2 |  r2 |  s8 |  s9 |     | r2  |  r2 ||     |     |     |
// |---------------------------------------------------------------------------|
// |   13  ||     |  r4 |  r4 |  r4 |  r4 |     |  r4 |  r4 ||     |     |     |
// |---------------------------------------------------------------------------|
// |   14  ||     |  r5 |  r5 |  r5 |  r5 |     |  r5 |  r5 ||     |     |     |
// |---------------------------------------------------------------------------|
// |   15  ||     |  r7 | r7  |  r7 |  r7 |     | r7  |  r7 ||     |     |     |
// |---------------------------------------------------------------------------|

//
//		In the 2D array string array used to represent this predictive parsing table, 
//		The character '~' is used to represent an empty cell.
//=======================================================================================
#include <iostream>
#include <string>
#include <stack>
#include <ctime>
#include <sstream>
#include <cctype>
using namespace std;

const int ROWS = 16;
const int COLS = 11;

bool testWord(string input,string table[ROWS][COLS]); //main test function
bool isValidTableEntry(string state); // checks to make sure that table entry is not empty

// obtain state from LR parse table
string aquireTableEntry (stack<string>& parseStack, string input, int & index, string table [ROWS][COLS], string & colString, string & rowString, string state);
int aquireColFromRule (string & colString, string state); // when state (table entry) is a rule, use the left side as the column
int aquireColFromInput (string input, int index, string & colString); // when state (table entry) is not a rule, use input string as the column

void handleTableEntry(stack <string> & parseStack, string  state, string & colString, string rowString, string input, int & index, string table [ROWS][COLS]);
void handleShift(stack <string> & parseStack, string & state, string colString, string rowString,string input, int & index);
void handleRule(stack <string> & parseStack, string & state, string rowString);
void handleConstant(stack <string> & parseStack, string  state, string colString, string rowString);

void displayStack(stack <string> parseStack); 
void displayLocalTime();
void readNextInput (int & index, string input);

int main()
{
	//TEST CASES
	// w1 =(i+i)*i$ 
	// w2 =(i*)$

	string word1 = "(i+i)*i";
	string word2 = "(i*)";

	//char choice = 'Y';
	// Predictive Parse Table from given CFG
	string lrParseTable [ROWS][COLS] = 
	{
		 /*0*/{"s5", "~"  , "~" , "~" , "~" , "s4", "~" , "~" , "1" ,"2" , "3"},
		 /*1*/{"~" ,"s6"  ,"s7 ", "~" , "~" , "~" ,"~"  ,"acc",  "~","~" , "~"},
		 /*2*/{"~ ", "r3" , "r3", "s8", "s9","~ " , "r3", "r3", "~" ,"~" , "~"},
	  	 /*3*/{"~" , "r6" , "r6","r6" ,"r6 ", "~" , "r6", "r6", "~" ,"~" , "~"},
		 /*4*/{"s5",  "~" , "~" , "~" , "~" ,"s4 ", "~" , "~" , "10","2" , "3"},
		 /*5*/{"~" , "r8" , "r8", "r8","r8" ,"~  ", "r8","r8" , "~" ,"~" , "~"},
		 /*6*/{"s5",  "~" , "~" , "~" , "~" ,"s4 ", "~" , "~" , "~" ,"11", "3"},
		 /*7*/{"s5",  "~" , "~" , "~" , "~" ,"s4 ", "~" , "~" , "~" ,"12", "3"},
		 /*8*/{"s5",  "~" , "~" , "~" , "~" ,"s4 ", "~" , "~" , "~" ,"~" ,"13"},
		 /*9*/{"s5",  "~" , "~" , "~" , "~" ,"s4 ", "~" , "~" , "~" ,"~" ,"14"},
		/*10*/{"~" , "s6" , "s7", "~" , "~" ,"~  ","s15", "~" , "~" ,"~" , "~"},
		/*11*/{"~" , "r1" , "r1","s8" ,"s9" ,"~  ", "r1","r1" , "~" ,"~" , "~"},
		/*12*/{"~" , "r2" , "r2", "s8", "s9","~  ", "r2", "r2", "~" ,"~" , "~"},
		/*13*/{"~" , "r4" , "r4", "r4","r4" ,"~  ", "r4", "r4", "~" ,"~" , "~"},
		/*14*/{"~" , "r5" , "r5", "r5", "r5","~  ", "r5", "r5", "~" ,"~" , "~"},
		/*15*/{"~" , "r7" , "r7", "r7", "r7","~  ", "r7", "r7", "~" ,"~" , "~"},
	};

	// Display today's date and time
	displayLocalTime();

	cout << "\t**LR PARSER**\n\nnote: ~ represents empty.\n\n";
	cout << "Word being tested: " << word1 << endl;
	if (testWord(word1,lrParseTable))
		cout << "The word " << word1 << " is accepted\n"
		<< "****************************************************\n";
	else
		cout << "The word " << word1 << " is not accepted\n"
		<< "****************************************************\n";

	cout << "\nWord being tested: " << word2 << endl;
	if (testWord(word2,lrParseTable))
		cout << "The word " << word2 << " is accepted\n"
		<< "****************************************************\n";
	else
		cout << "The word " << word2 << " is not accepted\n"
		<< "****************************************************\n";

	cout << endl;
	return 0; 
}// end of main()
// ____________________________________________________________________
//
//							*FUNCTIONS*
//_____________________________________________________________________

// ======= testWord() =================================================
// pre-value: string representing word to test, table representing FA.
// post-value: bool value representing if word is in language or not.
//
// description: This function accepts a string and a 2D int array
// representing the predictice parsing table generated for a given language.
// This function will return true or false depending on if
// the word is accepted by the grammar or not.
//====================================================================
bool testWord(string input,string table[ROWS][COLS])
{
	bool verified = false; // word is verified or not
	stack <string> parseStack; // stack to push table entries
	int index = 0; //current letter in word
	input += "$"; // concatenate '$' to words for testing

	string state = ""; // table entry result
	string rowString = ""; //string representation of row being used
	string colString = ""; //string representation of column being used
	string prevState = ""; // needed for implementing functionality of rule states 

	parseStack.push("0"); // INITIALIZE STACK
	cout << "\nPush: 0\n";
	displayStack(parseStack);
	cout << "\nNow reading " << input[index] <<"\n";
	cout <<"\nRemaining Input: ";
	for (int i = index+1; i < input.length(); ++i)
			cout << input[i];
	cout << endl;
	state =  aquireTableEntry(parseStack, input, index, table,colString, rowString,prevState); // aquire first state
	handleTableEntry(parseStack,state,colString, rowString,input,index,table); // handle first state
	while (index < input.length() && state != "acc") //continue algorithm until end of input string or "acc" is reached
	{
		// Check table entry for empty cell (~).
		if( !isValidTableEntry(state))
		{
			cout << "\nEmpty table entry detected, invalid word. . .\n";
			//exit loop
			index = input.length();
		}
		else  // handle entry
		{
			state = aquireTableEntry(parseStack,input,index,table, colString,rowString,state);
			handleTableEntry(parseStack,state,colString, rowString,input,index,table);
			displayStack(parseStack);
		}
	}// end of while loop
	if (state == "acc")
		{
			verified = true;
			while(!parseStack.empty())
				parseStack.pop();
		}
	else {};
	return verified;
} // end of bool testWord()
//====================================================================

// ======= aquireRow() ========================================================
// Prevalue: string stack, string containing the current row
//
// Postvalue: updates row needed for next table entry
//
// Description: updates a string with the value of the current row.
//=====================================================================================
int aquireRow(stack <string> & parseStack, string & rowString)
{
	int row = 0;
	int rowInt= 0;
	stringstream temp; // stream to cast top of parse stack to integer
	temp.str(parseStack.top());
	temp >> rowInt;

	switch(rowInt)
	{
	case 0: row = 0; rowString = "0";
		break;
	case 1: row = 1; rowString = "1";
		break;
	case 2: row = 2; rowString = "2";
		break;
	case 3: row = 3; rowString = "3";
		break;
	case 4: row = 4; rowString = "4";
		break;
	case 5: row = 5; rowString = "5";
		break;
	case 6: row = 6; rowString = "6";
		break;
	case 7: row = 7; rowString = "7";
		break;
	case 8: row = 8; rowString = "8";
		break;
	case 9: row = 9; rowString = "9";
		break;
	case 10: row = 10; rowString ="10";
		break;
	case 11: row =11; rowString = "11";
		break;
	case 12: row = 12; rowString = "12";
		break;
	case 13: row = 13; rowString = "13";
		break;
	case 14: row = 14; rowString = "14";
		break;
	case 15: row = 15; rowString = "15";
		break;
	}
	return row;
} // end of aquireRow()
// ============================================================================

// ======= aquireTableEntry() ========================================
// Prevalue: string stack, string containing test input, 2d string array representing parse table
//		
//
// Postvalue: string containing resulting table entry (state).
//
// Description: This function separates the means of aquiring the column entry based on the previous
//		state. This dunction will then return the value of the table entry (state) based on the 
//		results of aquireCol and aquireRow.
//====================================================================
string aquireTableEntry (stack<string>& parseStack, string input, int & index, string table [ROWS][COLS], string & colString, string & rowString,string state)
{
	unsigned int row = NULL;
	unsigned int col = NULL; 
	
	string prevState = state;
	if (prevState == "") // no previous state
	{
		col = aquireColFromInput(  input,  index,colString );
		row = aquireRow(parseStack,rowString);
		
		cout << "POP: " << parseStack.top();
		parseStack.pop();
	}
	else
	{
		if(prevState[0] == 'r') // if a rule state was the previous state
		{
			col =  aquireColFromRule ( colString,prevState);
			
			row = aquireRow(parseStack,rowString);
		//	cout << "\ncol: " << col << "\trow: " << row << endl;
			cout << "POP: " << parseStack.top();
			parseStack.pop();
		}
		else
		{
			col = aquireColFromInput ( input,  index,colString );
			row = aquireRow(parseStack,rowString);
		
			cout << "\nPOP: " << parseStack.top();
				parseStack.pop();
		}
	}
	cout << "\nTable entry [" << rowString  << "," << colString << "] = "
		<< table[row][col] << endl;
	return table[row][col];
}// end of aquireTableEntry()
//====================================================================


// ======= isValidTableEntry() =============================
// Prevalue: string containing current table entry
//
// Postvalue: boolean value depending on if table entry is non-empty. 
//
// Description: Makes sure that table entry is not an empty entry
//========================================================
bool isValidTableEntry(string state)
{
	if (state == "~")
		return false;
	else return true;
}// end of isValidTableEntry()
//====================================================================


// ======= handleTableEntry() ========================================================
// Prevalue: state aquried from table
//
// Postvalue: boolean describing if the table entry is empty or not.
//
// Description: differentiates between type of table entry (shift,rule,constant)
//		and excecutes proper command.
//=====================================================================================
void handleTableEntry(stack <string> & parseStack, string  state, string & colString, string rowString,string input, int & index, string table [ROWS][COLS])
{

	if (state[0] == 's')
		handleShift(parseStack,state,colString,rowString,input,index);

	else if (state[0] == 'r')
		handleRule(parseStack,state,rowString);

	else if ((state[0] == '1')||(state[0] == '2')||( state[0] == '3')) // table entry is digit so push back
		handleConstant(parseStack,state,colString,rowString);
}// end of handleTableEntry()
//====================================================================



// ======= displayStack() ============================================================
// Prevalue: current state of string stack
//
// Postvalue: Contents of stack output to screen
//
// Description: Displays contents of stack passed as argument
//=====================================================================================
void displayStack(stack <string> myStack)
{
	cout << "  Top of stack <-- ";
	if (!myStack.empty())
	{
		for (stack <string> temp = myStack; !temp.empty(); temp.pop())
			cout << temp.top() <<  " " ;
		cout << endl;
	}
	else
		cout << "*Empty*\n";
}// end of displayStack
//====================================================================

// =======readNextInput() ============================================================
// Prevalue: current index value
//
// Postvalue: index incremented by one
//
// Description: read next character in input string
//=====================================================================================
void readNextInput(int & index, string input)
{
		//cout << "current index = " << index;
		index++;
		//cout << "\tNew index = " << index << endl;
		cout << "\n\tNow reading " << input[index];
			
		cout <<"\nRemaining Input: ";
		if (index == input.length())
			cout << "None\n";
		else
		{
			for (int i = index+1; i < input.length(); ++i)
				cout << input[i];
			cout << endl;
		}
}// end of readNextInput
// ======= handleShift() ========================================================
// Prevalue: string stack, current state, the value of the current row and column, 
//			the input string, the current index of the input string
//
// Postvalue: the string stack will have 3 values pushed onto it and index will
//			be incremented
//
// Description: This function handles the behavior of a shift when a certain value is 
//				obtained from the parse table. This behavior includes pushing the state
//			number, the row, and then the column onto the stack. The next character in the
//			input string will now be used for aquiring subsequent table entries (index++). 
//=====================================================================================
void handleShift(stack <string> & parseStack, string & state, string colString, string rowString ,string input, int & index)
{
	cout <<"SHIFT, Enter state: ";
		for (int i = 1; i < state.length(); i ++)
			cout << state[i];

		if (state == "s4")
		{
			cout << "\nPUSH " << rowString << " " << colString << " 4 \n";
			parseStack.push(rowString);
			parseStack.push(colString);
			parseStack.push("4");
			displayStack(parseStack);
			readNextInput(index,input);
		}
		else if (state == "s5")
		{
			cout << "\nPUSH " << rowString << " " << colString << " 5 \n";
			parseStack.push(rowString);
			parseStack.push(colString);
			parseStack.push("5");
			displayStack(parseStack);
			readNextInput(index,input);
		}
		else if (state == "s6")
		{
			cout << "\nPUSH " << rowString << " " << colString << " 6 \n";
			parseStack.push(rowString);
			parseStack.push(colString);
			parseStack.push("6");
			displayStack(parseStack);
			readNextInput(index,input);
		}
		else if (state == "s7")
		{
			cout << "\nPUSH " << rowString << " " << colString << " 7 \n";
			parseStack.push(rowString);
			parseStack.push(colString);
			parseStack.push("7");
			displayStack(parseStack);
			readNextInput(index,input);
		}
		else if (state == "s8")
		{
			cout << "\nPUSH " << rowString << " " << colString << " 8 \n";
			parseStack.push(rowString);
			parseStack.push(colString);
			parseStack.push("8");
			displayStack(parseStack);
			readNextInput(index,input);
		}
		else if (state == "s9")
		{
			cout << "\nPUSH " << rowString << " " << colString << " 9 \n";
			parseStack.push(rowString);
			parseStack.push(colString);
			parseStack.push("9");
			displayStack(parseStack);
			readNextInput(index,input);
		}
		else if (state == "s15")
		{
			cout << "\nPUSH " << rowString << " " << colString << " 15 \n";
			parseStack.push(rowString);
			parseStack.push(colString);
			parseStack.push("15");
			displayStack(parseStack);
			readNextInput(index,input);
		}
}// end of handleShift()
// ============================================================================

// ======= handleRule() ========================================================
// Prevalue:The current state from table	
//
// Postvalue: updated stack and column value.
//
// Description: This function handles a rule table entry. This is done by pushing back the 
//			state number and then popping 2 times the length of the right side
//			of rule elements. Another table entry is then obtained by using the left
//			side of the rule in the next state aquisition.
//=====================================================================================
void handleRule(stack <string> & parseStack, string & state, string rowString)
{

	string rule1 = "EE+T"; // E -> E+T
	string rule2 = "EE-T"; // E -> E-T
	string rule3 = "ET";   // E -> T
	string rule4 = "TT*F"; // T -> T*F
	string rule5 = "TT/F"; // T -> T/F
	string rule6 = "TF";   // T -> F
	string rule7 = "F(E)"; // F -> (E)
	string rule8 = "Fi";   // F -> i

	cout << "Rule ";
		for (int i = 1; i < state.length(); i ++)
			cout << state[i];
		cout << ":\t";
			
	if (state == "r1")
		{
			cout << rule1[0] << " -> " ;
			for (int i = 1; i < rule1.length(); i ++)
				cout << rule1[i]<< " ";
			cout <<endl;
			cout << "PUSH " << rowString << " \n";
			parseStack.push(rowString);
			displayStack(parseStack);
			cout << "POP " << (2* rule1.length() - 2) << " elements, POP: ";
			for ( int i = 1; i < rule1.length(); i++)
			{
			cout << parseStack.top() << " ";
				parseStack.pop();
			cout << parseStack.top() << " ";
				parseStack.pop();
			}
			cout << endl ;
		}
		else if (state == "r2")
		{
			cout << rule2[0] << " -> " ;
			for (int i = 1; i < rule2.length(); i ++)
				cout << rule2[i]<< " ";
			cout <<endl;
			cout << "PUSH " << rowString << " \n";
			parseStack.push(rowString);
			displayStack(parseStack);
			cout << "POP " << (2* rule2.length() - 2) << " elements, POP: ";
			for ( int i = 1; i < rule2.length(); i++)
			{
			cout << parseStack.top() << " ";
				parseStack.pop();
			cout << parseStack.top() << " ";
				parseStack.pop();
			}
			cout << endl ;
		}
		else if (state == "r3")
		{
			cout << rule3[0] << " -> " ;
			for (int i = 1; i < rule3.length(); i ++)
				cout << rule3[i]<< " ";
			cout <<endl;
			cout << "PUSH " << rowString << " \n";
			parseStack.push(rowString);
			displayStack(parseStack);
			cout << "POP " << (2* rule3.length() - 2) << " elements, POP: ";
			for ( int i = 1; i < rule3.length(); i++)
			{
			cout << parseStack.top() << " ";
				parseStack.pop();
			cout << parseStack.top() << " ";
				parseStack.pop();
			}
			cout << endl ;
		}
		else if (state == "r4")
		{
			cout << rule4[0] << " -> " ;
			for (int i = 1; i < rule4.length(); i ++)
				cout << rule4[i]<< " ";
			cout <<endl;
			cout << "PUSH " << rowString << " \n";
			parseStack.push(rowString);
			displayStack(parseStack);
			cout << "POP " << (2* rule4.length() - 2) << " elements, POP: ";
			for ( int i = 1; i < rule4.length(); i++)
			{
			cout << parseStack.top() << " ";
				parseStack.pop();
			cout << parseStack.top() << " ";
				parseStack.pop();
			}
			cout << endl ;
		}
		else if (state == "r5")
		{
			cout << rule5[0] << " -> " ;
			for (int i = 1; i < rule5.length(); i ++)
				cout << rule5[i]<< " ";
			cout <<endl;
			cout << "PUSH " << rowString << " \n";
			parseStack.push(rowString);
			displayStack(parseStack);
			cout << "POP " << (2* rule5.length() - 2) << " elements, POP: ";
			for ( int i = 1; i < rule5.length(); i++)
			{
			cout << parseStack.top() << " ";
				parseStack.pop();
			cout << parseStack.top() << " ";
				parseStack.pop();
			}
			cout << endl ;
		}
		else if (state == "r6")
		{
			cout << rule6[0] << " -> " ;
			for (int i = 1; i < rule6.length(); i ++)
				cout << rule6[i]<< " ";
			cout <<endl;
			cout << "PUSH " << rowString << " \n";
			parseStack.push(rowString);
			displayStack(parseStack);
			cout << "POP " << (2* rule6.length() - 2) << " elements, POP: ";
			for ( int i = 1; i < rule6.length(); i++)
			{
			cout << parseStack.top() << " ";
				parseStack.pop();
			cout << parseStack.top() << " ";
				parseStack.pop();
			}
			cout << endl;
		}
		else if (state == "r7")
		{
			cout << rule7[0] << " -> " ;
			for (int i = 1; i < rule7.length(); i ++)
				cout << rule7[i]<< " ";
			cout <<endl;
			cout << "PUSH " << rowString << " \n";
			parseStack.push(rowString);
			displayStack(parseStack);
			cout << "POP " << (2* rule7.length() - 2) << " elements, POP: ";
			for ( int i = 1; i < rule7.length(); i++)
			{
			cout << parseStack.top() << " ";
				parseStack.pop();
			cout << parseStack.top() << " ";
				parseStack.pop();
			}
			cout << endl;
		}
		else if (state == "r8")
		{
			cout << rule8[0] << " -> " ;
			for (int i = 1; i < rule8.length(); i ++)
				cout << rule8[i]<< " ";
			cout <<endl;
			cout << "PUSH " << rowString << " \n";
			parseStack.push(rowString);
			displayStack(parseStack);
			cout << "POP " << (2* rule8.length() - 2) << " elements, POP: ";
			for ( int i = 1; i < rule8.length(); i++)
			{
			cout << parseStack.top() << " ";
				parseStack.pop();
			cout << parseStack.top() << " ";
				parseStack.pop();
			}
			cout << endl;
		}
}// end of handleRule
// =============================================================================

// ======= handleConstant() ========================================================
// Prevalue: 
//
// Postvalue: 
//
// Description: pushes back the row, the column, and the state back onto the stack.
//=====================================================================================
void handleConstant(stack <string> & parseStack, string  state, string colString, string rowString)
{
	cout << "PUSH " << rowString <<" " << colString<< " "  << state << " \n";
		parseStack.push(rowString);
		parseStack.push(colString);
		parseStack.push(state);
}// end of handleConstant()
// =========================================================================================

// ======= aquireColFromInput() ========================================================
// Prevalue: 
//
// Postvalue: 
//
// Description: this function uses the input string as the column value and will
//			return the proper column value.
//=====================================================================================
int aquireColFromInput (string input, int index, string & colString)
{
	int col = 0;

	switch (input[index])
	{
		case 'i': col = 0; colString = "i"; 
			break;
		case '+': col = 1; colString = "+";
			break;
		case '-': col = 2; colString = "-";
			break;
		case '*': col = 3; colString = "*";
			break;
		case '/': col = 4; colString = "/";
			break;
		case '(': col = 5; colString = "(";
			break;
		case ')': col = 6; colString = ")";
			break;
		case '$': col = 7; colString = "$";
			break;		
		case 'E': col = 8; colString = "E";
			break;
		case 'T': col = 9; colString = "T";
			break;
		case 'F': col = 10; colString = "F";
			break;
	}

	return col;		
} // end of aquireColFromInput
// ===================================================================================

// ======= aqureColFromRule() ========================================================
// Prevalue: char stack object and a string containing the current table entry
//
// Postvalue: 
//
// Description: This function uses the left side of a rule production as the basis for
//			obtaining the column value. This is used if the table entry results in a 
//			"rule".
//=====================================================================================
int aquireColFromRule (string & colString, string state)
{
	int col = 0;
	char production = 0;

	if (state[1] == '1'||state[1] == '2' ||state[1] == '3')
		production = 'E';

	else if (state[1] == '4'||state[1] == '5' ||state[1] == '6')
		production = 'T';

	else
		production = 'F';

	switch (production)
		{
		case 'E': col = 8; colString = "E";
			break;
		case 'T': col = 9; colString = "T";
			break;
		case 'F': col = 10; colString = "F";
			break;
		}	
	return col;
} // end of aquireColFromRule
// ==================================================================================

// ======= displayLocalTime() ========================================================
// Prevalue: none
//
// Postvalue: displays local time
//
// Description: Uses strftime function contained in ctime library to display the local
//time
//=====================================================================================
void displayLocalTime()
{
	const int BUFFSIZE=200; // buffer for tmdesc[] which is used to format the time
	// set up to use strftime()
	time_t myTime = time(0); //obtain the current time_t value
	tm now= *localtime(& myTime); //convert to tm, a struct defined in ctime library
	char tmdesc[BUFFSIZE] ={0}; //ensures finite characters for displaying time output
	// output statement for tm struct
	// strftime() token reference: http://en.cppreference.com/w/cpp/chrono/c/strftime
	const char fmt[] = "Current local time and date:\n%A, %B %d %Y \n%I:%M:%S %p %Z";
	// Display today's date and time
	if (strftime(tmdesc, sizeof(tmdesc)-1, fmt, &now) >0)
		cout << tmdesc << endl <<endl ;
	else
		cerr<<"strftime failed. Errno code: " <<errno << endl;
}// end of displayLocalTime()
//====================================================================
/* OUTPUT

****************************************************

Current local time and date:
Monday, April 20 2015
04:44:23 PM Pacific Daylight Time

        **LR PARSER**

note: ~ represents empty.

Word being tested: (i+i)*i

Push: 0
  Top of stack <-- 0

Now reading (

Remaining Input: i+i)*i$
POP: 0
Table entry [0,(] = s4
SHIFT, Enter state: 4
PUSH 0 ( 4
  Top of stack <-- 4 ( 0

        Now reading i
Remaining Input: +i)*i$

POP: 4
Table entry [4,i] = s5
SHIFT, Enter state: 5
PUSH 4 i 5
  Top of stack <-- 5 i 4 ( 0

        Now reading +
Remaining Input: i)*i$
  Top of stack <-- 5 i 4 ( 0

POP: 5
Table entry [5,+] = r8
Rule 8: F -> i
PUSH 5
  Top of stack <-- 5 i 4 ( 0
POP 2 elements, POP: 5 i
  Top of stack <-- 4 ( 0
POP: 4
Table entry [4,F] = 3
PUSH 4 F 3
  Top of stack <-- 3 F 4 ( 0

POP: 3
Table entry [3,+] = r6
Rule 6: T -> F
PUSH 3
  Top of stack <-- 3 F 4 ( 0
POP 2 elements, POP: 3 F
  Top of stack <-- 4 ( 0
POP: 4
Table entry [4,T] = 2
PUSH 4 T 2
  Top of stack <-- 2 T 4 ( 0

POP: 2
Table entry [2,+] = r3
Rule 3: E -> T
PUSH 2
  Top of stack <-- 2 T 4 ( 0
POP 2 elements, POP: 2 T
  Top of stack <-- 4 ( 0
POP: 4
Table entry [4,E] = 10
PUSH 4 E 10
  Top of stack <-- 10 E 4 ( 0

POP: 10
Table entry [10,+] = s6
SHIFT, Enter state: 6
PUSH 10 + 6
  Top of stack <-- 6 + 10 E 4 ( 0

        Now reading i
Remaining Input: )*i$
  Top of stack <-- 6 + 10 E 4 ( 0

POP: 6
Table entry [6,i] = s5
SHIFT, Enter state: 5
PUSH 6 i 5
  Top of stack <-- 5 i 6 + 10 E 4 ( 0

        Now reading )
Remaining Input: *i$
  Top of stack <-- 5 i 6 + 10 E 4 ( 0

POP: 5
Table entry [5,)] = r8
Rule 8: F -> i
PUSH 5
  Top of stack <-- 5 i 6 + 10 E 4 ( 0
POP 2 elements, POP: 5 i
  Top of stack <-- 6 + 10 E 4 ( 0
POP: 6
Table entry [6,F] = 3
PUSH 6 F 3
  Top of stack <-- 3 F 6 + 10 E 4 ( 0

POP: 3
Table entry [3,)] = r6
Rule 6: T -> F
PUSH 3
  Top of stack <-- 3 F 6 + 10 E 4 ( 0
POP 2 elements, POP: 3 F
  Top of stack <-- 6 + 10 E 4 ( 0
POP: 6
Table entry [6,T] = 11
PUSH 6 T 11
  Top of stack <-- 11 T 6 + 10 E 4 ( 0

POP: 11
Table entry [11,)] = r1
Rule 1: E -> E + T
PUSH 11
  Top of stack <-- 11 T 6 + 10 E 4 ( 0
POP 6 elements, POP: 11 T 6 + 10 E
  Top of stack <-- 4 ( 0
POP: 4
Table entry [4,E] = 10
PUSH 4 E 10
  Top of stack <-- 10 E 4 ( 0

POP: 10
Table entry [10,)] = s15
SHIFT, Enter state: 15
PUSH 10 ) 15
  Top of stack <-- 15 ) 10 E 4 ( 0

        Now reading *
Remaining Input: i$
  Top of stack <-- 15 ) 10 E 4 ( 0

POP: 15
Table entry [15,*] = r7
Rule 7: F -> ( E )
PUSH 15
  Top of stack <-- 15 ) 10 E 4 ( 0
POP 6 elements, POP: 15 ) 10 E 4 (
  Top of stack <-- 0
POP: 0
Table entry [0,F] = 3
PUSH 0 F 3
  Top of stack <-- 3 F 0

POP: 3
Table entry [3,*] = r6
Rule 6: T -> F
PUSH 3
  Top of stack <-- 3 F 0
POP 2 elements, POP: 3 F
  Top of stack <-- 0
POP: 0
Table entry [0,T] = 2
PUSH 0 T 2
  Top of stack <-- 2 T 0

POP: 2
Table entry [2,*] = s8
SHIFT, Enter state: 8
PUSH 2 * 8
  Top of stack <-- 8 * 2 T 0

        Now reading i
Remaining Input: $
  Top of stack <-- 8 * 2 T 0

POP: 8
Table entry [8,i] = s5
SHIFT, Enter state: 5
PUSH 8 i 5
  Top of stack <-- 5 i 8 * 2 T 0

        Now reading $
Remaining Input:
  Top of stack <-- 5 i 8 * 2 T 0

POP: 5
Table entry [5,$] = r8
Rule 8: F -> i
PUSH 5
  Top of stack <-- 5 i 8 * 2 T 0
POP 2 elements, POP: 5 i
  Top of stack <-- 8 * 2 T 0
POP: 8
Table entry [8,F] = 13
PUSH 8 F 13
  Top of stack <-- 13 F 8 * 2 T 0

POP: 13
Table entry [13,$] = r4
Rule 4: T -> T * F
PUSH 13
  Top of stack <-- 13 F 8 * 2 T 0
POP 6 elements, POP: 13 F 8 * 2 T
  Top of stack <-- 0
POP: 0
Table entry [0,T] = 2
PUSH 0 T 2
  Top of stack <-- 2 T 0

POP: 2
Table entry [2,$] = r3
Rule 3: E -> T
PUSH 2
  Top of stack <-- 2 T 0
POP 2 elements, POP: 2 T
  Top of stack <-- 0
POP: 0
Table entry [0,E] = 1
PUSH 0 E 1
  Top of stack <-- 1 E 0

POP: 1
Table entry [1,$] = acc
  Top of stack <-- E 0
The word (i+i)*i is accepted
****************************************************

Word being tested: (i*)

Push: 0
  Top of stack <-- 0

Now reading (

Remaining Input: i*)$
POP: 0
Table entry [0,(] = s4
SHIFT, Enter state: 4
PUSH 0 ( 4
  Top of stack <-- 4 ( 0

        Now reading i
Remaining Input: *)$

POP: 4
Table entry [4,i] = s5
SHIFT, Enter state: 5
PUSH 4 i 5
  Top of stack <-- 5 i 4 ( 0

        Now reading *
Remaining Input: )$
  Top of stack <-- 5 i 4 ( 0

POP: 5
Table entry [5,*] = r8
Rule 8: F -> i
PUSH 5
  Top of stack <-- 5 i 4 ( 0
POP 2 elements, POP: 5 i
  Top of stack <-- 4 ( 0
POP: 4
Table entry [4,F] = 3
PUSH 4 F 3
  Top of stack <-- 3 F 4 ( 0

POP: 3
Table entry [3,*] = r6
Rule 6: T -> F
PUSH 3
  Top of stack <-- 3 F 4 ( 0
POP 2 elements, POP: 3 F
  Top of stack <-- 4 ( 0
POP: 4
Table entry [4,T] = 2
PUSH 4 T 2
  Top of stack <-- 2 T 4 ( 0

POP: 2
Table entry [2,*] = s8
SHIFT, Enter state: 8
PUSH 2 * 8
  Top of stack <-- 8 * 2 T 4 ( 0

        Now reading )
Remaining Input: $
  Top of stack <-- 8 * 2 T 4 ( 0

POP: 8
Table entry [8,)] = ~
  Top of stack <-- * 2 T 4 ( 0

Empty table entry detected, invalid word. . .
The word (i*) is not accepted
****************************************************

Press any key to continue . . .

// ============================= END OF ASSIGNMENT 9 ================================*/


/* Implementation of PLSL Interpreter
 * parseInt.cpp
 * Programming Assignment 3
 * Spring 2022
 */

#include <vector>
#include <string>
#include "parseInt.h"
#include "val.h"
#include "lex.cpp"
using namespace std;
map<string, bool> defVar;
map<string, Token> SymTable;
map<string, Value> TempsResults; // Container of temporary locations of Value objects for results of expressions, variables values and constants
queue<Value> *ValQue;			 // declare a pointer variable to a queue of Value objects

namespace Parser
{
	bool pushed_back = false;
	LexItem pushed_token;

	static LexItem GetNextToken(istream &in, int &line)
	{
		if (pushed_back)
		{
			pushed_back = false;
			return pushed_token;
		}
		return getNextToken(in, line);
	}

	static void PushBackToken(LexItem &t)
	{
		if (pushed_back)
		{
			abort();
		}
		pushed_back = true;
		pushed_token = t;
	}

}

static int error_count = 0;

int ErrCount()
{
	return error_count;
}

void ParseError(int line, string msg)
{
	++error_count;
	cout << line << ": " << msg << endl;
}

bool IdentList(istream &in, int &line)
{
	bool status = false;
	string identstr;

	LexItem tok = Parser::GetNextToken(in, line);
	if (tok == IDENT)
	{
		// set IDENT lexeme to the type tok value
		identstr = tok.GetLexeme();
		if (!(defVar.find(identstr)->second))
		{
			defVar[identstr] = true;
		}
		else
		{
			ParseError(line, "Variable Redefinition");
			return false;
		}
	}
	else
	{
		Parser::PushBackToken(tok);
		return true;
	}

	tok = Parser::GetNextToken(in, line);

	if (tok == COMMA)
	{
		status = IdentList(in, line);
	}
	else if (tok == COLON)
	{
		Parser::PushBackToken(tok);
		return true;
	}
	else
	{
		ParseError(line, "Unrecognized Input Pattern");
		cout << "(" << tok.GetLexeme() << ")" << endl;
		return false;
	}
	return status;

} // End of IdentList

// Program is: Prog = PROGRAM IDENT {Decl} {Stmt} END PROGRAM IDENT
bool Prog(istream &in, int &line)
{
	bool f1, f2;
	LexItem tok = Parser::GetNextToken(in, line);

	if (tok.GetToken() == PROGRAM)
	{
		tok = Parser::GetNextToken(in, line);
		if (tok.GetToken() == IDENT)
		{

			tok = Parser::GetNextToken(in, line);
			if (tok.GetToken() == SEMICOL)
			{
				f1 = DeclBlock(in, line);

				if (f1)
				{
					f2 = ProgBody(in, line);
					if (!f2)
					{
						ParseError(line, "Incorrect Program Body.");
						return false;
					}

					return true; // Successful Parsing is completed
				}
				else
				{
					ParseError(line, "Incorrect Declaration Section.");
					return false;
				}
			}
			else
			{
				// Parser::PushBackToken(tok);
				ParseError(line - 1, "Missing Semicolon.");
				return false;
			}
		}
		else
		{
			ParseError(line, "Missing Program Name.");
			return false;
		}
	}
	else if (tok.GetToken() == ERR)
	{
		ParseError(line, "Unrecognized Input Pattern");
		cout << "(" << tok.GetLexeme() << ")" << endl;
		return false;
	}
	else if (tok.GetToken() == DONE && tok.GetLinenum() <= 1)
	{
		ParseError(line, "Empty File");
		return true;
	}
	ParseError(line, "Missing PROGRAM.");
	return false;
}

bool DeclBlock(istream &in, int &line)
{
	bool status = false;
	LexItem tok;
	// cout << "in Decl" << endl;
	LexItem t = Parser::GetNextToken(in, line);
	if (t == VAR)
	{
		status = DeclStmt(in, line);

		while (status)
		{
			tok = Parser::GetNextToken(in, line);
			if (tok != SEMICOL)
			{
				line--;
				ParseError(line, "Missing semicolon in Declaration Statement.");
				return false;
			}
			status = DeclStmt(in, line);
		}

		tok = Parser::GetNextToken(in, line);
		if (tok == BEGIN)
		{
			Parser::PushBackToken(tok);
			return true;
		}
		else
		{
			ParseError(line, "Syntactic error in Declaration Block.");
			return false;
		}
	}
	else
	{
		ParseError(line, "Non-recognizable Declaration Block.");
		return false;
	}

} // end of DeclBlock function

bool DeclStmt(istream &in, int &line)
{
	LexItem t;
	bool status = IdentList(in, line);

	if (!status)
	{
		ParseError(line, "Incorrect variable in Declaration Statement.");
		return status;
	}

	t = Parser::GetNextToken(in, line);
	if (t == COLON)
	{
		t = Parser::GetNextToken(in, line);
		if (t == INTEGER || t == REAL || t == STRING)
		{
			map<string, bool>::iterator it;
			for (it = defVar.begin(); it != defVar.end(); it++)
			{
				if (it->second)
				{
					if (t == INTEGER)
					{
						SymTable.insert({it->first, INTEGER});
					}
					else if (t == STRING)
					{
						SymTable.insert({it->first, STRING});
					}
					else if (t == REAL)
					{
						SymTable.insert({it->first, REAL});
					}
					it->second = false;
				}
			}

			return true;
		}
		else
		{
			ParseError(line, "Incorrect Declaration Type.");
			return false;
		}
	}
	else
	{
		Parser::PushBackToken(t);
		return false;
	}

} // End of DeclStmt

bool ProgBody(istream &in, int &line)
{
	bool status;

	LexItem tok = Parser::GetNextToken(in, line);

	if (tok.GetToken() == BEGIN)
	{

		status = Stmt(in, line);

		while (status)
		{
			tok = Parser::GetNextToken(in, line);
			if (tok != SEMICOL)
			{
				line--;
				ParseError(line, "Missing semicolon in Statement.");
				return false;
			}

			status = Stmt(in, line);
		}

		tok = Parser::GetNextToken(in, line);
		if (tok == END)
		{
			return true;
		}
		else
		{
			ParseError(line, "Syntactic error in Program Body.");
			return false;
		}
	}
	else
	{
		ParseError(line, "Non-recognizable Program Body.");
		return false;
	}
} // End of ProgBody function

bool Stmt(istream &in, int &line)
{
	bool status;
	// cout << "in ContrlStmt" << endl;
	LexItem t = Parser::GetNextToken(in, line);

	switch (t.GetToken())
	{

	case WRITELN:
		// cout << "entering writeln: " << t.GetLexeme() << endl;

		status = WriteLnStmt(in, line);
		// cout << "exiting writeln: " << t.GetLexeme() << endl;

		// cout << "After WriteStmet status: " << (status? true:false) <<endl;
		break;

	case IF:
		// cout << "entering if: " << t.GetLexeme() << endl;

		status = IfStmt(in, line);
		// cout << "exiting if: " << t.GetLexeme() << endl;

		break;

	case IDENT:
		Parser::PushBackToken(t);
		// cout << "entering assig: " << t.GetLexeme() << endl;

		status = AssignStmt(in, line);
		// cout << "exiting assig: " << t.GetLexeme() << endl;

		break;

	default:
		Parser::PushBackToken(t);
		return false;
	}

	return status;
} // End of Stmt

// WriteStmt:= wi, ExpreList
bool WriteLnStmt(istream &in, int &line)
{
	LexItem t;
	// cout << "in WriteStmt" << endl;
	ValQue = new queue<Value>;
	t = Parser::GetNextToken(in, line);
	if (t != LPAREN)
	{

		ParseError(line, "Missing Left Parenthesis");
		return false;
	}

	bool ex = ExprList(in, line);

	if (!ex)
	{
		ParseError(line, "Missing expression after WriteLn");
		return false;
	}

	t = Parser::GetNextToken(in, line);
	if (t != RPAREN)
	{

		ParseError(line, "Missing Right Parenthesis");
		return false;
	}

	// Evaluate: print out the list of expressions' values
	while (!(*ValQue).empty())
	{
		Value nextVal = (*ValQue).front();
		cout << nextVal;
		ValQue->pop();
	}
	cout << endl;

	return ex;
} // End of WriteLnStmt

// IfStmt:= if (Expr) then Stm} [Else Stmt]
bool IfStmt(istream &in, int &line)
{
	bool ex = false, status;
	LexItem t;

	t = Parser::GetNextToken(in, line);
	if (t != LPAREN)
	{

		ParseError(line, "Missing Left Parenthesis");
		return false;
	}

	Value logexResult;
	ex = LogicExpr(in, line, logexResult);
	if (!ex)
	{
		ParseError(line, "Missing if statement Logic Expression");
		return false;
	}

	t = Parser::GetNextToken(in, line);
	if (t != RPAREN)
	{

		ParseError(line, "Missing Right Parenthesis");
		return false;
	}

	t = Parser::GetNextToken(in, line);
	if (t != THEN)
	{
		ParseError(line, "If-Stmt Syntax Error");
		return false;
	}
	if (logexResult.GetBool())
	{
		status = Stmt(in, line);
		// cout << "in if stmt" << endl;
		if (!status)
		{
			ParseError(line, "Missing Statement for If-Stmt Then-Part");
			return false;
		}
		// cout << "out if stmt" << endl;
		t = Parser::GetNextToken(in, line);
		if (t == ELSE)
		{
			while (t != SEMICOL)
			{
				t = Parser::GetNextToken(in, line);
			}
		}
	}
	else
	{
		while (t != ELSE && t != SEMICOL)
		{
			t = Parser::GetNextToken(in, line);
		}
		if (t == ELSE)
		{
			// cout << "in else stmt" << endl;
			status = Stmt(in, line);
			if (!status)
			{
				ParseError(line, "Missing Statement for If-Stmt Else-Part");
				return false;
			}
			t = Parser::GetNextToken(in, line);
		}
	}
	Parser::PushBackToken(t);

	return true;
} // End of IfStmt function

// AssignStmt:= Var = Expr
bool AssignStmt(istream &in, int &line)
{
	Value val;
	LexItem varLex;
	bool varstatus = false, status = false;
	LexItem t;

	varstatus = Var(in, line, varLex);

	if (varstatus)
	{
		t = Parser::GetNextToken(in, line);

		if (t == ASSOP)
		{
			status = Expr(in, line, val);
			if (!status)
			{
				ParseError(line, "Missing Expression in Assignment Statment");
				return false;
			}
		}
		else if (t.GetToken() == ERR)
		{
			ParseError(line, "Unrecognized Input Pattern");
			cout << "(" << t.GetLexeme() << ")" << endl;
			return false;
		}
		else
		{
			ParseError(line, "Missing Assignment Operator");
			return false;
		}
	}
	else
	{
		ParseError(line, "Missing Left-Hand Side Variable in Assignment statement");
		return false;
	}
	string varName = varLex.GetLexeme();
	Token varType = SymTable.find(varName)->second;

	if (varType == STRING)
	{
		if (val.GetType() == VSTRING)
		{
			if (TempsResults.find(varName) != TempsResults.end())
			{
				TempsResults[varName] = val;
			}
			else
			{
				TempsResults.insert({varLex.GetLexeme(), val});
			}
		}
		else
		{
			ParseError(line, "Incorrect value type assigned to variable");
			return false;
		}
	}
	else if (varType == INTEGER)
	{
		if (val.GetType() == VINT)
		{
			if (TempsResults.find(varName) != TempsResults.end())
			{
				TempsResults[varName] = val;
			}
			else
			{
				TempsResults.insert({varLex.GetLexeme(), val});
			}
		}
		else if (val.GetType() == VREAL)
		{
			int newVal = val.GetReal();
			if (TempsResults.find(varName) != TempsResults.end())
			{
				TempsResults[varName] = Value(newVal);
			}
			else
			{
				TempsResults.insert({varLex.GetLexeme(), Value(newVal)});
			}
		}
		else
		{
			ParseError(line, "Incorrect value type assigned to variable");
			return false;
		}
	}
	else if (varType == REAL)
	{
		if (val.GetType() == VREAL)
		{
			if (TempsResults.find(varName) != TempsResults.end())
			{
				TempsResults[varName] = val;
			}
			else
			{
				TempsResults.insert({varLex.GetLexeme(), val});
			}
		}
		else if (val.GetType() == VINT)
		{
			float newVal = val.GetInt();
			if (TempsResults.find(varName) != TempsResults.end())
			{
				TempsResults[varName] = Value(newVal);
			}
			else
			{
				TempsResults.insert({varLex.GetLexeme(), Value(newVal)});
			}
		}
		else
		{
			ParseError(line, "Incorrect value type assigned to variable");
			return false;
		}
	}
	else
	{
		ParseError(line, "somehow  variable type isn't one of the three");
		return false;
	}

	// cout << "exiting expr: " << t.GetLexeme() << endl;

	return true;
}

bool Var(istream &in, int &line, LexItem &idtok)
{
	LexItem t = Parser::GetNextToken(in, line);
	if (defVar.find(t.GetLexeme()) == defVar.end())
	{
		ParseError(line, "Variable not defined");
		return false;
	}
	else if (t.GetToken() == ERR)
	{
		ParseError(line, "Unrecognized Input Pattern");
		cout << "(" << t.GetLexeme() << ")" << endl;
		return false;
	}
	idtok = t;
	return true;
}

bool ExprList(istream &in, int &line)
{
	bool status = false;
	// cout << "in ExprList and before calling Expr" << endl;
	// cout << "entering expr: " << endl;
	Value val;
	status = Expr(in, line, val);
	if (!status)
	{
		ParseError(line, "Missing Expression");
		return false;
	}
	ValQue->push(val);
	// cout << "exiting expr: " << endl;

	LexItem tok = Parser::GetNextToken(in, line);

	if (tok == COMMA)
	{
		// cout << "before calling ExprList" << endl;
		status = ExprList(in, line);
		// cout << "after calling ExprList" << endl;
	}
	else if (tok.GetToken() == ERR)
	{
		ParseError(line, "Unrecognized Input Pattern");
		cout << "(" << tok.GetLexeme() << ")" << endl;
		return false;
	}
	else
	{
		Parser::PushBackToken(tok);
		return true;
	}
	return status;
}

// LogicExpr = Expr (== | <) Expr
bool LogicExpr(istream &in, int &line, Value &retVal)
{
	Value val1, val2;
	bool t1 = Expr(in, line, val1);
	LexItem tok;

	if (!t1)
	{
		return false;
	}

	tok = Parser::GetNextToken(in, line);
	if (tok.GetToken() == ERR)
	{
		ParseError(line, "Unrecognized Input Pattern");
		cout << "(" << tok.GetLexeme() << ")" << endl;
		return false;
	}
	if (tok == GTHAN || tok == EQUAL || tok == LTHAN)
	{
		t1 = Expr(in, line, val2);
		if (!t1)
		{
			ParseError(line, "Missing expression after relational operator");
			return false;
		}
		if (tok == EQUAL)
		{
			retVal = val1 == val2;
		}
		else if (tok == GTHAN)
		{
			retVal = val1 > val2;
		}
		else if (tok == LTHAN)
		{
			retVal = val1 < val2;
		}
		if (retVal.GetType() == VERR)
		{
			ParseError(line, "Illegal mixed-type operands");
			return false;
		}
		return true;
	}
	Parser::PushBackToken(tok);
	return true;
}

// Expr:= Term {(+|-) Term}
bool Expr(istream &in, int &line, Value &retVal)
{
	Value val1, val2;
	// cout << "in Expr" << endl;
	bool t1 = Term(in, line, val1);
	LexItem tok;

	if (!t1)
	{
		return false;
	}
	retVal = val1;

	tok = Parser::GetNextToken(in, line);
	if (tok.GetToken() == ERR)
	{
		ParseError(line, "Unrecognized Input Pattern");
		cout << "(" << tok.GetLexeme() << ")" << endl;
		return false;
	}
	// Evaluate: evaluate the expression for addition or subtraction
	while (tok == PLUS || tok == MINUS)
	{
		t1 = Term(in, line, val2);
		if (!t1)
		{
			ParseError(line, "Missing operand after operator");
			return false;
		}

		if (tok == PLUS)
		{
			retVal = retVal + val2;
			if (retVal.IsErr())
			{
				ParseError(line, "Illegal addition operation.");
				// cout << "(" << tok.GetLexeme() << ")" << endl;
				return false;
			}
		}
		else if (tok == MINUS)
		{
			retVal = retVal - val2;
			if (retVal.IsErr())
			{
				ParseError(line, "Illegal subtraction operation.");
				return false;
			}
		}

		tok = Parser::GetNextToken(in, line);
		if (tok.GetToken() == ERR)
		{
			ParseError(line, "Unrecognized Input Pattern");
			cout << "(" << tok.GetLexeme() << ")" << endl;
			return false;
		}
	}
	Parser::PushBackToken(tok);
	return true;
} // end of Expr

// Term:= SFactor {(*|/) SFactor}
bool Term(istream &in, int &line, Value &retVal)
{
	Value val1, val2;
	bool t1 = SFactor(in, line, val1);
	LexItem tok;

	if (!t1)
	{
		return false;
	}

	retVal = val1;

	tok = Parser::GetNextToken(in, line);
	if (tok.GetToken() == ERR)
	{
		ParseError(line, "Unrecognized Input Pattern");
		cout << "(" << tok.GetLexeme() << ")" << endl;
		return false;
	}
	while (tok == MULT || tok == DIV)
	{
		t1 = SFactor(in, line, val2);

		if (!t1)
		{
			ParseError(line, "Missing operand after operator");
			return false;
		}

		if (tok == MULT)
		{
			retVal = retVal * val2;
			if (retVal.GetType() == VERR)
			{
				ParseError(line, "Illegal multiplication operation");
				return false;
			}
		}
		else if (tok == DIV)
		{
			if (val2.GetType() == VINT)
			{
				if (val2.GetInt() == 0)
				{
					ParseError(line, "Illegal division by 0");
					return false;
				}
			}
			else if (val2.GetType() == VREAL)
			{
				if (val2.GetReal() == 0)
				{
					ParseError(line, "Illegal division by 0");
					return false;
				}
			}
			retVal = retVal / val2;
			if (retVal.GetType() == VERR)
			{
				ParseError(line, "Illegal division operation");
				return false;
			}
		}

		tok = Parser::GetNextToken(in, line);
		if (tok.GetToken() == ERR)
		{
			ParseError(line, "Unrecognized Input Pattern");
			cout << "(" << tok.GetLexeme() << ")" << endl;
			return false;
		}
	}
	Parser::PushBackToken(tok);
	return true;
}

bool SFactor(istream &in, int &line, Value &retVal)
{
	Value val;
	LexItem t = Parser::GetNextToken(in, line);
	bool status;
	int sign = 0;
	if (t == MINUS)
	{
		sign = -1;
	}
	else if (t == PLUS)
	{
		sign = 1;
	}
	else
		Parser::PushBackToken(t);

	status = Factor(in, line, sign, val);
	retVal = val;
	return status;
}

bool Factor(istream &in, int &line, int sign, Value &retVal)
{
	Value val;
	LexItem t = Parser::GetNextToken(in, line);
	// cout << "line: " << line << " lexeme " << t.GetLexeme() << " token " << t.GetToken() << endl;
	if (t == SCONST)
	{
		if (sign == 1 || sign == -1)
		{
			ParseError(line, "Invalid sign in front of string");
			return false;
		}
		retVal = Value(t.GetLexeme());
		return true;
	}
	else if (t == ICONST)
	{
		int v = stoi(t.GetLexeme());
		if (sign == -1)
		{
			v *= -1;
		}
		retVal = Value(v);
		return true;
	}
	else if (t == RCONST)
	{
		float v = stof(t.GetLexeme());
		if (sign == -1)
		{
			v *= -1;
		}
		retVal = Value(v);
		return true;
	}
	else if (t == IDENT)
	{
		// cout << "line: " << line << " Token: " << t.GetToken() << " lexeme: " << t.GetLexeme() << endl;
		if (TempsResults.find(t.GetLexeme()) == TempsResults.end())
		{
			ParseError(line, "Using Undefined Variable");
			return false;
		}
		Value v = TempsResults.find(t.GetLexeme())->second;
		if (v.GetType() == VINT)
		{
			int i = v.GetInt();
			if (sign == -1)
			{
				i *= -1;
			}
			retVal = Value(i);
		}
		else if (v.GetType() == VREAL)
		{
			float f = v.GetReal();
			if (sign == -1)
			{
				f *= -1;
			}
			retVal = Value(f);
		}
		else if (v.GetType() == VSTRING)
		{
			if (sign == -1)
			{
				ParseError(line, "Invalid sign in front of string");
				return false;
			}
			retVal = v;
		}
		else
		{
			ParseError(line, "Invalid identity");
			return false;
		}

		return true;
	}
	else if (t == LPAREN)
	{
		if (!Expr(in, line, val))
		{
			ParseError(line, "Invalid expression");
			return false;
		}
		t = Parser::GetNextToken(in, line);
		if (t != RPAREN)
		{
			Parser::PushBackToken(t);
			ParseError(line, "Missing parenthesis");
			return false;
		}
		retVal = val;
		return true;
	}
	else if (t.GetToken() == ERR)
	{
		ParseError(line, "Unrecognized Input Pattern");
		cout << "(" << t.GetLexeme() << ")" << endl;
		return false;
	}

	return false;
}

#include "lex.h"
#include <map>

bool identOver(char c)
{
	switch (c)
	{
	case ' ':
		return true;
	case '\n':
		return true;
	case ';':
		return true;
	case ',':
		return true;
	case '=':
		return true;
	case '+':
		return true;
	case '-':
		return true;
	case '/':
		return true;
	case '*':
		return true;
	case '>':
		return true;
	case '<':
		return true;
	}
	return false;
}

LexItem id_or_kw(const string &lexeme, int linenum)
{
	map<string, Token> m = {
		{"PROGRAM", PROGRAM}, {"WRITELN", WRITELN}, {"INTEGER", INTEGER}, {"BEGIN", BEGIN}, {"END", END}, {"IF", IF}, {"REAL", REAL}, {"STRING", STRING}, {"VAR", VAR}, {"ELSE", ELSE}, {"FOR", FOR}, {"THEN", THEN}, {"DO", DO}, {"TO", TO}, {"DOWNTO", DOWNTO}, {"IDENT", IDENT}, {"ICONST", ICONST}, {"RCONST", RCONST}, {"SCONST", SCONST}, {"PLUS", PLUS}, {"MINUS", MINUS}, {"MULT", MULT}, {"DIV", DIV}, {"ASSOP", ASSOP}, {"LPAREN", LPAREN}, {"RPAREN", RPAREN}, {"COMMA", COMMA}, {"EQUAL", EQUAL}, {"GTHAN", GTHAN}, {"LTHAN", LTHAN}, {"SEMICOL", SEMICOL}, {"COLON", COLON}, {"ERR", ERR}, {"DONE", DONE}};
	LexItem l;
	if (m.count(lexeme))
	{
		Token t = m[lexeme];
		l = LexItem(t, lexeme, linenum);
	}
	else
	{
		Token t = IDENT;
		l = LexItem(t, lexeme, linenum);
	}
	return l;
}

ostream &operator<<(ostream &out, const LexItem &tok)
{
	map<Token, string> m = {
		{PROGRAM, "PROGRAM"}, {WRITELN, "WRITELN"}, {INTEGER, "INTEGER"}, {BEGIN, "BEGIN"}, {END, "END"}, {IF, "IF"}, {REAL, "REAL"}, {STRING, "STRING"}, {VAR, "VAR"}, {ELSE, "ELSE"}, {FOR, "FOR"}, {THEN, "THEN"}, {DO, "DO"}, {TO, "TO"}, {DOWNTO, "DOWNTO"}, {IDENT, "IDENT"}, {ICONST, "ICONST"}, {RCONST, "RCONST"}, {SCONST, "SCONST"}, {PLUS, "PLUS"}, {MINUS, "MINUS"}, {MULT, "MULT"}, {DIV, "DIV"}, {ASSOP, "ASSOP"}, {LPAREN, "LPAREN"}, {RPAREN, "RPAREN"}, {COMMA, "COMMA"}, {EQUAL, "EQUAL"}, {GTHAN, "GTHAN"}, {LTHAN, "LTHAN"}, {SEMICOL, "SEMICOL"}, {COLON, "COLON"}, {ERR, "ERR"}, {DONE, "DONE"}};
	Token t = tok.GetToken();
	Token t1 = IDENT;
	Token t2 = ICONST;
	Token t3 = SCONST;
	Token t4 = RCONST;
	if (t == t1 || t == t2 || t == t3 || t == t4)
	{
		out << m[tok.GetToken()] << "(" << tok.GetLexeme() << ")" << std::endl;
	}
	else
	{
		out << m[t] << std::endl;
	}
	return out;
}

string allCap(string &s)
{
	for (int i = 0; i < s.length(); i++)
	{
		if (isalpha(s[i]))
		{
			s[i] = toupper(s[i]);
		}
	}
	return s;
}

LexItem getNextToken(istream &in, int &linenum)
{
	enum States
	{
		START,
		LIDENT,
		LINT,
		LSTRING,
		LCOMMENT,
		LREAL
	};
	States state = START;
	char c;
	string lex;
	char stringStart;

	while (in.get(c))
	{
		switch (state)
		{
		case START:
			if (c == '\n')
			{
				linenum++;
			}
			lex.clear();
			lex += c;
			if (isalpha(c) || c == '_')
			{
				state = LIDENT;
			}
			if (isdigit(c))
			{
				state = LINT;
			}
			if (c == '"' || c == '\'')
			{
				stringStart = c;
				state = LSTRING;
			}
			if (c == '.')
			{
				state = LREAL;
			}
			if (c == ('('))
			{
				if (in.peek() == '*')
				{
					state = LCOMMENT;
				}
				else
				{
					return LexItem(LPAREN, lex, linenum);
				}
			}
			if (c == ')')
			{
				Token t = RPAREN;
				return LexItem(t, lex, linenum);
			}
			if (c == ';')
			{
				Token t = SEMICOL;
				return LexItem(t, lex, linenum);
			}
			if (c == '=')
			{
				Token t = EQUAL;
				return LexItem(t, lex, linenum);
			}
			if (c == ':')
			{
				if (in.peek() == '=')
				{
					lex += in.get();
					Token t = ASSOP;
					return LexItem(t, lex, linenum);
				}
				else
				{
					Token t = COLON;
					return LexItem(t, lex, linenum);
				}
			}
			if (c == ',')
			{
				Token t = COMMA;
				return LexItem(t, lex, linenum);
			}
			if (c == '+')
			{
				Token t = PLUS;
				return LexItem(t, lex, linenum);
			}
			if (c == '-')
			{
				Token t = MINUS;
				return LexItem(t, lex, linenum);
			}
			if (c == '*')
			{
				Token t = MULT;
				return LexItem(t, lex, linenum);
			}
			if (c == '/')
			{
				Token t = DIV;
				return LexItem(t, lex, linenum);
			}
			if (c == '>')
			{
				Token t = GTHAN;
				return LexItem(t, lex, linenum);
			}
			if (c == '<')
			{
				Token t = LTHAN;
				return LexItem(t, lex, linenum);
			}
			if (c == '?')
			{
				Token t = ERR;
				return LexItem(t, lex, linenum);
			}

			break;

		case LIDENT:
			if (isalpha(c) || isdigit(c) || c == '_')
			{
				lex += c;
				if (in.eof())
				{
					return id_or_kw(allCap(lex), linenum);
				}
				if (identOver(in.peek()))
				{
					state = START;
					return id_or_kw(allCap(lex), linenum);
				}
			}
			else if (identOver(c))
			{
				in.putback(c);
				state = START;
				return id_or_kw(allCap(lex), linenum);
			}
			else if (c == '?')
			{
				in.putback(c);
				return id_or_kw(allCap(lex), linenum);
			}
			else
			{
				lex += c;
				Token t = ERR;
				return LexItem(t, lex, linenum);
			}

			break;

		case LINT:
			if (c == '.')
			{
				lex += c;
				state = LREAL;
				break;
			}
			else if (isdigit(c))
			{
				lex += c;
				break;
			}
			else if (identOver(c))
			{
				in.putback(c);
				Token t = ICONST;
				return LexItem(t, lex, linenum);
			}
			else if (isalpha(c) || c == '_')
			{
				in.putback(c);
				Token t = ICONST;
				return LexItem(t, lex, linenum);
			}
			else
			{
				Token t = ERR;
				lex += c;
				return LexItem(t, lex, linenum);
			}

		case LSTRING:
			if (c == '\'' || c == '"')
			{
				if (c == stringStart)
				{
					lex += c;
					// state = START;
					Token t = SCONST;
					lex = lex.substr(1, lex.length() - 2);
					return LexItem(t, lex, linenum);
				}
				else
				{
					lex += c;
					Token t = ERR;
					return LexItem(t, lex, linenum);
				}
			}
			else
			{
				char x;
				in.get(x);
				if (in.eof())
				{
					Token t = ERR;
					return LexItem(t, lex, linenum);
				}
				in.putback(x);
				lex += c;
				break;
			}

		case LCOMMENT:
			if (c == '*' && in.peek() == ')')
			{
				in.get();
				state = START;
			}
			else if (c == '\n')
			{
				linenum++;
			}

			char x;
			in.get(x);
			if (in.eof())
			{
				Token t = ERR;
				return LexItem(t, "errcomment", linenum);
			}
			in.putback(x);

			break;

		case LREAL:
			lex += c;
			if (isdigit(c))
			{
				if (identOver(in.peek()))
				{
					// state = START;
					Token t = RCONST;
					return LexItem(t, lex, linenum);
				}
			}
			else
			{
				Token t = ERR;
				return LexItem(t, lex, linenum);
			}
			break;
		}
	}

	Token t = DONE;
	return LexItem(t, "DONE", linenum);
}
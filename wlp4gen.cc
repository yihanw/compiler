#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <ctype.h>
#include <map>
#include <utility>
#include <iterator>
using namespace std;

struct Derivation {
	string lhs;
	vector<string> rhs;

	Derivation(vector<string> splitted) {
		lhs = splitted[0];
		splitted.erase(splitted.begin());
		rhs = splitted;
	}

	/* print lhs + rhs */
	string toString(void) {
		string ret = lhs;
		for (int i = 0; i < rhs.size(); i++) {
			ret += " " + rhs[i];
		}
		return ret;
	}
};

enum Type {
	INT,
	POINTER,
	NONE
};

struct Node {
	Derivation d;
	/* isTerminal, lexeme, children are initialized in createParseTree() */
	bool isTerminal;
	string lexeme;
	Type type;
	vector<Node> children;

	Node(vector<string> splitted) : d(splitted), type(NONE){}
};

/* A helper function that splits a string by ' ',
   and returns a vector<string> */
vector<string> split(string line) {
	vector<string> ret;
	stringstream ss(line);
	string s;
	while(getline(ss, s, ' ')) {
		ret.push_back(s);
	}
	return ret;
}

/* A helper function that checks if a string is all uppercase */
bool isUpper(string lhs) {
	int i = 0;
	while(lhs[i]) {
		if (!isupper(lhs[i])) {
			return false;
		}
		i++;
	}
	return true;
}
/* A helper function that creates a parse tree for parsed wlp4 file */
Node createParseTree(void) {
	string curLine;
	getline(cin, curLine);
	vector<string>splitted = split(curLine);

	Node curNode = Node(splitted);
	if (isUpper(curNode.d.lhs)) {
		curNode.isTerminal = true;
		curNode.lexeme = curNode.d.rhs[0];
	} else {
		curNode.isTerminal = false;
		for (int i = 0; i < curNode.d.rhs.size(); i++) {
			curNode.children.push_back(createParseTree());
		}
	}

	return curNode;
}

/* A8P1-4: symbol table */
void createSymbolTable (map<string, pair<vector<pair<string, string> >, map<string, string> > > &symbolTable,
                        Node &curNode, string curScope) {
	string nextScope = curScope;
	string name, type;
	/* main function */
	if (curNode.d.lhs == "main") {
		nextScope = "wain";
		symbolTable[nextScope].first = vector<pair<string, string> >();
		symbolTable[nextScope].second = map<string, string>();

		/* fill in parameter list */
		int paramPos[2] = {3, 5};
		for (int i = 0; i < 2; i++) {
			Node paramType = curNode.children[paramPos[i]].children[0];
			type = "";
			name = "";
			for (int j = 0; j < paramType.children.size(); j++) {
				type += paramType.children[j].lexeme;
			}
			name = curNode.children[paramPos[i]].children[1].lexeme;
			symbolTable["wain"].first.push_back(make_pair(type, name));
		}
	} else
	/* procedure function */
	if (curNode.d.lhs == "procedure") {
		name = curNode.children[1].lexeme;
		if (symbolTable.count(name)) {
			throw "ERROR: procedure " + name + " already declared";
		}
		symbolTable[name].first = vector<pair<string, string> >();
		symbolTable[name].second = map<string, string>();
		nextScope = name;
	} else
	/* paramter list for procedure function */
	if (curNode.d.lhs == "params") {
		if (curNode.children.size() > 0) {
			Node paramList = curNode.children[0];
			while (true) {
				Node paramType = paramList.children[0].children[0];
				type = "";
				name = "";
				for (int i = 0; i < paramType.children.size(); i++) {
					type += paramType.children[i].lexeme;
				}
				name = paramList.children[0].children[1].lexeme;
				symbolTable[curScope].first.push_back(make_pair(type, name));
				if (paramList.children.size() > 1) {
					paramList = paramList.children[2];
				} else {
					break;
				}
			}
		}
	} else
	/* declare variable */
	/* dcl -> type ID */
	if (curNode.d.lhs == "dcl") {
		name = curNode.children[1].lexeme;
		if (symbolTable[curScope].second.count(name)) {
			throw "ERROR: " + name + " already declared";
		}
		int typeLen = curNode.children[0].children.size();
		for (int i = 0; i < typeLen; i++) {
			type += curNode.children[0].children[i].lexeme;
		}
		symbolTable[curScope].second[name] = type;
	} else
	/* check variables */
	if ((curNode.d.toString() == "factor ID") || (curNode.d.toString() == "lvalue ID")) {
		name = curNode.children[0].lexeme;
		if (!symbolTable[curScope].second.count(name)) {
			throw "ERROR: " + name + "not declared";
		}
	} else
	/* check procedure */
	if ((curNode.d.toString() == "factor ID LPAREN RPAREN") ||
	    (curNode.d.toString() == "factor ID LPAREN arglist RPAREN")) {
		name = curNode.children[0].lexeme;
		if (!symbolTable.count(name)) {
			throw "ERROR: " + name + "not declared";
		}
		if (symbolTable[nextScope].second.count(name)) {
			throw "ERROR: " + name + "is declared";
		}
	}

	for (int i = 0; i < curNode.children.size(); i++) {
		createSymbolTable(symbolTable, curNode.children[i], nextScope);
	}
}

/* A helper function to print symbol table */
ostream& operator<<(ostream &out, map<string, pair<vector<string>, map<string, string> > > &symbolTable) {
	for (map<string, pair<vector<string>, map<string, string> > >::iterator scopeIt = symbolTable.begin();
	     scopeIt != symbolTable.end(); ++scopeIt) {
		/* print function name and parameter type list */
		out << scopeIt->first;
		for (vector<string>::iterator paramIt = scopeIt->second.first.begin();
		     paramIt != scopeIt->second.first.end(); ++paramIt) {
			out << " " << *paramIt;
		}
		out << endl;

		/* print type and name */
		for (map<string, string>::iterator variableIt = scopeIt->second.second.begin();
		     variableIt != scopeIt->second.second.end(); variableIt++) {
			out << variableIt->first << " " << variableIt->second << endl;
		}

		if (distance(scopeIt, symbolTable.end()) > 1) {
			out << endl;
		}
	}
}

/* A8P5-6: type checking */
Type checkType (map<string, pair<vector<pair<string, string> >, map<string, string> > > &symbolTable,
                Node &curNode, string scope) {
	string type[3] = {"int", "int*", "NONE"};
	string curString = curNode.d.toString();
	string e = "ERROR: " + curString + " wrong type";

	if (curString == "start BOF procedures EOF") {
		checkType(symbolTable, curNode.children[1], scope); // procedures
	} else if (curString == "procedures procedure procedures") {
		checkType(symbolTable, curNode.children[0], scope); // procedure
		checkType(symbolTable, curNode.children[1], scope); // procedures
	} else if (curString == "procedures main") {
		checkType(symbolTable, curNode.children[0], scope); // main
	} else if (curString == "procedure INT ID LPAREN params RPAREN LBRACE dcls statements RETURN expr SEMI RBRACE") {
		scope = curNode.children[1].lexeme;
		checkType(symbolTable, curNode.children[6], scope); // dcls
		checkType(symbolTable, curNode.children[7], scope); // statements

		if (checkType(symbolTable, curNode.children[9], scope) != INT) { // expr
			throw e;
		}
	} else if (curString == "main INT WAIN LPAREN dcl COMMA dcl RPAREN LBRACE dcls statements RETURN expr SEMI RBRACE") {
		scope = "wain";
		if (checkType(symbolTable, curNode.children[5], scope) != INT) { // dcl
			throw e;
		}
		checkType(symbolTable, curNode.children[8], scope); // dcl
		checkType(symbolTable, curNode.children[9], scope); // statement
		if (checkType(symbolTable, curNode.children[11], scope) != INT) { // expr
			throw e;
		}
	} else if (curString == "dcls dcls dcl BECOMES NUM SEMI") {
		if (checkType(symbolTable, curNode.children[1], scope) != INT) { //dcl
			throw e;
		}
		checkType(symbolTable, curNode.children[0], scope); // dcls
	} else if (curString == "dcls dcls dcl BECOMES NULL SEMI") {
		if (checkType(symbolTable, curNode.children[1], scope) != POINTER) { //dcl
			throw e;
		}
		checkType(symbolTable, curNode.children[0], scope); // dcls
	} else if (curString == "dcl type ID") {
		string name = symbolTable[scope].second[curNode.children[1].lexeme];
		if (name == "int") {
			curNode.type = INT;
			return INT;
		} else {
			curNode.type = POINTER;
			return POINTER;
		}
	} else if (curString == "statements statements statement") {
		checkType(symbolTable, curNode.children[0], scope); // statements
		checkType(symbolTable, curNode.children[1], scope); // statement
	} else if (curString == "statement lvalue BECOMES expr SEMI") {
		Type lvalueType = checkType(symbolTable, curNode.children[0], scope); // lvalue
		Type exprType = checkType(symbolTable, curNode.children[2], scope); // expr
		if (lvalueType != exprType) {
			throw e;
		}
	} else if (curString == "statement IF LPAREN test RPAREN LBRACE statements RBRACE ELSE LBRACE statements RBRACE") {
		checkType(symbolTable, curNode.children[2], scope); // test
		checkType(symbolTable, curNode.children[5], scope); // statements
		checkType(symbolTable, curNode.children[9], scope); // statements
	} else if (curString == "statement WHILE LPAREN test RPAREN LBRACE statements RBRACE") {
		checkType(symbolTable, curNode.children[2], scope); // test
		checkType(symbolTable, curNode.children[5], scope); // statements
	} else if (curString == "statement PRINTLN LPAREN expr RPAREN SEMI") {
		if (checkType(symbolTable, curNode.children[2], scope) != INT) { // expr
			throw e;
		}
	} else if (curString == "statement DELETE LBRACK RBRACK expr SEMI") {
		if (checkType(symbolTable, curNode.children[3], scope) != POINTER) { // expr
			throw e;
		}
	} else if ((curString == "test expr EQ expr") ||
		   (curString == "test expr NE expr") ||
		   (curString == "test expr LT expr") ||
		   (curString == "test expr LE expr") ||
		   (curString == "test expr GE expr") ||
		   (curString == "test expr GT expr")) {
		Type exprType1 = checkType(symbolTable, curNode.children[0], scope); // expr
		Type exprType2 = checkType(symbolTable, curNode.children[2], scope); // expr
		if (exprType1 != exprType2) {
			throw e;
		}
	} else if (curString == "expr term") {
		Type termType = checkType(symbolTable, curNode.children[0], scope);
		curNode.type = termType;
		return termType;
	} else if (curString == "expr expr PLUS term") {
		Type exprType = checkType(symbolTable, curNode.children[0], scope); // expr
		Type termType = checkType(symbolTable, curNode.children[2], scope); // term
		if ((exprType == INT) && (termType == INT)) {
			curNode.children[0].type = INT;
			curNode.children[2].type = INT;
			curNode.type = INT;
			return INT;
		} else if ((exprType == POINTER) && (termType == INT)) {
			curNode.children[0].type = POINTER;
			curNode.children[2].type = INT;
			curNode.type = POINTER;
			return POINTER;
		} else if ((exprType == INT) && (termType == POINTER)) {
			curNode.children[0].type = INT;
			curNode.children[2].type = POINTER;
			curNode.type = POINTER;
			return POINTER;
		} else {
			throw e;
		}
		
	} else if (curString == "expr expr MINUS term") {
		Type exprType = checkType(symbolTable, curNode.children[0], scope); // expr	
		Type termType = checkType(symbolTable, curNode.children[2], scope); // term
		if ((exprType == INT) && (termType == INT)) {
			curNode.children[0].type = INT;
			curNode.children[2].type = INT;
			curNode.type = INT;
			return INT;
		} else if ((exprType == POINTER) && (termType == INT)) {
			curNode.children[0].type = POINTER;
			curNode.children[2].type = INT;
			curNode.type = POINTER;
			return POINTER;
		} else if ((exprType == POINTER) && (termType == POINTER)) {
			curNode.children[0].type = POINTER;
			curNode.children[2].type = POINTER;
			curNode.type = INT;
			return INT;
		} else {
			throw e;
		}
	} else if (curString == "term factor") {
		Type factorType = checkType(symbolTable, curNode.children[0], scope); // factor
		curNode.type = factorType;
		return factorType;
	} else if ((curString == "term term STAR factor") ||
		   (curString == "term term SLASH factor") ||
		   (curString == "term term PCT factor")) {
		Type termType = checkType(symbolTable, curNode.children[0], scope); // term
		Type exprType = checkType(symbolTable, curNode.children[2], scope); // fator
		if ((termType != INT) || (exprType != INT)) {
			throw e;
		}
		curNode.type = INT;
		return INT;
	} else if (curString == "factor ID") {
		string name = symbolTable[scope].second[curNode.children[0].lexeme];
		if (name == "int") {
			curNode.type = INT;
			return INT;
		} else {
			curNode.type = POINTER;
			return POINTER;
		}
	} else if (curString == "factor NUM") {
		curNode.type = INT;
		return INT;
	} else if (curString == "factor NULL") {
		curNode.type = POINTER;
		return POINTER;
	} else if (curString == "factor LPAREN expr RPAREN") {
		Type exprType = checkType(symbolTable, curNode.children[1], scope); // expr
		curNode.type = exprType;
		return exprType;
	} else if (curString == "factor AMP lvalue") {
		if (checkType(symbolTable, curNode.children[1], scope) != INT) { // lvalue
			throw e;
		}
		curNode.type = POINTER;
		return POINTER;
	} else if (curString == "factor STAR factor") {
		if (checkType(symbolTable, curNode.children[1], scope) != POINTER) { // factor
			throw e;
		}
		curNode.type = INT;
		return INT;
	} else if (curString == "factor NEW INT LBRACK expr RBRACK") {
		if (checkType(symbolTable, curNode.children[3], scope) != INT) { // expr
			throw e;
		}
		curNode.type = POINTER;
		return POINTER;
	} else if (curString == "factor ID LPAREN RPAREN") {
		string name = curNode.children[0].lexeme;
		if (!symbolTable[name].first.empty()) {
			throw e;
		}
		curNode.type = INT;
		return INT;
	} else if (curString == "factor ID LPAREN arglist RPAREN") {
		string name = curNode.children[0].lexeme;
		vector<Type> typeList;
		Node arglist = curNode.children[2];
		while (true) {
			typeList.push_back(checkType(symbolTable, arglist.children[0], scope));
			if (arglist.children.size() > 1) {
				arglist = arglist.children[2];
			} else {
				break;
			}
		}

		vector<string> paramType = vector<string>();
		for (int i = 0; i < symbolTable[name].first.size(); i++) {
			paramType.push_back(symbolTable[name].first[i].first);
		}
		if (typeList.size() != paramType.size()) {
			throw e;
		}
		for (int i = 0; i < typeList.size(); i++) {
			if (type[typeList[i]] != paramType[i]) {
				throw e;
			}
		}
		checkType(symbolTable, curNode.children[2], scope);
		curNode.type = INT;
		return INT;
	} else if (curString == "lvalue ID") {
		string name = symbolTable[scope].second[curNode.children[0].lexeme];
		if (name == "int") {
			return INT;
		} else {
			return POINTER;
		}
	} else if (curString == "lvalue STAR factor") {
		if (checkType(symbolTable, curNode.children[1], scope) != POINTER) { // factor
			throw e;
		}	
		curNode.type = INT;
		return INT;
	} else if (curString == "lvalue LPAREN lvalue RPAREN") {
		Type lvalueType = checkType(symbolTable, curNode.children[1], scope); // lvalue
		curNode.type = lvalueType;
		return lvalueType;
		
	} else if (curString == "arglist expr") {
		checkType(symbolTable, curNode.children[0], scope);
	} else if (curString == "arglist expr COMMA arglist") {
		checkType(symbolTable, curNode.children[0], scope);
		checkType(symbolTable, curNode.children[2], scope);
	}

	return NONE;
}

/* A helper function that write MIPS code to push register r on stack */
void pushRegister(int r) {
	cout << "sw $" << r << ", -4($30)" << endl;
	cout << "sub $30, $30, $4" << endl;
}

/* A helper function that write MIPS code to pop register r from stack */
void popRegister(int r) {
	cout << "add $30, $30, $4" << endl;
	cout << "lw $" << r << ", -4($30)" << endl;
}

/* prolog */
void prolog(bool isInt) {
	cout << ".import print" << endl;
	cout << ".import init" << endl;
	cout << ".import new" << endl;
	cout << ".import delete" << endl << endl;

	cout << "lis $4" << endl;
	cout << ".word 4" << endl;
	cout << "lis $11" << endl;
	cout << ".word 1" << endl;
	pushRegister(31);
	cout << endl;

	if (isInt) {
		pushRegister(2);
		cout << "add $2, $0, $0" << endl;
	}
	cout << "lis $3" << endl;
	cout << ".word init" << endl;
	cout << "jalr $3" << endl;
	if (isInt) {
		popRegister(2);
	}
	popRegister(31);

	pushRegister(31);
	cout << "lis $3" << endl;
	cout << ".word wain" << endl;
	cout << "jalr $3" << endl;
	popRegister(31);
	cout << "jr $31" << endl;
}

/* epilog */
void epilog(void) {
	cout << "jr $31" << endl;
}

/* A helper function that generates the MIPS code */
void generateCode (map<string, pair<vector<pair<string, string> >, map<string, string> > > &symbolTable,
                   map<string, map<string, int> > &variableTable,
                   Node curNode, string scope, int &whileCount, int &ifCount) {
	string curString = curNode.d.toString();

	if (curString == "start BOF procedures EOF") {
		generateCode(symbolTable, variableTable, curNode.children[1], scope, whileCount, ifCount);
	} else if (curString == "procedures main") {
		generateCode(symbolTable, variableTable, curNode.children[0], scope, whileCount, ifCount);
	} else if (curString == "main INT WAIN LPAREN dcl COMMA dcl RPAREN LBRACE dcls statements RETURN expr SEMI RBRACE") {
		scope = "wain";

		cout << "wain: " << endl;
		pushRegister(31);
		cout << "add $29, $30, $0" << endl;
		pushRegister(1);
		variableTable[scope][curNode.children[3].children[1].lexeme] = -4;
		pushRegister(2);
		variableTable[scope][curNode.children[5].children[1].lexeme] = -8;
		cout << endl;

		/* dcls */
		generateCode(symbolTable, variableTable, curNode.children[8], scope, whileCount, ifCount);
		/* statements */
		generateCode(symbolTable, variableTable, curNode.children[9], scope, whileCount, ifCount);
		/* expr */
		generateCode(symbolTable, variableTable, curNode.children[11], scope, whileCount, ifCount);

		int totalOffset = variableTable[scope].size() * 4;
		cout << "lis $5" << endl;
		cout << ".word " << totalOffset << endl;
		cout << "add $30, $30, $5" << endl;
		popRegister(31);
		epilog();
	} else if (curString == "expr term") {
		generateCode(symbolTable, variableTable, curNode.children[0], scope, whileCount, ifCount);
	} else if (curString == "term factor") {
		generateCode(symbolTable, variableTable, curNode.children[0], scope, whileCount, ifCount);
	} else if (curString == "factor ID") {
		string symbol = curNode.children[0].lexeme;
		int offset = variableTable[scope][symbol];
		cout << "lw $3, " << offset << "($29)" << endl;
	} else if (curString == "factor LPAREN expr RPAREN") {
		Node exprNode = curNode.children[1];
		while ((exprNode.d.toString() == "expr term") &&
		       (exprNode.children[0].d.toString() == "term factor") &&
		       (exprNode.children[0].children[0].d.toString() == curString)) {
			exprNode = exprNode.children[0].children[0];
		}
		generateCode(symbolTable, variableTable, exprNode, scope, whileCount, ifCount);	
	} else if (curString == "expr expr PLUS term") {
		cout << "; add" << endl;
		Type exprType = curNode.children[0].type;
		Type termType = curNode.children[2].type;

		/* case 1: expr == INT , term == INT */
		if ((exprType == INT) && (termType == INT)) {
			pushRegister(5);
			generateCode(symbolTable, variableTable, curNode.children[0], scope, whileCount, ifCount);
			pushRegister(3);
			generateCode(symbolTable, variableTable, curNode.children[2], scope, whileCount, ifCount);
			popRegister(5);
			cout << "add $3, $5, $3" << endl;
			popRegister(5);
		} else 
		/* case 2: expr == POINTER, term == INT */
		if ((exprType == POINTER) && (termType == INT)) {
			generateCode(symbolTable, variableTable, curNode.children[2], scope, whileCount, ifCount);
			cout << "mult $3, $4" << endl;
			cout << "mflo $3" << endl;
			pushRegister(3);
			generateCode(symbolTable, variableTable, curNode.children[0], scope, whileCount, ifCount);
			popRegister(5);
			cout << "add $3, $3, $5" << endl;
		} else
		/* case 3: expr == INT, term == POINTER */
		if ((exprType == INT) && (termType == POINTER)) {
			generateCode(symbolTable, variableTable, curNode.children[0], scope, whileCount, ifCount);
			cout << "mult $3, $4" << endl;
			cout << "mflo $3" << endl;
			pushRegister(3);
			generateCode(symbolTable, variableTable, curNode.children[2], scope, whileCount, ifCount);
			popRegister(5);
			cout << "add $3, $5, $3" << endl;
		}

		cout << endl;
	} else if (curString == "expr expr MINUS term") {
		cout << "; minus" << endl;
		Type exprType = curNode.children[0].type;
		Type termType = curNode.children[2].type;

		/* case 1: expr == INT, term == INT */
		if ((exprType == INT) && (termType == INT)) {
			pushRegister(5);
			generateCode(symbolTable, variableTable, curNode.children[0], scope, whileCount, ifCount);
			pushRegister(3);
			generateCode(symbolTable, variableTable, curNode.children[2], scope, whileCount, ifCount);
			popRegister(5);
			cout << "sub $3, $5, $3" << endl;
			popRegister(5);
		} else 
		/* case 2: expr == POINTER, term == INT */
		if ((exprType == POINTER) && (termType == INT)) {
			generateCode(symbolTable, variableTable, curNode.children[2], scope, whileCount, ifCount);
			cout << "mult $3, $4" << endl;
			cout << "mflo $3" << endl;
			pushRegister(3);
			generateCode(symbolTable, variableTable, curNode.children[0], scope, whileCount, ifCount);
			popRegister(5);
			cout << "sub $3, $3, $5" << endl;
		} else
		/* case 3: expr == POINTER, term == POINTER */
		if ((exprType == POINTER) && (termType == POINTER)) {
			pushRegister(5);
			generateCode(symbolTable, variableTable, curNode.children[0], scope, whileCount, ifCount);
			pushRegister(3);
			generateCode(symbolTable, variableTable, curNode.children[2], scope, whileCount, ifCount);
			popRegister(5);
			cout << "sub $3, $5, $3" << endl;
			cout << "divu $3, $4" << endl;
			cout << "mflo $3" << endl;
			popRegister(5);
		}
		cout << endl;
	} else if (curString == "term term STAR factor") {
		cout << "; mult" << endl;
		pushRegister(5);
		generateCode(symbolTable, variableTable, curNode.children[0], scope, whileCount, ifCount);
		pushRegister(3);
		generateCode(symbolTable, variableTable, curNode.children[2], scope, whileCount, ifCount);
		popRegister(5);
		cout << "mult $3, $5" << endl;
		cout << "mflo $3" << endl;
		popRegister(5);
		cout << endl;
	} else if (curString == "term term SLASH factor") {
		cout << "; div" << endl;
		pushRegister(5);
		generateCode(symbolTable, variableTable, curNode.children[0], scope, whileCount, ifCount);
		pushRegister(3);
		generateCode(symbolTable, variableTable, curNode.children[2], scope, whileCount, ifCount);
		popRegister(5);
		cout << "div $5, $3" << endl;
		cout << "mflo $3" << endl;
		popRegister(5);
		cout << endl;
	} else if (curString == "term term PCT factor") {
		cout << "; %" << endl;
		pushRegister(5);
		generateCode(symbolTable, variableTable, curNode.children[0], scope, whileCount, ifCount);
		pushRegister(3);
		generateCode(symbolTable, variableTable, curNode.children[2], scope, whileCount, ifCount);
		popRegister(5);
		cout << "div $5, $3" << endl;
		cout << "mfhi $3" << endl;
		popRegister(5);
		cout << endl;
	} else if (curString == "factor NUM") {
		string symbol = curNode.children[0].lexeme;
		cout << "lis $3" << endl;
		cout << ".word " << symbol << endl;
	} else if (curString == "statements statements statement") {
		generateCode(symbolTable, variableTable, curNode.children[0], scope, whileCount, ifCount);
		generateCode(symbolTable, variableTable, curNode.children[1], scope, whileCount, ifCount);
	} else if (curString == "statement PRINTLN LPAREN expr RPAREN SEMI") {
		pushRegister(1);
		pushRegister(31);
		generateCode(symbolTable, variableTable, curNode.children[2], scope, whileCount, ifCount);

		cout << "; print" << endl;
		cout << "add $1, $3, $0" << endl;
		cout << "lis $10" << endl;
		cout << ".word print" << endl;
		cout << "jalr $10" << endl << endl;

		popRegister(31);
		popRegister(1);
	} else if (curString == "dcls dcls dcl BECOMES NUM SEMI") {
		generateCode(symbolTable, variableTable, curNode.children[0], scope, whileCount, ifCount);

		string symbol = curNode.children[1].children[1].lexeme;
		string assignValue = curNode.children[3].lexeme;

		cout << "; " << symbol << " = " << assignValue << endl;
		cout << "lis $3" << endl;
		cout << ".word " << assignValue << endl;
		pushRegister(3);
		cout << endl;

		int offset = 0;
		for (map<string, int>::iterator it = variableTable[scope].begin(); it != variableTable[scope].end(); ++it) {
			if (it->second <= 0) {
				offset++;
			}
		}
		offset++;
		offset *= -4;

		variableTable[scope][symbol] = offset;
	} else if (curString == "statement lvalue BECOMES expr SEMI") {
		Node lvalueNode = curNode.children[0];
		while (lvalueNode.d.toString() == "lvalue LPAREN lvalue RPAREN") {
			lvalueNode = lvalueNode.children[1];
		}

		if (lvalueNode.d.toString() == "lvalue ID") {
			generateCode(symbolTable, variableTable, curNode.children[2], scope, whileCount, ifCount);
			string symbol = lvalueNode.children[0].lexeme;
			int offset = variableTable[scope][symbol];
			cout << "sw $3, " << offset << "($29)" << endl;
		} else if (lvalueNode.d.toString() == "lvalue STAR factor") {
			generateCode(symbolTable, variableTable, lvalueNode.children[1], scope, whileCount, ifCount);
			cout << "add $7, $3, $0" << endl; //copy $3 to $7
			generateCode(symbolTable, variableTable, curNode.children[2], scope, whileCount, ifCount);
			cout << "sw $3, 0($7)" << endl << endl; // store back to $3
		}
	} else if (curString == "statement WHILE LPAREN test RPAREN LBRACE statements RBRACE") {
		cout << "; while loop" << endl;

		whileCount++;
		int curWhile = whileCount;
		cout << "sWhile" << curWhile<< ":" << endl;
		generateCode(symbolTable, variableTable, curNode.children[2], scope, whileCount, ifCount);
		cout << "beq $3, $0, eWhile" << curWhile<< endl;
		generateCode(symbolTable, variableTable, curNode.children[5], scope, whileCount, ifCount);
		cout << "beq $0, $0, sWhile" << curWhile<< endl;
		cout << "eWhile" << curWhile << ":" << endl << endl;
	} else if (curString == "test expr LT expr") {
		cout << "; <" << endl;
		pushRegister(5);
		generateCode(symbolTable, variableTable, curNode.children[0], scope, whileCount, ifCount);
		pushRegister(3);
		generateCode(symbolTable, variableTable, curNode.children[2], scope, whileCount, ifCount);
		popRegister(5);
		if (curNode.children[0].type == INT) {
			cout << "slt $3, $5, $3" << endl;
		} else if (curNode.children[0].type == POINTER) {
			cout << "sltu $3, $5, $3" << endl;
		}
		popRegister(5);
		cout << endl;
	} else if (curString == "test expr EQ expr") {
		cout << "; =" << endl;
		pushRegister(5);
		pushRegister(6);
		pushRegister(7);
		generateCode(symbolTable, variableTable, curNode.children[0], scope, whileCount, ifCount);
		pushRegister(3);
		generateCode(symbolTable, variableTable, curNode.children[2], scope, whileCount, ifCount);
		popRegister(5);
		if (curNode.children[0].type == INT) {
			cout << "slt $6, $3, $5" << endl;
			cout << "slt $7, $5, $3" << endl;
		} else if (curNode.children[0].type == POINTER) {
			cout << "sltu $6, $3, $5" << endl;
			cout << "sltu $7, $5, $3" << endl;
		}
		cout << "add $3, $6, $7" << endl;
		cout << "sub $3, $11, $3" << endl;
		popRegister(7);
		popRegister(6);
		popRegister(5);
		cout << endl;
	} else if (curString == "test expr NE expr") {
		cout << "; != " << endl;
		pushRegister(5);
		pushRegister(6);
		pushRegister(7);
		generateCode(symbolTable, variableTable, curNode.children[0], scope, whileCount, ifCount);
		pushRegister(3);
		generateCode(symbolTable, variableTable, curNode.children[2], scope, whileCount, ifCount);
		popRegister(5);
		if (curNode.children[0].type == INT) {
			cout << "slt $6, $3, $5" << endl;
			cout << "slt $7, $5, $3" << endl;
		} else if (curNode.children[0].type == POINTER) {
			cout << "sltu $6, $3, $5" << endl;
			cout << "sltu $7, $5, $3" << endl;
		}
		cout << "add $3, $6, $7" << endl;
		popRegister(7);
		popRegister(6);
		popRegister(5);
		cout << endl;
	} else if (curString == "test expr LE expr") {
		cout << "; <=" << endl;
		pushRegister(5);
		generateCode(symbolTable, variableTable, curNode.children[0], scope, whileCount, ifCount);
		pushRegister(3);
		generateCode(symbolTable, variableTable, curNode.children[2], scope, whileCount, ifCount);
		popRegister(5);
		if (curNode.children[0].type == INT) {
			cout << "slt $3, $3, $5" << endl;
		} else if (curNode.children[0].type == POINTER) {
			cout << "sltu $3, $3, $5" << endl;
		}
		cout << "sub $3, $11, $3" << endl;
		popRegister(5);
		cout << endl;
	} else if (curString == "test expr GE expr") {
		cout << "; >=" << endl;
		pushRegister(5);
		generateCode(symbolTable, variableTable, curNode.children[0], scope, whileCount, ifCount);
		pushRegister(3);
		generateCode(symbolTable, variableTable, curNode.children[2], scope, whileCount, ifCount);
		popRegister(5);
		if (curNode.children[0].type == INT) {
			cout << "slt $3, $5, $3" << endl;
		} else if (curNode.children[0].type == POINTER) {
			cout << "sltu $3, $5, $3" << endl;
		}
		cout << "sub $3, $11, $3" << endl;
		popRegister(5);
		cout << endl;
	} else if (curString == "test expr GT expr") {
		cout << "; >" << endl;
		pushRegister(5);
		generateCode(symbolTable, variableTable, curNode.children[0], scope, whileCount, ifCount);
		pushRegister(3);
		generateCode(symbolTable, variableTable, curNode.children[2], scope, whileCount, ifCount);
		popRegister(5);
		if (curNode.children[0].type == INT) {
			cout << "slt $3, $3, $5" << endl;
		} else if (curNode.children[0].type == POINTER) {
			cout << "sltu $3, $3, $5" << endl;
		}
		popRegister(5);
		cout << endl;
	} else if (curString == "statement IF LPAREN test RPAREN LBRACE statements RBRACE ELSE LBRACE statements RBRACE") {
		ifCount++;
		int curIf = ifCount;
		cout << "; if" << endl;
		generateCode(symbolTable, variableTable, curNode.children[2], scope, whileCount, ifCount);
		cout << "beq $3, $0, sIf" << curIf << endl;
		generateCode(symbolTable, variableTable, curNode.children[5], scope, whileCount, ifCount);
		cout << "beq $0, $0, eIf" << curIf << endl;
		cout << "sIf" << curIf << ":" << endl;
		generateCode(symbolTable, variableTable, curNode.children[9], scope, whileCount, ifCount);
		cout << "eIf" << curIf << ":" << endl;
	} else 
	
	/* A10P1 */
	if (curString == "factor STAR factor") {
		cout << "; dereference" << endl;
		generateCode(symbolTable, variableTable, curNode.children[1], scope, whileCount, ifCount);
		cout << "lw $3, 0($3)" << endl << endl;
	} else if (curString == "factor NULL") {
		cout << "; dereference NULL" << endl;
		cout << "add $3, $0, $11" << endl << endl;
	} else if (curString == "factor AMP lvalue") {
		Node lvalueNode = curNode.children[1];

		while (lvalueNode.d.toString() == "lvalue LPAREN lvalue RPAREN") {
			lvalueNode = lvalueNode.children[1];
		}
		/* case 1: lvalue ID */
		if (lvalueNode.d.toString() == "lvalue ID") {
			string symbol = lvalueNode.children[0].lexeme;
			int offset = variableTable[scope][symbol];
			cout << "lis $3" << endl;
			cout << ".word " << offset << endl;
			cout << "add $3, $3, $29" << endl << endl;
		} else 
		/* case 2: lvalue STAR factor */
		if (lvalueNode.d.toString() == "lvalue STAR factor") {
			generateCode(symbolTable, variableTable, lvalueNode.children[1], scope, whileCount, ifCount);
		}
		
	} else if (curString == "dcls dcls dcl BECOMES NULL SEMI") {
		generateCode(symbolTable, variableTable, curNode.children[0], scope, whileCount, ifCount);

		string symbol = curNode.children[1].children[1].lexeme;
		string assignValue = curNode.children[3].lexeme;

		cout << "; " << symbol << " = " << assignValue << endl;
		cout << "add $3, $0, $11" << endl;
		pushRegister(3);
		cout << endl;

		int offset = 0;
		for (map<string, int>::iterator it = variableTable[scope].begin(); it != variableTable[scope].end(); ++it) {
			if (it->second <= 0) {
				offset++;
			}
		}
		offset++;
		offset *= -4;

		variableTable[scope][symbol] = offset;
	} else 

	/* A10P2 */
	if (curString == "factor NEW INT LBRACK expr RBRACK") {
		cout << "; new" << endl;
		generateCode(symbolTable, variableTable, curNode.children[3], scope, whileCount, ifCount);

		pushRegister(1);
		pushRegister(5);
//		pushRegister(10);
		pushRegister(31);
#if 0
		cout << "add $1, $0, $3" << endl;
		cout << "lis $10" << endl;
		cout << ".word new" << endl;
		cout << "jalr $10" << endl;

		cout << "slt $5, $3, $11" << endl;
		cout << "bne $5, $11, 1" << endl;
		cout << "add $3, $0, $11" << endl;
#endif

		cout << "add $1, $0, $3" << endl;
		cout << "lis $5" << endl;
		cout << ".word new" << endl;
		cout << "jalr $5" << endl;
		cout << "add $5, $0, $3" << endl;
		cout << "sltu $3, $5, $11" << endl;
		cout << "beq $3, $11, 2"  << endl;
		cout << "add $3, $0, $5" << endl;
		cout << "beq $0, $0, 1" << endl;
		cout << "add $3, $0, $11" << endl;
		popRegister(31);
//		popRegister(10);
		popRegister(5);
		popRegister(1);
		cout << endl;
	} else if (curString == "statement DELETE LBRACK RBRACK expr SEMI") {
		cout << "; delete" << endl;
		pushRegister(1);
		pushRegister(10);
		pushRegister(31);
		generateCode(symbolTable, variableTable, curNode.children[3], scope, whileCount, ifCount);
		cout << "beq $3, $11, 4" << endl;
		cout << "add $1, $0, $3" << endl;
		cout << "lis $10" << endl;
		cout << ".word delete" << endl;
		cout << "jalr $10" << endl;
		popRegister(31);
		popRegister(10);
		popRegister(1);
		cout << endl;
	} else if (curString == "procedures procedure procedures") {
		generateCode(symbolTable, variableTable, curNode.children[0], scope, whileCount, ifCount);
		generateCode(symbolTable, variableTable, curNode.children[1], scope, whileCount, ifCount);
	} else if (curString == "procedure INT ID LPAREN params RPAREN LBRACE dcls statements RETURN expr SEMI RBRACE") {
		scope = curNode.children[1].lexeme;
		cout << "func" << scope << ":" << endl;
		cout << "add $29, $0, $30" << endl;

		/* fill in variableTable */
		int paramCount = symbolTable[scope].first.size();
		for (int i = 0; i< paramCount; i++) {
			int offset = -4 * (i + 1);
			string name = symbolTable[scope].first[i].second;
			variableTable[scope][name] = offset;
		}
	
		/* dcls */
		generateCode(symbolTable, variableTable, curNode.children[6], scope, whileCount, ifCount);

		/* update variableTable */
		for (map<string, int>::iterator it = variableTable[scope].begin(); it != variableTable[scope].end(); it++) {
			variableTable[scope][it->first] += 4 * paramCount;
		}

		/* statements */
		generateCode(symbolTable, variableTable, curNode.children[7], scope, whileCount, ifCount);
		/* expr */
		generateCode(symbolTable, variableTable, curNode.children[9], scope, whileCount, ifCount);

		int totalOffset = (variableTable[scope].size() - paramCount ) * 4;
		cout << "; procedure pop" << endl;
		cout << "lis $5" << endl;
		cout << ".word " << totalOffset << endl;
		cout << "add $30, $30, $5" << endl;
		epilog();
	} else if (curString == "factor ID LPAREN RPAREN") {
		pushRegister(31);
		cout << "lis $3" << endl;
		string funcName = curNode.children[0].lexeme;
		cout << ".word func" << funcName << endl;
		cout << "jalr $3" << endl;
		popRegister(31);
	} else if (curString == "factor ID LPAREN arglist RPAREN") {
		cout << "; procedure with param" << endl;
		pushRegister(29);
		pushRegister(31);
		pushRegister(5);
		generateCode(symbolTable, variableTable, curNode.children[2], scope, whileCount, ifCount);

		string name = curNode.children[0].lexeme;
		int paramCount = symbolTable[name].first.size();
		cout << "lis $3" << endl;
		cout << ".word func" << name << endl;
		cout << "jalr $3" << endl;
		int totalOffset = paramCount * 4;
		cout << "lis $5" << endl;
		cout << ".word " << totalOffset << endl;
		cout << "add $30, $30, $5" << endl;
		popRegister(5);
		popRegister(31);
		popRegister(29);
	} else if (curString == "arglist expr") {
		generateCode(symbolTable, variableTable, curNode.children[0], scope, whileCount, ifCount);
		pushRegister(3);
	} else if (curString == "arglist expr COMMA arglist") {
		generateCode(symbolTable, variableTable, curNode.children[0], scope, whileCount, ifCount);
		pushRegister(3);
		generateCode(symbolTable, variableTable, curNode.children[2], scope, whileCount, ifCount);
	}
}

int main(void) {
	Node root = createParseTree();

	/* symbolTable: <scope, <paramlist, <name, type>>> */
	map<string, pair<vector<pair<string, string> >, map<string, string> > > symbolTable;

	try {
		createSymbolTable(symbolTable, root, "");
	} catch (string e) {
		cerr << e << endl;
		return 0;
	}

//	cerr << symbolTable;

	try {
		checkType(symbolTable, root, "");
	} catch (string e) {
		cerr << e;
		return 0;
	}

	/* variableTable: <scope, <symbol, offset>> */
	map<string, map<string, int> > variableTable;

	/* prolog pass in true if wain returns an int, otherwise false */
	if (symbolTable["wain"].first[0].first == "int") {
		prolog(true);
	} else {
		prolog(false);
	}

	int whileCount = 0, ifCount = 0;
	generateCode(symbolTable, variableTable, root, "", whileCount, ifCount);

	return 0;
}

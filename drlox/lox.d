module lox;

import std.stdio;
import std.file;
import std.string;
import std.ascii;
import std.conv;

enum Lox {
	INVALID = 0,
	
	NUMBER,
	STRING,
	IDENTIFIER,
	BOOLEAN,
	
	LEFT_PAREN,
	RIGHT_PAREN,
	LEFT_BRACE,
	RIGHT_BRACE,
	COMMA,
	DOT,
	MINUS,
	PLUS,
	SEMICOLON,
	SLASH,
	STAR,
	
	BANG,
	BANG_EQUAL,
	EQUAL,
	EQUAL_EQUAL,
	GREATER,
	GREATER_EQUAL,
	LESS,
	LESS_EQUAL,
	
	AND,
	CLASS,
	ELSE,
	FASLE,
	FUN,
	FOR,
	IF,
	NIL,
	OR,
	PRINT,
	RETURN,
	SUPER,
	THIS,
	TRUE,
	VAR,
	WHILE,
	
	END,
}

union Value {
	string asString;
	long asInteger;
	double asNumber;
	bool asBoolean;
	
	this(string a) {
		asString = a;
	}
	
	this(long a) {
		asInteger = a;
	}
	
	this(double a) {
		asNumber = a;
	}
	
	this(bool a) {
		asBoolean = a;
	}
}

struct Token {
	Lox type;
	Value value;
	size_t location;
	
	this(Lox type, Value value, size_t location) {
		this.type = type;
		this.value = value;
		this.location = location;
	}
	
	void print() {
		write(location, " ", type, " ");
		
		if (type == Lox.NUMBER) {
			writeln(value.asNumber);
		}
		else if (type == Lox.STRING) {
			writeln("'", value.asString, "'");
		}
		else if (type == Lox.IDENTIFIER) {
			writeln(value.asString);
		}
	}
}

struct Node {
	Lox type;
	Value value;
	Node[] nodes;
	size_t location;
	
	this(Lox type, Value value, size_t location) {
		this.type = type;
		this.value = value;
		this.location = location;
	}
	
	void addSub(Node node) {
		nodes.length += 1;
		nodes[$ - 1] = node;
	}
}

struct Enviornment {
	string[string] env;
}

Token[] tokenise(string content) {
	Token[] tokens;
	
	for (size_t i = 0; i < content.length - 1; i++) {
		char current = content[i];
		
		switch (current) {
			case '(': {
				tokens.length += 1;
				tokens[$ - 1] = Token(Lox.LEFT_PAREN, Value(0), i);
				break;
			}
			
			case ')': {
				tokens.length += 1;
				tokens[$ - 1] = Token(Lox.RIGHT_PAREN, Value(0), i);
				break;
			}
			
			case '{': {
				tokens.length += 1;
				tokens[$ - 1] = Token(Lox.LEFT_BRACE, Value(0), i);
				break;
			}
			
			case '}': {
				tokens.length += 1;
				tokens[$ - 1] = Token(Lox.RIGHT_BRACE, Value(0), i);
				break;
			}
			
			case ',': {
				tokens.length += 1;
				tokens[$ - 1] = Token(Lox.COMMA, Value(0), i);
				break;
			}
			
			case '.': {
				tokens.length += 1;
				tokens[$ - 1] = Token(Lox.DOT, Value(0), i);
				break;
			}
			
			case '-': {
				tokens.length += 1;
				tokens[$ - 1] = Token(Lox.MINUS, Value(0), i);
				break;
			}
			
			case '+': {
				tokens.length += 1;
				tokens[$ - 1] = Token(Lox.PLUS, Value(0), i);
				break;
			}
			
			case ';': {
				tokens.length += 1;
				tokens[$ - 1] = Token(Lox.SEMICOLON, Value(0), i);
				break;
			}
			
			case '/': {
				tokens.length += 1;
				tokens[$ - 1] = Token(Lox.SLASH, Value(0), i);
				break;
			}
			
			case '*': {
				tokens.length += 1;
				tokens[$ - 1] = Token(Lox.STAR, Value(0), i);
				break;
			}
			
			// Strings
			case '"': {
				tokens.length += 1;
				
				size_t start = ++i;
				
				do {
					i++;
				} while (content[i] != '"' && i < content.length - 1);
				
				size_t end = i;
				
				string s = content[start .. end];
				tokens[$ - 1] = Token(Lox.STRING, Value(s), start);
				
				break;
			}
			
			case ' ':
			case '\t':
			case '\n':
			case '\r': {
				break;
			}
			
			default: {
				if (isAlpha(current) || current == '_') {
					tokens.length += 1;
					
					size_t start = i;
					
					do {
						i++;
					} while ((isAlphaNum(content[i]) || content[i] == '_') && i < content.length - 1);
					
					size_t end = i;
					
					string s = content[start .. end];
					tokens[$ - 1] = Token(Lox.IDENTIFIER, Value(s), start);
				}
				
				else if (isDigit(current)) {
					tokens.length += 1;
					
					size_t start = i;
					
					do {
						i++;
					} while ((isDigit(content[i]) || content[i] == '.') && i < content.length - 1);
					
					size_t end = i;
					
					string s = content[start .. end];
					
					switch (s) {
						case "and": {
							tokens[$ - 1] = Token(Lox.AND, Value(0), start);
							break;
						}
						case "class": {
							tokens[$ - 1] = Token(Lox.CLASS, Value(0), start);
							break;
						}
						case "else": {
							tokens[$ - 1] = Token(Lox.ELSE, Value(0), start);
							break;
						}
						case "false": {
							tokens[$ - 1] = Token(Lox.BOOLEAN, Value(false), start);
							break;
						}
						default: {
							tokens[$ - 1] = Token(Lox.NUMBER, Value(to!double(s)), start);
						}
					}
				}
			}
		}
	}
	
	return tokens;
}

class Script {
	Enviornment env;
	
	this() {}
	
	void run(string content) {
		Token[] tokens = tokenise(content);
		
		foreach (Token t; tokens) {
			t.print();
		}
	}
}

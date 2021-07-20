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
		else {
			write("\n");
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
	
	this(Lox type, Value value, size_t location, Node left) {
		this.type = type;
		this.value = value;
		this.location = location;
		
		this.addSub(left);
	}
	
	this(Lox type, Value value, size_t location, Node left, Node right) {
		this.type = type;
		this.value = value;
		this.location = location;
		
		this.addSub(left);
		this.addSub(right);
	}
	
	this(Lox type, Value value, size_t location, Node left, Node centre, Node right) {
		this.type = type;
		this.value = value;
		this.location = location;
		
		this.addSub(left);
		this.addSub(centre);
		this.addSub(right);
	}
	
	void addSub(Node node) {
		nodes.length += 1;
		nodes[$ - 1] = node;
	}
	
	Node getSub(size_t index) {
		return nodes[$ - 1];
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
			
			case '!': {
				++i;
				if (content[i] == '=') {
					tokens.length += 1;
					tokens[$ - 1] = Token(Lox.BANG_EQUAL, Value(0), i);
					break;
				}
				else {
					tokens.length += 1;
					tokens[$ - 1] = Token(Lox.BANG, Value(0), i);
					break;
				}
			}
			
			case '=': {
				++i;
				if (content[i] == '=') {
					tokens.length += 1;
					tokens[$ - 1] = Token(Lox.EQUAL_EQUAL, Value(0), i);
					break;
				}
				else {
					tokens.length += 1;
					tokens[$ - 1] = Token(Lox.EQUAL, Value(0), i);
					break;
				}
			}
			
			case '>': {
				++i;
				if (content[i] == '=') {
					tokens.length += 1;
					tokens[$ - 1] = Token(Lox.GREATER_EQUAL, Value(0), i);
					break;
				}
				else {
					tokens.length += 1;
					tokens[$ - 1] = Token(Lox.GREATER, Value(0), i);
					break;
				}
			}
			
			case '<': {
				++i;
				if (content[i] == '=') {
					tokens.length += 1;
					tokens[$ - 1] = Token(Lox.LESS_EQUAL, Value(0), i);
					break;
				}
				else {
					tokens.length += 1;
					tokens[$ - 1] = Token(Lox.LESS, Value(0), i);
					break;
				}
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
					
					size_t end = i--;
					
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
						case "fun": {
							tokens[$ - 1] = Token(Lox.FUN, Value(0), start);
							break;
						}
						case "for": {
							tokens[$ - 1] = Token(Lox.FOR, Value(0), start);
							break;
						}
						case "if": {
							tokens[$ - 1] = Token(Lox.IF, Value(0), start);
							break;
						}
						case "nil": {
							tokens[$ - 1] = Token(Lox.NIL, Value(0), start);
							break;
						}
						case "or": {
							tokens[$ - 1] = Token(Lox.OR, Value(0), start);
							break;
						}
						case "print": {
							tokens[$ - 1] = Token(Lox.PRINT, Value(0), start);
							break;
						}
						case "return": {
							tokens[$ - 1] = Token(Lox.RETURN, Value(0), start);
							break;
						}
						case "super": {
							tokens[$ - 1] = Token(Lox.SUPER, Value(0), start);
							break;
						}
						case "this": {
							tokens[$ - 1] = Token(Lox.THIS, Value(0), start);
							break;
						}
						case "true": {
							tokens[$ - 1] = Token(Lox.BOOLEAN, Value(true), start);
							break;
						}
						case "var": {
							tokens[$ - 1] = Token(Lox.VAR, Value(0), start);
							break;
						}
						case "while": {
							tokens[$ - 1] = Token(Lox.WHILE, Value(0), start);
							break;
						}
						default: {
							tokens[$ - 1] = Token(Lox.IDENTIFIER, Value(s), start);
							break;
						}
					}
				}
				
				else if (isDigit(current)) {
					tokens.length += 1;
					
					size_t start = i;
					
					do {
						i++;
					} while ((isDigit(content[i]) || content[i] == '.') && i < content.length - 1);
					
					size_t end = i--;
					
					string s = content[start .. end];
					tokens[$ - 1] = Token(Lox.NUMBER, Value(to!double(s)), start);
				}
			}
		}
	}
	
	return tokens;
}

Node parse(Token[] content) {
	return Node(Lox.INVALID, Value(0), 0);
}

class Script {
	Enviornment env;
	
	this() {}
	
	void run(string content) {
		Token[] tokens = tokenise(content);
		
		foreach (Token t; tokens) {
			t.print();
		}
		
		Node node = parse(tokens);
	}
}

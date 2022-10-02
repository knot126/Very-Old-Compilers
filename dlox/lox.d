module lox;

import std.stdio;
import std.file;
import std.string;
import std.ascii;
import std.conv;

enum Lox {
	INVALID = 0,
	
	GROUPING,
	
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
	PERCENT,
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

struct InterpreterValue {
	Lox type;
	Value value;
	
	this(Lox type) {
		this.type = type;
		this.value = Value(0);
	}
	
	this(Lox type, Value value) {
		this.type = type;
		this.value = value;
	}
	
	void print() {
		if (type == Lox.NUMBER) {
			write(value.asNumber);
		}
		else if (type == Lox.STRING) {
			write("'", value.asString, "'");
		}
		else if (type == Lox.IDENTIFIER) {
			write(value.asString);
		}
		else if (type == Lox.BOOLEAN) {
			if (value.asBoolean == false) {
				write("false");
			}
			else {
				write("true");
			}
		}
		else if (type == Lox.NIL) {
			write("nil");
		}
		else {
			write("<object ", type, ">");
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
	
	void print(size_t level) {
		for (size_t i = 0; i < level; i++) {
			write("\t");
		}
		
		write("(", this.location, ") ", this.type, " (= ");
		
		if (type == Lox.NUMBER) {
			write(value.asNumber);
		}
		else if (type == Lox.STRING) {
			write("'", value.asString, "'");
		}
		else if (type == Lox.IDENTIFIER) {
			write(value.asString);
		}
		else {
			write("(value)");
		}
		
		writeln("):");
		
		foreach (Node n; this.nodes) {
			n.print(level + 1);
		}
	}
}

struct Enviornment {
	string[string] env;
}

/**
 * Errors
 */

class LoxError : Exception {
	this(string msg, string file = __FILE__, size_t line = __LINE__) {
		super(msg, file, line);
	}
}

class ParsingError : LoxError {
	this(string msg, string file = __FILE__, size_t line = __LINE__) {
		super(msg, file, line);
	}
}

class InterpreterError : LoxError {
	this(string msg, string file = __FILE__, size_t line = __LINE__) {
		super(msg, file, line);
	}
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
			
			case '%': {
				tokens.length += 1;
				tokens[$ - 1] = Token(Lox.PERCENT, Value(0), i);
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
					tokens[$ - 1] = Token(Lox.BANG, Value(0), --i);
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
					tokens[$ - 1] = Token(Lox.EQUAL, Value(0), --i);
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
					tokens[$ - 1] = Token(Lox.GREATER, Value(0), --i);
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
					tokens[$ - 1] = Token(Lox.LESS, Value(0), --i);
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

class Parser {
	Token[] tokens;
	size_t current;
	
	this() {
		this.tokens = null;
		this.current = 0;
	}
	
	bool match(Lox type) {
		if (current == tokens.length) {
			return false;
		}
		
		if (type == tokens[current].type) {
			this.current += 1;
			return true;
		}
		
		return false;
	}
	
	void expect(Lox type, string reason) {
		if (tokens[current].type != type) {
			throw new ParsingError(reason);
		}
		else {
			this.current += 1;
		}
	}
	
	Token previous() {
		return tokens[current - 1];
	}
	
	Lox previous_type() {
		return tokens[current - 1].type;
	}
	
	size_t location() {
		return tokens[current - 1].location;
	}
	
	Node[] parse(Token[] tokens) {
		this.tokens = tokens;
		Node[] nodes;
		
		while (this.current < this.tokens.length) {
			nodes.length += 1;
			nodes[$ - 1] = this.expr_stmt();
		}
		
		return nodes;
	}
	
	Node stmt() {
		if (this.match(Lox.PRINT)) {
			
		}
	}
	
	Node expr_stmt() {
		/**
		 * Do an expression statement
		 */
		Node n = this.expression();
		this.expect(Lox.SEMICOLON, "Expecting semicolon at end of expresion statement.");
		return n;
	}
	
	Node print_stmt() {
		
	}
	
	Node expression() {
		return this.equality();
	}
	
	Node equality() {
		Node left = this.comparison();
		
		while (this.match(Lox.BANG_EQUAL) || this.match(Lox.EQUAL_EQUAL)) {
			Lox type = this.previous_type();
			Node right = this.comparison();
			left = Node(type, Value(0), this.location(), left, right);
		}
		
		return left;
	}
	
	Node comparison() {
		Node left = this.term();
		
		while (this.match(Lox.GREATER) || this.match(Lox.GREATER_EQUAL) || this.match(Lox.LESS) || this.match(Lox.GREATER)) {
			Lox type = this.previous_type();
			Node right = this.term();
			left = Node(type, Value(0), this.location(), left, right);
		}
		
		return left;
	}
	
	Node term() {
		Node left = this.factor();
		
		while (this.match(Lox.PLUS) || this.match(Lox.MINUS)) {
			Lox type = this.previous_type();
			Node right = this.factor();
			left = Node(type, Value(0), this.location(), left, right);
		}
		
		return left;
	}
	
	Node factor() {
		Node left = this.unary();
		
		while (this.match(Lox.SLASH) || this.match(Lox.STAR) || this.match(Lox.PERCENT)) {
			Lox type = this.previous_type();
			Node right = this.unary();
			left = Node(type, Value(0), this.location(), left, right);
		}
		
		return left;
	}
	
	Node unary() {
		if (this.match(Lox.BANG) || this.match(Lox.MINUS)) {
			Lox type = this.previous_type();
			Node left = this.unary();
			return Node(type, Value(0), this.location(), left);
		}
		
		return this.primary();
	}
	
	Node primary() {
		if (this.match(Lox.FASLE)) {
			return Node(Lox.BOOLEAN, Value(false), this.location());
		}
		
		if (this.match(Lox.TRUE)) {
			return Node(Lox.BOOLEAN, Value(true), this.location());
		}
		
		if (this.match(Lox.NIL)) {
			return Node(Lox.NIL, Value(0), this.location());
		}
		
		if (this.match(Lox.NUMBER)) {
			return Node(Lox.NUMBER, this.previous().value, this.location());
		}
		
		if (this.match(Lox.STRING)) {
			return Node(Lox.STRING, this.previous().value, this.location());
		}
		
		if (this.match(Lox.LEFT_PAREN)) {
			Node left = this.expression();
			this.expect(Lox.RIGHT_PAREN, "Expecting ')' to end expression.");
			return Node(Lox.GROUPING, Value(0), this.location(), left);
		}
		
		throw new ParsingError("Not a valid primary expression.");
		
		return Node(Lox.INVALID, Value(0), this.location());
	}
}

Node[] parse(Token[] content) {
	Parser p = new Parser();
	
	return p.parse(content);
}

InterpreterValue ivNegate(InterpreterValue a) {
	if (a.type == Lox.NUMBER) {
		return InterpreterValue(Lox.NUMBER, Value(-a.value.asNumber));
	}
	
	throw new InterpreterError("Cannot negate this type.");
}

InterpreterValue ivTrue(InterpreterValue a) {
	if (
		(a.type == Lox.BOOLEAN && a.value.asBoolean == false) || 
		(a.type == Lox.NUMBER && a.value.asNumber == 0.0) ||
		(a.type == Lox.NIL)
	) {
		return InterpreterValue(Lox.BOOLEAN, Value(false));
	}
	
	return InterpreterValue(Lox.BOOLEAN, Value(true));
}

InterpreterValue ivOpposite(InterpreterValue a) {
	return InterpreterValue(Lox.BOOLEAN, Value(!a.value.asBoolean));
}

InterpreterValue ivAdd(InterpreterValue a, InterpreterValue b) {
	if (a.type == Lox.NUMBER && b.type == Lox.NUMBER) {
		return InterpreterValue(Lox.NUMBER, Value(a.value.asNumber + b.value.asNumber));
	}
	
	else if (a.type == Lox.STRING && b.type == Lox.STRING) {
		return InterpreterValue(Lox.STRING, Value(format("%s%s", a.value.asString, b.value.asString)));
	}
	
	throw new InterpreterError("Cannot add or concatinate two values of this type.");
}

InterpreterValue ivSub(InterpreterValue a, InterpreterValue b) {
	if (a.type == Lox.NUMBER && b.type == Lox.NUMBER) {
		return InterpreterValue(Lox.NUMBER, Value(a.value.asNumber - b.value.asNumber));
	}
	
	throw new InterpreterError("Cannot subtract two non-number values.");
}

InterpreterValue ivMul(InterpreterValue a, InterpreterValue b) {
	if (a.type == Lox.NUMBER && b.type == Lox.NUMBER) {
		return InterpreterValue(Lox.NUMBER, Value(a.value.asNumber * b.value.asNumber));
	}
	
	throw new InterpreterError("Cannot multiply two non-number values.");
}

InterpreterValue ivDiv(InterpreterValue a, InterpreterValue b) {
	if (a.type == Lox.NUMBER && b.type == Lox.NUMBER) {
		return InterpreterValue(Lox.NUMBER, Value(a.value.asNumber / b.value.asNumber));
	}
	
	throw new InterpreterError("Cannot divide two non-number values.");
}

InterpreterValue ivMod(InterpreterValue a, InterpreterValue b) {
	if (a.type == Lox.NUMBER && b.type == Lox.NUMBER) {
		return InterpreterValue(Lox.NUMBER, Value(cast(double)(cast(long)a.value.asNumber % cast(long)b.value.asNumber)));
	}
	
	throw new InterpreterError("Cannot modulo two non-number values.");
}

InterpreterValue ivGreater(InterpreterValue a, InterpreterValue b) {
	if (a.type == Lox.NUMBER && b.type == Lox.NUMBER) {
		return InterpreterValue(Lox.BOOLEAN, Value(a.value.asNumber > b.value.asNumber));
	}
	
	throw new InterpreterError("Cannot compare two non-number values.");
}

InterpreterValue ivGreaterEq(InterpreterValue a, InterpreterValue b) {
	if (a.type == Lox.NUMBER && b.type == Lox.NUMBER) {
		return InterpreterValue(Lox.BOOLEAN, Value(a.value.asNumber >= b.value.asNumber));
	}
	
	throw new InterpreterError("Cannot compare two non-number values.");
}

InterpreterValue ivLess(InterpreterValue a, InterpreterValue b) {
	if (a.type == Lox.NUMBER && b.type == Lox.NUMBER) {
		return InterpreterValue(Lox.BOOLEAN, Value(a.value.asNumber < b.value.asNumber));
	}
	
	throw new InterpreterError("Cannot compare two non-number values.");
}

InterpreterValue ivLessEq(InterpreterValue a, InterpreterValue b) {
	if (a.type == Lox.NUMBER && b.type == Lox.NUMBER) {
		return InterpreterValue(Lox.BOOLEAN, Value(a.value.asNumber <= b.value.asNumber));
	}
	
	throw new InterpreterError("Cannot compare two non-number values.");
}

InterpreterValue ivEqual(InterpreterValue a, InterpreterValue b) {
	if (a.type == Lox.STRING && b.type == Lox.STRING) {
		return InterpreterValue(Lox.BOOLEAN, Value(a.value.asString == b.value.asString));
	}
	
	return InterpreterValue(Lox.BOOLEAN, Value(a.value.asNumber == b.value.asNumber));
}

InterpreterValue interpret(Node node) {
	switch (node.type) {
		case Lox.NIL: {
			return InterpreterValue(Lox.NIL);
			break;
		}
		
		case Lox.NUMBER:
		case Lox.STRING:
		case Lox.BOOLEAN:
		case Lox.IDENTIFIER: {
			return InterpreterValue(node.type, node.value);
			break;
		}
		
		case Lox.GROUPING: {
			return interpret(node.nodes[0]);
			break;
		}
		
		case Lox.PLUS: {
			InterpreterValue left = interpret(node.nodes[0]);
			InterpreterValue right = interpret(node.nodes[1]);
			return ivAdd(left, right);
			break;
		}
		
		case Lox.MINUS: {
			if (node.nodes.length == 2) {
				InterpreterValue left = interpret(node.nodes[0]);
				InterpreterValue right = interpret(node.nodes[1]);
				return ivSub(left, right);
			}
			else {
				InterpreterValue left = interpret(node.nodes[0]);
				return ivNegate(left);
			}
			break;
		}
		
		case Lox.STAR: {
			InterpreterValue left = interpret(node.nodes[0]);
			InterpreterValue right = interpret(node.nodes[1]);
			return ivMul(left, right);
			break;
		}
		
		case Lox.SLASH: {
			InterpreterValue left = interpret(node.nodes[0]);
			InterpreterValue right = interpret(node.nodes[1]);
			return ivDiv(left, right);
			break;
		}
		
		case Lox.PERCENT: {
			InterpreterValue left = interpret(node.nodes[0]);
			InterpreterValue right = interpret(node.nodes[1]);
			return ivMod(left, right);
			break;
		}
		
		case Lox.BANG: {
			InterpreterValue left = interpret(node.nodes[0]);
			return ivOpposite(ivTrue(left));
			break;
		}
		
		case Lox.GREATER: {
			InterpreterValue left = interpret(node.nodes[0]);
			InterpreterValue right = interpret(node.nodes[1]);
			return ivGreater(left, right);
			break;
		}
		
		case Lox.GREATER_EQUAL: {
			InterpreterValue left = interpret(node.nodes[0]);
			InterpreterValue right = interpret(node.nodes[1]);
			return ivGreaterEq(left, right);
			break;
		}
		
		case Lox.LESS: {
			InterpreterValue left = interpret(node.nodes[0]);
			InterpreterValue right = interpret(node.nodes[1]);
			return ivLess(left, right);
			break;
		}
		
		case Lox.LESS_EQUAL: {
			InterpreterValue left = interpret(node.nodes[0]);
			InterpreterValue right = interpret(node.nodes[1]);
			return ivLessEq(left, right);
			break;
		}
		
		case Lox.EQUAL_EQUAL: {
			InterpreterValue left = interpret(node.nodes[0]);
			InterpreterValue right = interpret(node.nodes[1]);
			return ivEqual(left, right);
			break;
		}
		
		case Lox.BANG_EQUAL: {
			InterpreterValue left = interpret(node.nodes[0]);
			InterpreterValue right = interpret(node.nodes[1]);
			return ivOpposite(ivEqual(left, right));
			break;
		}
		
		default: {
			throw new InterpreterError("Unsupported node type.");
			break;
		}
	}
	
	return InterpreterValue(Lox.INVALID);
}

void interpret_list(Node[] nodes) {
	for (size_t i = 0; i < nodes.length; i++) {
		interpret(nodes[i]).print();
		writeln();
	}
}

class Script {
	Enviornment env;
	
	this() {}
	
	void run(string content) {
		try {
			Token[] tokens = tokenise(content);
			
			Node[] nodes = parse(tokens);
			
			interpret_list(nodes);
		}
		catch (LoxError e) {
			writeln("\033[1;31mERROR\033[0m\n", e.msg);
		}
	}
}

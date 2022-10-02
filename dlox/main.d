module main;

import std.stdio;
import lox;

int main(string[] args) {
	Script script = new Script();
	
	while (true) {
		write("> ");
		
		string code = readln();
		
		if (code == "@exit\n") {
			return 0;
		}
		
		script.run(code);
		
		write("\n");
	}
	
	return 0;
}

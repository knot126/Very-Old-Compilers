# Knot's Programming Langauges

This repo contains some of my first attempts at creating interpreters for simple programming languages. I will be using resources like the guide at [Crafting Interpreters](https://craftinginterpreters.com/) but also probably referring to other resources like the Dragon book and the like.

Currently, this repo contains the following languages:

  * **Honeydew**: This was going to be a very simple C/lox like language, but the interpreter became unmaintainable due to some poor error handling, and I didn't really like not using bytecode and a VM.
  * **Dew**: Basically a renamed version of honeydew, but I also restarted the interpeter to just be in one big file, which can make development simpler.
  * **Drlox**: My own implementation of Lox, from the book *Crafting Interpreters*. I am currently still just planning on making it, but I might stop to restart the book so I can focus on making that before I try making my own language.

## Honeydew Language

The **Honeydew Language** is my first formal attempt at partially designing and writing an interpreted programming language. 

The language will most likely be C-like, since I quite like those and I am fimilar with them. I would like to keep things fairly simple but I might try to go out and add extra things to it.

## Dew Language

The **Dew** language is basically the same as the honeydew language.

### Examples (may not work yet)

```c
void fib(n) {
	/**
	 * Print fibseq up to n.
	 */
	
	int i = 0;
	int j = 1;
	
	for (int i = 0; i < n; i++) {
		j = i + j;
		i = i + j;
		
		print("Numbers: ", i, j, "\n");
	}
}

fib(7);
```

## Drlox

There is not much to say about **Drlox** yet; it's short for *Dragon's Lox*.

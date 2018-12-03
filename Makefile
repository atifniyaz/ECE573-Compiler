compiler:
	flex micro.lex
	bison -d -o micro.c micro.y
	g++ -std=c++14 -Wall -g -lfl -o micro lex.yy.c micro.c main.cpp SymbolTableStack.cpp Identifier.cpp LLString.cpp SymbolTable.cpp

clean:
	rm -f lex.yy.c
	rm -f micro
	rm -f datastructures.o
	rm -f micro.c
	rm -f micro.h
	rm -f *.hpp.gch
team:
	echo "Team: Atif Niyaz\n\nAtif Niyaz\natifniyaz"

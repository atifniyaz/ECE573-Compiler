compiler:
	flex micro.lex
	bison -d -o micro.c micro.y
	gcc lex.yy.c micro.c -lfl -o micro

clean:
	rm -f lex.yy.c
	rm -f micro
	rm -f micro.c
	rm -f micro.h
team:
	echo "Team: Atif Niyaz\n\nAtif Niyaz\natifniyaz"

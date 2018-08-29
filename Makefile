compiler:
	flex micro.lex
	gcc lex.yy.c -lfl -o micro

clean:
	rm -f lex.yy.c
	rm -f micro

team:
	echo "Team: Atif Niyaz\n\nAtif Niyaz\natifniyaz"

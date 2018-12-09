CPPFLAGS = -std=c++0x -Wall -g
GPP = g++ $(CPPFLAGS)

compiler:
	flex micro.lex
	bison -d -o micro.c micro.y
	$(GPP) -c *.cpp
	$(GPP) -o micro lex.yy.c micro.c *.o -lfl

clean:
	rm -f lex.yy.c
	rm -f micro
	rm -f *.o
	rm -f micro.c
	rm -f micro.h
	rm -f *.hpp.gch

team:
	echo "Team: Atif Niyaz\n\nAtif Niyaz\natifniyaz"

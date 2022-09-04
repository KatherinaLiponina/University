all: grammatic.tab.o lex.yy.o proof_part.o
	g++ lex.yy.o grammatic.tab.o proof_part.o -o parser
grammatic.tab.o: grammatic.tab.c
	g++ -c grammatic.tab.c
lex.yy.o: lex.yy.c
	g++ -c lex.yy.c
proof_part.o: proof_part.cpp
	g++ -c proof_part.cpp
run: 
	./parser

#include <iostream>
#include <string>
#include <list>
#include <iterator>
#include <map>
using namespace std;

enum Operation { Implication, Disjunction, Conjunction, Negation, Variable };

struct Tree {
    string name;
    Tree * left;
    Tree * right;
    int value;
    Operation op;

    Tree(Tree* _l, Tree* _r, string _name, Operation _o) : left(_l), right(_r), name(_name), op(_o) {} //конструктор

    string right_string(void);
    bool axiom(void);
    bool equal(Tree *);
};

//int implication(int, int);
string delete_space(string);
Tree* newTree(Tree*, Tree*, string, Operation);

int fhypothesis(string);
int process(Tree *);
void print_proof(string);
void add_bk(string);
string find_modus_ponens(string);
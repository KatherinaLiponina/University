#include <iostream>
#include <string.h>
#include <ctime>
using namespace std;

enum Operation { expression, disjuncture, conjunction, negative, variable };
string names[16];
int display[16];
int count = 0;

int implication(int a, int b);

struct Tree {
    Tree* left;
    Tree* right;
    string name;
    int number = 0;
    Operation op; 

    Tree (Tree* _l, Tree* _r, string _name, Operation _o) : left(_l), right(_r), name(_name), op(_o) {} //конструктор

    int to_value(){
        switch (op) {
			case disjuncture: return left->to_value() || right->to_value();
			case conjunction: return left->to_value() && right->to_value();
			case variable: return display[number];
            case negative: return right->to_value() ? 0 : 1; 
            case expression: return implication(left->to_value(), right->to_value());
			default: return -1;
		}
    }
    
    int counter(){
        if (name != ""){
            for (int i = 0; i < count; i++){
                if (name == names[i]){
                    number = i;
                    goto done;
                }
            }
            names[count] = name;
            number = count;
            count++;
        }
        done:
        if (left != NULL){
            left->counter();
        }
        if (right != NULL){
            right->counter();
        }
        return count;
    }
};

struct Parser {
    int pos;
    string str;

    Tree* parse (string s) {
		str = s + ";";
		pos = 0;

		return parse_expression();
	}

    Tree* parse_variable(){
        string var = "";
        while (str[pos] >= 'A' && str[pos] <= 'Z' || str[pos] >= '0' && str[pos] <= '9' || str[pos] == '\''){
            var += str[pos];
            pos++;
        }
        if (var != ""){
            return new Tree(NULL, NULL, var, variable);
        }
        else {
            cout << "???" << endl;
            exit(-1);
        }
    }

    Tree* parse_negation(){
        if (str[pos] == '!'){
            pos++;
            Tree* res = parse_negation();
            return new Tree(NULL, res, "", negative);
        }
        if (str[pos] == '('){
            pos++;
            Tree* res = parse_expression();
            if (str[pos] != ')'){
                cout << "???" << endl;
                exit(1);
            }
            pos++;
            return res;
        }
        Tree* res = parse_variable();
        return res;
    }

    Tree* parse_conjunction(){
        Tree* left = parse_negation();
        if (str[pos] == '&'){
            pos++;
            Tree* right = parse_conjunction();
            return new Tree(left, right, "", conjunction);
        }
        return left;
    }

    Tree* parse_disjuncture(){
        Tree* left = parse_conjunction();
        if (str[pos] == '|'){
            pos++;
            Tree* right = parse_disjuncture();
            return new Tree(left, right, "", disjuncture);
        }
        return left;
    }

    Tree* parse_expression(){
        Tree* left = parse_disjuncture();
        if (str[pos] == '-'){
            pos += 2;
            Tree* right = parse_expression();
            return new Tree(left, right, "", expression);
        }
        return left;
    }

};

int delete_space(char* str);

int main(void){
    //clock_t begin = clock();
    for (int i = 0; i < 16; i++)
        display[i] = 0;
    char str[257];
    cin.getline(str, 257);
    delete_space(str);
    Parser p;
    Tree* tree = p.parse(str);
    tree->counter();
    int result = 0, loop = 1;
    for (int i = 0; i < count; i++) //2^count
        loop *= 2;
    double avg = 0;
    for (int i = 0; i < loop; i++){
        for (int j = 0; j < count; j++){
            display[j] = (i >> j) % 2;
        }
        int res = tree->to_value();
        result += res;
    }
    if (result == loop){
        cout << "Valid" << endl;
    }
    else if (result == 0){
        cout << "Unsatisfiable" << endl;
    }
    else {
        cout << "Satisfiable and invalid, " << result << " true and " << loop - result << " false cases" << endl;
    }
    //clock_t all_end = clock();
    //cout << "time: " << (double)(all_end - begin)/CLOCKS_PER_SEC << endl;
    return 0;
}

int implication(int a, int b){
    if (a == 0){
        return 1;
    }
    if (b == 1)
        return 1;
    return 0;
}

int delete_space(char* str){
    if (str == NULL)
        return -1;
    int len = 0;
    while (str[len++] != '\0'); len--;
    for (int i = 0; i < len; i++){
        if (*(str + i) == ' ' || *(str + i) == '\t' || *(str + i) == '\n' || *(str + i) == '\r'){
            for (int j = i; j < len; j++){
                str[j] = str[j+1];
            }
            i--;
            len--;
        }
    }
    return len;
}
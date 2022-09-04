#include <iostream>
#include <cstring>
using namespace std;

enum Operation { Negation, Disjunction, Conjunction, Implication, Variable };
string names[16];
int display[16];
int Count;

int implication(int a, int b);
bool is_axiom(char*);
string delete_space(string str);

struct tree_string {
	string str;
	tree_string* left;
	tree_string* right;
	Operation op;
	int number;

	tree_string(string s, tree_string* l, tree_string* r, Operation o) : str(s), left(l), right(r), op(o) {}

    string to_string() {
		switch (op) {
		case Disjunction: return string("(|,") + left->to_string() + "," + right->to_string() + ")";
		case Conjunction: return string("(&,") + left->to_string() + "," + right->to_string() + ")";
		case Variable: return str;
		case Negation: return string("(!" + left->to_string() + ")");
		case Implication: return string("(->," + left->to_string() + "," + right->to_string() + ")");
		default: return "????";
		}
	}

	string right_string() {
		switch (op) {
		case Implication: return string("(") + left->right_string() + "->" + right->right_string() + ")";
		case Disjunction: return string("(") + left->right_string() + "|" + right->right_string() + ")";
		case Conjunction: return string("(") + left->right_string() + "&" + right->right_string() + ")";
		case Negation: return string("!") + left->right_string();
		case Variable: return string(str);
		default: return "????";
		}
	}

	int counter() {
		if (str != "") {
			for (int i = 0; i < Count; i++) {
				if (str == names[i]) {
					number = i;
					goto done;
				}
			}
			int i = 0;
			names[Count] = str;
			number = Count;
			Count++;
		}
	done:
		if (left != NULL) {
			left->counter();
		}
		if (right != NULL) {
			right->counter();
		}
		return Count;
	}

	int to_value() {
		switch (op) {
		case Disjunction:
			return left->to_value() || right->to_value();
		case Conjunction:
			return left->to_value() && right->to_value();
		case Variable:
			return display[number];
		case Negation:
			return left->to_value() ? 0 : 1;
		case Implication:
			return implication(left->to_value(), right->to_value());
		default:
			return -1;
		}
	}

};

tree_string* parse_string(string str) { //parse_string портит строку!
	int len = str.length();
    string copy = str;
start:
	if (copy[0] == '(') {
		int braket = 1;
		for (int i = 1; i < len - 1; i++) {
			if (copy[i] == ')') {
				braket--;
			}
			else if (copy[i] == '(') {
				braket++;
			}
			if (braket == 0) {
				goto no;
			}
		}
        copy.erase(len - 1, 1);
		copy.erase(0, 1);
        len = copy.length();
		goto start;
	}
no:
	int place_i = -1, place_d = -1, place_c = -1, place_n = -1, inside = 0;
	for (int i = 0; i < len; i++) { 
		switch (copy[i])
		{
		case '(':
			inside++;
			break;
		case ')':
			inside--;
			break;
		case '-':
			if (inside == 0 && place_i == -1) {
				place_i = i;
			}
			break;
		case '&':
			if (inside == 0) {
				place_c = i;
			}
			break;
		case '|':
			if (inside == 0) {
				place_d = i;
			}
			break;
		case '!':
			if (inside == 0) {
				place_n = i;
			}
			break;
		default:
			break;
		}
	}
	string lsub = "",rsub = "";
	int stop = 0;	
	Operation o;
	if (place_i != -1) {
		stop = place_i;
		o = Implication;
	}
	else if (place_d != -1) {
		stop = place_d;
		o = Disjunction;
	}
	else if (place_c != -1) {
		stop = place_c;
		o = Conjunction;
	}
	else if (place_n != -1) {
		stop = place_n;
		o = Negation;
	}
	else { //variable
		int l = copy.length();
		string temp = copy;
		return new tree_string(temp, NULL, NULL, Variable);
	}
	if (o == Negation) {
		int len = copy.length();
		string temp = copy;
        temp.erase(0,1);
		tree_string* l = parse_string(temp);
		return new tree_string("", l, NULL, Negation);
	}
	int start;
	if (o == Implication)
		start = stop + 2;
	else
		start = stop + 1;
	for (int i = 0; i < stop; i++) {
		lsub += copy[i];
	}
	for (int i = 0; i < len - start; i++) {
		rsub += copy[i + start];
	}
	//cout << "lsub " << lsub << endl;
	//cout << "rsub " << rsub << endl;
	tree_string* l = parse_string(lsub);
	tree_string* r = parse_string(rsub);
	return new tree_string("", l, r, o);
}

bool is_axiom(string str) {
	if (str == "") {
		cout << "in function is_axiom" << endl;
		cout << "error: wrong string" << endl;
		return false;
	}
	string temp = str;
	Count = 0;
	temp = delete_space(temp);
	for (int i = 0; i < 16; i++) {
		display[i] = 0;
		names[i] = "";
	}
	tree_string* tree = parse_string(temp);
	//cout << tree->right_string() << endl;
	tree->counter();
	int result = 0, loop = 1;
	for (int i = 0; i < Count; i++) //2^count
		loop *= 2;
	for (int i = 0; i < loop; i++) {
		for (int j = 0; j < Count; j++) {
			display[j] = (i >> j) % 2;
		}
		int res = tree->to_value();
		result += res;
	}
	if (result == loop)
		return true;
	else
		return false;
}

int implication(int a, int b) {
	if (a == 0) {
		return 1;
	}
	if (b == 1)
		return 1;
	return 0;
}

string delete_space(string str) {
	if (str == "")
		return "";
	string temp = str;
    int pos;
    while ((pos = temp.find(" ")) != -1){
        temp.erase(pos, 1);
    }
    while ((pos = temp.find("\n")) != -1){
        temp.erase(pos, 1);
    }
    while ((pos = temp.find("\t")) != -1){
        temp.erase(pos, 1);
    }
    while ((pos = temp.find("\r")) != -1){
        temp.erase(pos, 1);
    }
    return temp;
}


int main(){
    string str;
    getline(cin, str);
    str = delete_space(str);
    tree_string* tree = parse_string(str);
    cout << tree->to_string() << endl;
    //cout << is_axiom(str) << endl;

    return 0;
}
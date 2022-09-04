#include "proof_part.hpp"

string alpha;

string Tree::right_string(void){
    switch(op){
        case Implication: return string("(") + left->right_string() + string("->") + right->right_string() + string(")");
        case Disjunction: return string("(") + left->right_string() + string("|") + right->right_string() + string(")");
        case Conjunction: return string("(") + left->right_string() + string("&") + right->right_string() + string(")");
        case Negation: return string("!") + left->right_string();
        case Variable: return name;
        default: return string("?????");
    }
}

Tree* newTree(Tree* l, Tree* r, string n, Operation o){
    Tree * tree = new Tree(l, r, n, o);
    return tree;
}

bool Tree::equal(Tree * tree){// 0 - false, 1 - true
    if (op != tree->op)
		return false;
	if (op == Variable){
		if (name == tree->name)
			return true;
		return false;
	}
	int l = -1, r = -1;
	if (left != NULL && tree->left != NULL)
		l = left->equal(tree->left);
	if (l == 0 || l == -1)
		return false;
	if (right != NULL && tree->right != NULL)
		r = right->equal(tree->right);
	if (r == 0)
		return false;
	return true;
}

bool Tree::axiom(void){
    //#1 A->B->A
	if (op == Implication){
		if (right->op == Implication){
			if (left->equal(right->right)){
				//cout << "1" << endl;
				return true;
			}	
		}
	}
    //#2 (A->B)->(A->B->Y)->(A->Y)
	if (op == Implication && left->op == Implication && right->op == Implication){
		if (right->right->op == Implication && right->left->op == Implication){
			if (right->left->right->op == Implication){
				if (left->left->equal(right->left->left) && left->left->equal(right->right->left)){
					if (left->right->equal(right->left->right->left)){
						if (right->right->right->equal(right->left->right->right)){
							//cout << "2" << endl;
							return true;
						}
					}
				}
			}
		}
	}
    //#3 A->B->A&B
	if (op == Implication){
		if (right->op == Implication && right->right->op == Conjunction){
			if (left->equal(right->right->left) && right->left->equal(right->right->right)){
				//cout << "3" << endl;
				return true;
			}
		}
	}
    //#4 A&B->A, #5 A&B->B
	if (op == Implication && left->op == Conjunction){
		if (right->equal(left->left) || right->equal(left->right)){
			//cout << "4/5" << endl;
			return true;
		}		
	}
    //#6 A->A|B, #7 B->A|B
	if (op == Implication && right->op == Disjunction){
		if (left->equal(right->left) || left->equal(right->right)){
			//cout << "6/7" << endl;
			return true;
		}	
	}
    //#8 (A->Y)->(B->Y)->(A|B->Y)
	if (op == Implication && left->op == Implication && right->op == Implication){
		if (right->left->op == Implication && right->right->op == Implication && right->right->left->op == Disjunction){
			if (left->left->equal(right->right->left->left)){
				if (right->left->left->equal(right->right->left->right)){
					if (left->right->equal(right->left->right) && left->right->equal(right->right->right)){
						//cout << "8" << endl;
						return true;
					}
				}
			}
		}
	}
    //#9 (A->B)->(A->!B)->!A
	if (op == Implication && left->op == Implication && right->op == Implication){
		if (right->right->op == Negation && right->left->op == Implication && right->left->right->op == Negation){
			if (left->left->equal(right->right->left) && left->left->equal(right->left->left)){
				if (left->right->equal(right->left->right->left)){
					//cout << "9" << endl;
					return true;
				}
			}
		}
	}
    //#10 !!A->A
	if (op == Implication && left->op == Negation && left->left->op == Negation){
		if (right->equal(left->left->left)){
			//cout << "10" << endl;
			return true;
		}	
	}
	return false;
}

string delete_space(string str) {
	if (str == "")
		return "";
	string temp = str;
    int pos;
    while ((pos = temp.find(" ")) != -1){
        temp.erase(pos, 1);
    }
    while ((pos = temp.find("\t")) != -1){
        temp.erase(pos, 1);
    }
    while ((pos = temp.find("\n")) != -1){
        temp.erase(pos, 1);
    }
    while ((pos = temp.find("\r")) != -1){
        temp.erase(pos, 1);
    }
    return temp;
}

map <string, int> like_bj;
multimap <string, string> like_bk;
list <string> list_hypothesis;
Tree * current_tree;

int fhypothesis(string str){
	list <string>::iterator it = list_hypothesis.begin();
	int len = list_hypothesis.size();
	if (len != 0){
		cout << (*it); 
		for (it++; it != list_hypothesis.end(); it++){
			cout << "," << (*it);
		}
	}
	cout << "|-" << alpha << "->" << str << endl;
	return 0;
}

int process(Tree * tree){
	static int counter = 0;
	current_tree = tree;
	string str = tree->right_string();
	print_proof(str);
	cout << alpha << "->" << str << endl;
	like_bj[str] = counter++;
	add_bk(str);
	return 0;
}

void add_bk(string str){
	int len = str.size(), bracket = 0, pos = -1;
	for (int i = 1; i < len - 1; i++){
		if (str[i] == '('){
			bracket++;
		}
		else if (str[i] == ')'){
			bracket--;
		}
		else if (str[i] == '-' && str[i+1] == '>' && bracket == 0){
				pos = i;
				break;
		}	
	}
	if (pos == -1)
		return;
	string bj = "", bi = "";
	for (int i = 1; i < pos; i++){
		bj += str[i];
	}
	for (int i = pos + 2; i < len - 1; i++){
		bi += str[i];
	}
	like_bk.insert(pair<string,string>(bi,bj));
	return;
}

void print_proof(string str){
	//1
	//cout << "str = " << str << endl;
	//cout << "alpha = " << alpha << endl;
	if (str == alpha){
		//cout << "alpha" << endl;
		cout << alpha << "->(" << alpha << "->" << alpha << ")" << endl;

        cout << "(" << alpha << "->(" << alpha << "->" << alpha << "))->(" << alpha;
        cout << "->(" << alpha << "->" << alpha << ")->" << alpha << ")->(" << alpha;
        cout << "->" << alpha << ")" << endl;

        cout << "(" << alpha << "->(" << alpha << "->" << alpha << ")->" << alpha;
        cout << ")->(" << alpha << "->" << alpha << ")" << endl;

        cout << alpha << "->(" << alpha << "->" << alpha << ")->" << alpha << endl;
        return;
	}
	//2
	list <string>::iterator it;
	//cout << list_hypothesis.size() << endl;
	if (list_hypothesis.size() != 0){
		for (it = list_hypothesis.begin(); it != list_hypothesis.end(); it++){
			//cout << "Hypothesis: " << (*it) << endl;
			if ((*it) == str){
				//cout << "hypothesis" << endl;
				cout << str << "->" << alpha << "->" << str << endl;
            	cout << str << endl;
            	return;
			}
		}
	}
	//3
	if (current_tree->axiom()){
		//cout << "axiom" << endl;
		cout << str << "->" << alpha << "->" << str << endl;
        cout << str << endl;
        return;
	}
	//4
	string bj = find_modus_ponens(str);
	//cout << "modus ponens " << bj << endl;
    if (bj != ""){
        cout << "(" << alpha << "->" << bj << ")->(" << alpha << "->" << bj << "->" << str << ")->(" << alpha << "->" << str << ")" << endl;
        cout << "(" << alpha << "->" << bj << "->" << str << ")->(" << alpha << "->" << str << ")" << endl;
        
		return;
    }
	cout << "error: incorrect proof" << endl;
	exit(1);
}

string find_modus_ponens(string bi){
	pair<multimap<string,string>::iterator,multimap<string,string>::iterator> it;
    multimap<string,string>::iterator i;
    map <string, int> :: iterator iit;
    it = like_bk.equal_range(bi); //i->second = bj
    for (i = it.first; i != it.second; i++){
		//string tmp = string("(") + bi + string("->") + i->second + string(")");
        iit = like_bj.find(i->second);
        if (iit != like_bj.end()){
            return i->second;
        }
    }
    return "";
}


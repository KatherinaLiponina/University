#include <iostream>
#include <map>
#include "Parser.h"

string alpha; //та, что мы переносим через |- 
int len_alpha;
map <string, int> mp;
multimap <string, string> Bi_Bj;
//clock_t MP = 0;
//int MP_r = 0, ST = 0;

struct List {
    List* next;
    string str;
    List(List* _n, string s) {
        next = _n;
        str = s;
    }
    List() {
        next = NULL;
        str = "";
    }
    int print() {
        cout << str << endl;
        List* p = next;
        while (p) {
            cout << p->str << endl;
            p = p->next;
        }
        return 0;
    }
};

string make_right(string str){
    string temp = str;
    tree_string* tree = parse_string(temp);
    temp = tree->right_string();
    clean(tree);
    return temp;
}

List* parse_hypothesis(string s, int pos) {
    string hypothesis = "";
    while (s[pos] != ',' && (s[pos] != '|' || s[pos + 1] != '-')) {
        if (s[pos] == ' ' || s[pos] == '\n' || s[pos] == '\t' || s[pos] == '\r') {
            pos++;
            continue;
        }
        hypothesis += s[pos];
        pos++;
    }
    hypothesis = make_right(hypothesis);
    if (s[pos] == '|') {
        alpha = hypothesis;
        len_alpha = alpha.length();
        return new List(NULL, hypothesis);
    }
    else {
        pos++;
        List* p = parse_hypothesis(s, pos);
        return new List(p, hypothesis);
    }
}

void new_first_line(string str) {
    int len = str.length();
    string new_one = "";
    int i = 0;
    while(str[i] != '|' || str[i + 1] != '-'){
        new_one += str[i];
        i++;
    }
    int pos = new_one.rfind(",");
    if (pos == -1){
        pos = 0;
    }
    int l = new_one.length();
    new_one.erase(pos, l - pos);
    new_one += "|-";
    i += 2;
    new_one += alpha;
    new_one += "->";
    pos = str.find("|-");
    pos += 2;
    for (int i = 0; i < len - pos; i++){
        new_one += str[i + pos];
    }
    cout << new_one << endl;
}

/*string find_modus_ponens(string bi, List* start, int num) {
    clock_t st = clock(), en;
    //cout << "try modus ponens" << endl;
    string bj = "";
    List* p = start;
    int n = 0;
    string substring = "";
    map <string,int> :: iterator it = mp.begin();
    while (n < num){
        substring += "(";
        substring += p->str;
        substring += ")->(";
        substring += bi;
        substring += ")";
        substring = make_right(substring);
        it = mp.find(substring);
        if (it != mp.end()){
            en = clock();
            MP += (en - st);
            MP_r++;
            return p->str;
        }
        substring = "";
        n++;
        p = p->next;
    }
    en = clock();
    MP += (en - st);
    return "";
}*/

string find_modus_ponens(string bi, List* start, int num){
    pair<multimap<string,string>::iterator,multimap<string,string>::iterator> it;
    multimap<string,string>::iterator i;
    map <string, int> :: iterator iit;
    it = Bi_Bj.equal_range(bi); //i->second = bj
    for (i = it.first; i != it.second; i++){
        iit = mp.find(i->second);
        if (iit != mp.end()){
            return i->second;
        }
    }
    return "";
}

void what_path(string str, List* start, int num, List* hyp) { //перенести мп после всего, перед аксиомой
    //1 - аксиома, 2 - гипотеза, 3 - альфа, 4 - Modus Ponens
    if (str == alpha){
        //ST+=4;
        //cout << "alpha" << endl;
        //choise_alpha(str);
        // a->(a->a)
        // (a->(a->a))->(a->(a->a)->a)->(a->a)
        // (a->(a->a)->a)->(a->a)
        // a->(a->a)->a
        cout << alpha << "->(" << alpha << "->" << alpha << ")" << endl;

        cout << "(" << alpha << "->(" << alpha << "->" << alpha << "))->(" << alpha;
        cout << "->(" << alpha << "->" << alpha << ")->" << alpha << ")->(" << alpha;
        cout << "->" << alpha << ")" << endl;

        cout << "(" << alpha << "->(" << alpha << "->" << alpha << ")->" << alpha;
        cout << ")->(" << alpha << "->" << alpha << ")" << endl;

        cout << alpha << "->(" << alpha << "->" << alpha << ")->" << alpha << endl;
        return;
    }
    List* p = hyp;
    while (p) {
        if (p->str == str) {
            //ST+=2;
            //cout << "hypothesis" << endl;
            //choise_axiom(str);
            // str->A->str
            // str
            cout << str << "->" << alpha << "->" << str << endl;
            cout << str << endl;
            return;
        }
        p = p->next;
    }
    tree_string* t = parse_string(str);
    if (is_axiom(t)){
        //ST+=2;
        //cout << "axiom" << endl;
        //choise_axiom(str);
        // str->A->str
        // str
        cout << str << "->" << alpha << "->" << str << endl;
        cout << str << endl;
        return;
    }
    clean(t);
    if (num > 1) {
        string bj = find_modus_ponens(str, start, num);
        if (bj != ""){
            //ST+=2;
            //cout << "modus_ponens" << endl;
            //choise_modus_ponens(str, bj);
            // str = bi
            // (a->bj)->(a->bj->bi)->(a->bi)
            // (a->bj->bi)->(a->bi)
            cout << "(" << alpha << "->" << bj << ")->(" << alpha << "->" << bj << "->" << str << ")->(" << alpha << "->" << str << ")" << endl;
            cout << "(" << alpha << "->" << bj << "->" << str << ")->(" << alpha << "->" << str << ")" << endl;
            return;
        }
    }

    cout << "error: all paths are wrong on line " << str << endl;
    exit(1);
}

void map_adder(string str){
    int len = str.length();
    int imp = -1, inside = 0;
    for (int i = 1; i < len - 1; i++){
        if (str[i] == '(')
            inside++;
        else if (str[i] == ')')
            inside--;
        else if(str[i] == '-' && inside == 0){
            imp = i;
            break;
        }
    }
    if (imp == -1){
        return;
    }
    string bi = "";
    for (int i = imp + 2; i < len - 1; i++){
        bi += str[i];
    }
    string bj = "";
    for (int i = 1; i < imp; i++){
        bj += str[i];
    }
    Bi_Bj.insert(pair<string,string>(bi,bj));
    return;
}

int main(){
    //clock_t strt = clock();
    string hypothesis;
    getline(cin, hypothesis);
    hypothesis = delete_space(hypothesis);
    List* hyp = parse_hypothesis(hypothesis, 0);
    new_first_line(hypothesis);
    string str;
    int i = 0;
    getline(cin, str);
    str = make_right(str);
    List start(NULL, str);
    mp[str] = i;
    i++;
    what_path(str, &start, i, hyp);
    cout << alpha << "->" << str << endl;
    map_adder(str);
    List* ptr = &start;
    getline(cin, str);
    while (str != ""){
        str = delete_space(str);
        str = make_right(str);
        List* p = new List(NULL, str);
        what_path(str, &start, i, hyp);
        cout << alpha << "->" << str << endl;
        mp[str] = i;
        i++;
        map_adder(str);
        ptr->next = p;
        ptr = p;
        //ST++;
        getline(cin, str);
    }
    

    /*int num = 0;
    ptr = &start;
    while(ptr){
        what_path(ptr->str, &start, num, hyp);
        cout << alpha << "->" << ptr->str << endl;
        ptr = ptr->next;
        num++;
        ST++;
    }*/
    /*clock_t end = clock();
    cout << "итог" << endl;
    cout << "времени затрачено: " << double(end-strt)/CLOCKS_PER_SEC << endl;
    cout << "в том числе на MP: " << double(MP)/CLOCKS_PER_SEC << endl;
    cout << "что составляет " << (double(MP))*100/(double(end-strt)) << "%" << endl;
    cout << "MP было " << MP_r << endl;
    cout << "строк всего (в исходном доказательстве) " << i << endl;
    cout << "строк всего (в получившемся доказательстве) " << ST << endl;*/
    return 0;
}
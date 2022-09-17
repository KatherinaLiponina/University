#include <iostream>
#include <fstream>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string>
#include <cstring>
#include <dirent.h>
#include <regex>
#include <sys/ioctl.h>
#include <linux/fs.h>
#include <sys/inotify.h>
#include <signal.h>

using namespace std;

int init();
int add_file(int argc, char ** argv);
int close(string password);
int create_demon();

//sudo chown root:root lock
//sudo chmod 6711 lock

main(int argc, char ** argv) {
    if (argc < 2) {
        cout << "Using: lock command " << endl;
        cout << "command:\n\tinit - for start\n";
        cout << "\tadd <file> - under control\n";
        cout << "\tclose <password> - end application\n";
        return 0;
    }
    
    int result;
    string command = argv[1];

    if (command == "init") {
        result = init(); 
        if (result == 1) {
            remove("template.tbl");
        }
    }
    else if (command == "add") {
        if (argc == 2) {
            cout << "Sorry, need a file name" << endl;
            return 0;
        }
        result = add_file(argc, argv);
    }
    else if (command == "close") {
        if (argc == 2) {
            cout << "Sorry, need a password" << endl;
            return 0;
        }
        result = close(argv[2]);
    }
    else {
        cout << "Cannot recognize command!" << endl;
        return 0;
    }

    if (result == 0) {
        cout << "Task executed successfully" << endl;
    }
    else {
        cout << "An error occured during execution" << endl;
    }

    return 0;
}

int set_root(string filename);
int set_user(string filename);
int match(string filename, string template_form);
int sort_out_dir(string, int (*) (string));
int get_parent_uid(uid_t * uid, gid_t * gid);
int protect_dir();

int init() {
    string dir = get_current_dir_name();
    string template_string, password;
    DIR * working_directory = opendir(dir.c_str());
    string template_filename = dir + "/template.tbl";
    fstream template_file(template_filename.c_str(), fstream::in | std::fstream::out);
    if (!template_file.is_open()) {
        ofstream ofs(template_filename);
        ofs.close();
        template_file.open(template_filename.c_str(), fstream::in | std::fstream::out);
        if (!template_file.is_open()) {
            cout << "Cannot create \"template.tbl\"" << endl;
            return 1;
        }
        cout << "Enter template: ";
        cin >> template_string;
        cout << "Enter password: ";
        cin >> password;
        hash<string> hs;
        password = hs(password);
        template_file << password << '\n' << template_string << endl;
    }
    else {
        template_file >> password >> template_string;
    }
    //cout << password << template_string << endl;

    std::regex_error paren(std::regex_constants::error_paren);
    try {
        std::regex rx(template_string);
    }
    catch (const std::regex_error& rerr) {
        std::cout << "regex error: " << rerr.code() << std::endl;
        return 1;
    }

    if (protect_dir() == 1) {
        return 1;
    }
    set_root(template_filename);

    //поиск файлов по шаблону
    sort_out_dir(template_string, set_root);
    //file-storage names
    string name;
    while (template_file >> name) {
        set_root(name);
    }

    template_file.close();
    closedir(working_directory);
    if (create_demon() == 1) {
        return 1;
    }

    return 0;
}
int add_file(int argc, char ** argv){
    string dir = get_current_dir_name();
    string template_filename = dir + "/template.tbl";
    fstream template_file(template_filename.c_str(), fstream::in | std::fstream::out);
    if (!template_file.is_open()) {
        cout << "Really unexpected error!" << endl;
        return 1;
    }
    string filepass, filetemplate;
    template_file >> filepass >> filetemplate;
    for (int i = 2; i < argc; i++) {
        if (match(argv[i], filetemplate) == 0) {
            set_root(argv[i]);
            continue;
        }
        string name;
        while (template_file >> name) {
            if (strcmp(name.c_str(), argv[i]) == 0) {
                break;
            }
        }
        if (!(template_file >> name)) {
            //добавить в файл
            string str = argv[i];
            std::ofstream file;                   
            file.open(template_filename, std::ios::app); 
            file << str << endl;            
            file.close();

            if (set_root(str) == 1) {
                cout << "File does not exist! (but was added under control)" << endl;
                cout << "(" << str << ")" << endl;
            }
        }
        template_file.seekg(ios::beg);
        template_file >> filepass >> filetemplate;
    }
    template_file.close();
    return 0;
}

int close(string password) {

    //todo: найти процесс по его pid
    FILE * lookingForDemon = popen("ps -aux | grep \"./lock init\"", "r");
    char buffer[255];
    fread(buffer, 254, 254, lookingForDemon);
    int placeNumber = 0;
    pid_t demonPid = 0;
    while (buffer[placeNumber] < '0' || buffer[placeNumber] > '9') { placeNumber++; }
    while (buffer[placeNumber] >= '0' && buffer[placeNumber] <= '9') {
        demonPid = demonPid * 10 + (int)(buffer[placeNumber] - '0');
        placeNumber++;
    }
    pclose(lookingForDemon);
    //todo: убить процесс по его pid
    if (kill(demonPid, SIGTERM) == -1) {
        perror("kill");
    }

    string dir = get_current_dir_name();
    string template_filename = dir + "/template.tbl";
    fstream template_file(template_filename.c_str(), fstream::in | std::fstream::out);
    if (!template_file.is_open()) {
        cout << "Really unexpected error. We have a root around!" << endl;
        return 1;
    }
    string filepass, filetemplate;
    template_file >> filepass;
    hash<string> hs;
    password = hs(password);
    if (filepass != password) {
        cout << "Sorry, wrong password :)" << endl;
        return 0;
    }
    template_file >> filetemplate;
    set_user(dir);
    sort_out_dir(filetemplate, set_user);
    set_user(template_filename);

    string name;
    while (template_file >> name) {
        set_user(name);
    }
    set_user(template_filename);
    
    return 0;
}

int set_root(string filename) {
    
    if (chown(filename.c_str(), 0, 0) == -1) {
        //perror("Cannot change owner");
        return 1;
    }
    if (chmod(filename.c_str(), 01700) == -1) {
        //perror("Cannot change mode");
        return 1;
    }
    //todo: set immutable
    string template_file = get_current_dir_name();
    template_file += "/template.tbl";
    if (filename == template_file) {
        return 0;
    }
    return 0;
}

int protect_dir() {
    string dir = get_current_dir_name();
    if (chown(dir.c_str(), 0, 0) == -1) {
        perror("Cannot change owner");
        return 1;
    }
    if (chmod(dir.c_str(), 01777) == -1) {
        perror("Cannot change mode");
        return 1;
    }
}

int set_user(string filename) {
    uid_t uid; gid_t gid;
    if (get_parent_uid(&uid, &gid)) {
        return 1;
    }
    if (chown(filename.c_str(), uid, gid) == -1) {
        //perror("Cannot change owner");
        return 1;
    }
    if (chmod(filename.c_str(), 0775) == -1) {
        //perror("Cannot change mode");
        return 1;
    }
    return 0;
}

int match(string filename, string template_form) {
    regex checher(template_form);
    if (regex_match(filename.cbegin(), filename.cend(), checher)) {
        //cout << filename << endl;
        return 0;
    }
    return 1;
}

int sort_out_dir(string template_string, int (*set_function) (string)) {
    string dir = get_current_dir_name();
    DIR * working_directory = opendir(dir.c_str());
    struct dirent * block = readdir(working_directory);
    while (block) {
        //cout << block->d_name << endl;
        if (block->d_type == DT_REG && strcmp(block->d_name, "lock") && strcmp(block->d_name, "template.tbl")) {
            if (match(block->d_name, template_string) == 0) {
                //cout << block->d_name << endl;
                set_function(dir + "/" + block->d_name);
            }
        }
        //cout << block->d_name << endl;
        block = readdir(working_directory);
    }
    return 0;
}

int get_parent_uid(uid_t * uid, gid_t * gid) {
    static uid_t dir_uid = -1;
    static gid_t dir_gid = -1;
    if (dir_uid != -1 && dir_gid != -1) {
        *uid = dir_uid;
        *gid = dir_gid;
        return 0;
    }
    string dir = get_current_dir_name();
    dir += "/..";
    struct stat st;
    if (stat(dir.c_str(), &st) == -1) {
        perror("stat");
        return 1;
    }
    *uid = st.st_uid;
    *gid = st.st_gid;
    return 0;
}

int create_demon() {
    pid_t mypid = fork();
    if (mypid == -1){ //error
        perror("fork");
        return 1;
    }
    else if (mypid != 0){ //родительский
        return 0;
    }
    int initd = inotify_init1(0);
    if (initd == -1) {
        perror("inotify_init1");
        return 1;
    }
    int watchd = inotify_add_watch(initd, get_current_dir_name(), IN_CREATE);
    if (watchd == -1) {
        perror("inotify_add_watch");
        return 1;
    }
    char buffer[sizeof(struct inotify_event) + NAME_MAX + 1];
    int length;
    while(true) {
        length = read(initd, buffer, sizeof(struct inotify_event) + NAME_MAX + 1);
        if (errno == EINTR) {
            //think about it
        } else if (errno == EINVAL) {
            //buffer too small
        }
        struct inotify_event *event = ( struct inotify_event * ) buffer;

        string template_string;
        string dir = get_current_dir_name();
        string template_filename = dir + "/template.tbl";
        fstream template_file(template_filename.c_str(), fstream::in | std::fstream::out);
        template_file >> template_string >> template_string;

        sort_out_dir(template_string, set_root);
        string name;
        while (template_file >> name) {
            set_root(name);
        }
        template_file.close();
    }
}


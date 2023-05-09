#include <iostream>
#include <fstream>
#include <cstring>
#include <unistd.h>

using namespace std;

int main(int argc, char** argv) {
    char** shares;
    int t = 3;
    int n = 5;
    int debug = 0;

    string input;

    int option;
    while ((option = getopt(argc, argv, "t:n:")) != -1) {
        switch (option) {
            case 't':
                t = atoi(optarg);
                break;
            case 'n':
                n = atoi(optarg);
                break;
            default:
                cerr << "Usage: " << argv[0] << " -t <t> -n <n>" << endl;
                return 1;
        }
    }
    
    std::cin.sync_with_stdio(false);
    if (std::cin.rdbuf()->in_avail() != 0) {
        char line[406]; // Adjust the size as per your requirement
        while (std::cin.getline(line, sizeof(line))) {
            std::cout << line << std::endl;
        }
    } else {
        std::cerr << "No input provided through redirection." << std::endl;
        return 1;
    }


    return 0;
}

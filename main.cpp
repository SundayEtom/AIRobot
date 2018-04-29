#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <random>
#include <regex>

#include <cstdlib>
#include <cstring>
#include "includes/airobot.hpp"

using namespace std;


int main(int argc, char *argv[]){
    if(argc < 2){
        cerr<<"Usage: AIRobot <source_file>"<<endl;
        return 1;
    }

    Robot robot(argv[1]);
    robot.run();

    return 0;
}



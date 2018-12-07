#ifndef parser_hpp
#define parser_hpp

#include <iostream>
#include <string>
#include <vector>
using namespace std;

class Parser
{
public:
    pair<string, vector<int>> read_and_parse(); //function to read and parse input
};

#endif

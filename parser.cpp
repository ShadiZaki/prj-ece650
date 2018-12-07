#include "parser.hpp"

pair<string, vector<int>> Parser::read_and_parse() //function to read and parse input
{
    pair<string, vector<int>> parser_result; //output of parser, a command, and set of parameters
    string command; //input command
    vector<int> parameters; //vector to store parameters
    string input_string;    //input line
    getline(cin, input_string); //reading line
    
    if(!input_string.empty())   //check if an input was read
    {
        command = input_string.substr(0, input_string.find(" ")); //extract command from input line
    }
    else
    {
        command = input_string; //make sure command is an empty string
    }
    
    for(int i = 0; i < input_string.length(); i++)  //remove any non-digit character from input parameters
    {
        if(input_string[i] != '0' && input_string[i] != '1' && input_string[i] != '2' && input_string[i] != '3'
           && input_string[i] != '4' && input_string[i] != '5' && input_string[i] != '6' && input_string[i] != '7' && input_string[i] != '8' && input_string[i] != '9')
        {
            input_string[i] = ' ';
        }
    }
    
    for(int i = 0; i < input_string.length(); i++)  //extract all integers and add them to parameter vector
    {
        if(input_string[i] != ' ')
        {
            int j = i;
            while(input_string[j] != ' ')
            {
                j++;
                break;
            }
            parameters.push_back(stoi(input_string.substr(i,j-i+1)));
            i = j;
        }
    }
    
    parser_result = {command, parameters};
    return parser_result;   //return command and parameters
}
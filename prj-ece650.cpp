#include "minisat/core/SolverTypes.h"
#include "minisat/core/Solver.h"
#include "parser.hpp"
#include "edge.hpp"
#include <pthread.h>
#include <vector>
#include <algorithm>
#include <time.h>

using namespace std;

int vertex_count = 0;   //number of vertices
vector<Edge> edge_list; //vector to store edges
vector<int> cnf_sat_vc_out;
vector<int> approx_vc_1_out;
vector<int> approx_vc_2_out;

int read_write_mutex;
int cnf_sat_vc_mutex;
int approx_vc_1_mutex;
int approx_vc_2_mutex;

void* read_write(void* unused)
{
    read_write_mutex = 1;
    
    string command = "    ";
    bool v_specified = false;   //determines if the number of vertices has been specified
    bool e_specified = false;
    
    while(!e_specified) {
    Parser parser;  //parser instance
    pair<string, vector<int>> parser_result;    //result of parser (command, parameters)
    vector<int> parameters;     //input parameters
        
    parser_result = parser.read_and_parse();    //read and parse input
    command = parser_result.first;  //splitting parser result into command and parameters
    parameters = parser_result.second;
        
    if(command == "")
    {
        exit(0);
    }
        
    if(command == "V" || command == "v")    //check if command is a vertex number specification
    {
        if(parameters.size() == 1)  //check if there is only one parameter (vertex count)
        {
            vertex_count = parameters[0];
            v_specified = true;
        }
    }
    else if(command == "E" || command == "e")   //check if command is an edges specification
    {
        if(v_specified) //check if the number of vertices has been specified
        {
            if(parameters.size() % 2 == 0)  //check if the number of parameters is even (because edges are specified as vertex pairs)
            {
                bool valid_edges = true;    //determines if edge specification is valid
                    
                for(int i = 0; i < parameters.size(); i+=2)
                {
                    int vertex1 = parameters[i];
                    int vertex2 = parameters[i+1];
                    if(vertex1 < 0 || vertex1 >= vertex_count || vertex2 < 0 || vertex2 >= vertex_count || vertex1 == vertex2) //check if the vertices are not within range or equal to each other
                    {
                        valid_edges = false;
                        break;
                    }
                }
                    
                edge_list.clear();  //clear previous edges
                    
                if(valid_edges) //check if edges are valid
                {
                    for(int i = 0; i < parameters.size(); i+=2) //push every edge into the edges vector
                    {
                        int vertex1 = parameters[i];
                        int vertex2 = parameters[i+1];
                        Edge new_edge(vertex1, vertex2);
                        edge_list.push_back(new_edge);
                    }
                    e_specified = true;
                }
                else    //invalid edges
                {
                    cerr<<"Error: invalid edges"<<endl;
                }
            }
        }
        else   //number of vertices was not specified
        {
            cerr<<"Error: number of vertices was not specified"<<endl;
        }
    }
    }
    cnf_sat_vc_mutex = 1;
    approx_vc_1_mutex = 1;
    approx_vc_2_mutex = 1;
    
    read_write_mutex = 0;
    
    while(cnf_sat_vc_mutex == 1) { }
    while(approx_vc_1_mutex == 1) { }
    while(approx_vc_2_mutex == 1) { }
    
    cout<<"CNF-SAT-VC: ";
    for(int i = 0; i < cnf_sat_vc_out.size(); i++)    //output vertex cover
    {
        cout<<cnf_sat_vc_out.at(i);
        if(i != cnf_sat_vc_out.size()-1)
        {
            cout<<",";
        }
    }
    cout<<endl;
    cnf_sat_vc_out.clear();
    
    cout<<"APPROX-VC-1: ";
    for(int i = 0; i < approx_vc_1_out.size(); i++)
    {
        cout<<approx_vc_1_out.at(i);
        if(i != approx_vc_1_out.size()-1)
        {
            cout<<",";
        }
    }
    cout<<endl;
    approx_vc_1_out.clear();
    
    cout<<"APPROX-VC-2: ";
    for(int i = 0; i < approx_vc_2_out.size(); i++)
    {
        cout<<approx_vc_2_out.at(i);
        if(i != approx_vc_2_out.size()-1)
        {
            cout<<",";
        }
    }
    cout<<endl;
    approx_vc_2_out.clear();
}

void* cnf_sat_vc(void* unused) //function to find the vertex cover using cnf sat solver
{
    clockid_t cid;
    struct timespec ts;
    
    pthread_getcpuclockid(pthread_self(), &cid);
    clock_gettime(cid, &ts);
    
    cout<<"CNF-SAT-VC thread CPU Time: ";
    printf("%4ld.%03ld\n", ts.tv_sec, ts.tv_nsec / 1000000);
    
    int k = 1;  // start with a vertex cover of size 1
    bool sat = false;   // assume satisfiability is false initially
    
    while (!sat && k < vertex_count)
    {
        using Minisat::mkLit;
        using Minisat::lbool;
        
        Minisat::Solver solver;
        
        Minisat::Var variables[k][vertex_count];    //create variables
        
        for(int i = 0; i < k; i++)  //initialize variables
        {
            for (int j = 0; j < vertex_count; j++)
            {
                variables[i][j] = solver.newVar();
            }
        }
        
        for(int i = 0; i < k; i++)  //at least one vertex is the ith vertex in the vertex cover
        {
            Minisat::vec<Minisat::Lit> literals;
            for (int j = 0; j < vertex_count; j++)
            {
                literals.push(Minisat::mkLit(variables[i][j]));
            }
            solver.addClause(literals);
        }
        
        for(int m = 0; m < vertex_count; m++)   //no one vertex can appear twice in the vertex cover
        {
            for(int p = 0; p < k-1; p++)
            {
                for(int q = p+1; q < k; q++)
                {
                    solver.addClause(~mkLit(variables[p][m]), ~mkLit(variables[q][m]));
                }
            }
        }
        
        for(int m = 0; m < k; m++)  //no more than one vertex appears in the mth position of the vertex cover
        {
            for(int p = 0; p < vertex_count-1; p++)
            {
                for(int q = p+1; q < vertex_count; q++)
                {
                    solver.addClause(~mkLit(variables[m][p]), ~mkLit(variables[m][q]));
                }
            }
        }
        
        for(int edge_index = 0; edge_index < edge_list.size(); edge_index++)    //every edge is incident to at least one vertex in the vertex cover
        {
            int vertex1 = edge_list.at(edge_index).get_vertex1();
            int vertex2 = edge_list.at(edge_index).get_vertex2();
            
            Minisat::vec<Minisat::Lit> literals;
            
            for(int i = 0; i < k; i++)
            {
                literals.push(Minisat::mkLit(variables[i][vertex1]));
            }
            
            for(int i = 0; i < k; i++)
            {
                literals.push(Minisat::mkLit(variables[i][vertex2]));
            }
            solver.addClause(literals);
        }
        
        sat = solver.solve();   //find a possible solution if there is one
        
        if(sat)
        {
            bool sat_result[k][vertex_count];
            for(int i = 0; i < k; i++)
            {
                for(int j = 0; j < vertex_count; j++)
                {
                    sat_result[i][j] = (solver.modelValue(variables[i][j]) == Minisat::l_True);
                }
            }
            
            for(int i = 0; i < k; i++)  //create vertex cover
            {
                for(int j = 0; j < vertex_count; j++)
                {
                    if(sat_result[i][j])
                    {
                        cnf_sat_vc_out.push_back(j);
                        break;
                    }
                }
            }
            
            sort(cnf_sat_vc_out.begin(), cnf_sat_vc_out.end()); //sort vertex cover
            break;
        }
        k++;
    }
    
    cnf_sat_vc_mutex = 0;
}

void* approx_vc_1(void* unused)
{
    clockid_t cid;
    struct timespec ts;
    
    pthread_getcpuclockid(pthread_self(), &cid);
    clock_gettime(cid, &ts);
    
    cout<<"APPROX-VC-1 thread CPU Time: ";
    printf("%4ld.%03ld\n", ts.tv_sec, ts.tv_nsec / 1000000);
    
    vector<Edge> temp_edge_list = edge_list;
    while(temp_edge_list.size() != 0)
    {
        int *vertex_degrees = new int[vertex_count];
    
        for(int i = 0; i < vertex_count; i++)
        {
            vertex_degrees[i] = 0;
        }
    
        for(int i = 0; i < temp_edge_list.size(); i++)
        {
            vertex_degrees[temp_edge_list.at(i).get_vertex1()] = vertex_degrees[temp_edge_list.at(i).get_vertex1()] + 1;
            vertex_degrees[temp_edge_list.at(i).get_vertex2()] = vertex_degrees[temp_edge_list.at(i).get_vertex2()] + 1;
        }
    
        int max_index = 0;
        for(int i = 0; i < vertex_count; i++)
        {
            if(vertex_degrees[i] > vertex_degrees[max_index])
            {
                max_index = i;
            }
        }
    
        approx_vc_1_out.push_back(max_index);
    
        int edge_index = 0;
        while(edge_index < temp_edge_list.size())
        {
            if(temp_edge_list.at(edge_index).get_vertex1() == max_index || temp_edge_list.at(edge_index).get_vertex2() == max_index)
            {
                temp_edge_list.erase(temp_edge_list.begin()+edge_index);
                edge_index--;
            }
            edge_index++;
        }
    }
    
    sort(approx_vc_1_out.begin(), approx_vc_1_out.end());
    
    approx_vc_1_mutex = 0;
}

void* approx_vc_2(void* unused)
{
    clockid_t cid;
    struct timespec ts;
    
    pthread_getcpuclockid(pthread_self(), &cid);
    clock_gettime(cid, &ts);
    
    cout<<"APPROX-VC-2 thread CPU Time: ";
    printf("%4ld.%03ld\n", ts.tv_sec, ts.tv_nsec / 1000000);
    
    vector<Edge> temp_edge_list = edge_list;
    while (temp_edge_list.size() != 0)
    {
        int edge_index_to_remove = 0;
        int vertex1 = temp_edge_list.at(edge_index_to_remove).get_vertex1();
        int vertex2 = temp_edge_list.at(edge_index_to_remove).get_vertex2();
        approx_vc_2_out.push_back(vertex1);
        approx_vc_2_out.push_back(vertex2);
        
        int edge_index = 0;
        while(edge_index < temp_edge_list.size())
        {
            if(temp_edge_list.at(edge_index).get_vertex1() == vertex1 || temp_edge_list.at(edge_index).get_vertex1() == vertex2
               || temp_edge_list.at(edge_index).get_vertex2() == vertex1 || temp_edge_list.at(edge_index).get_vertex2() == vertex2)
            {
                temp_edge_list.erase(temp_edge_list.begin()+edge_index);
                edge_index--;
            }
            edge_index++;
        }
    }
    
    sort(approx_vc_2_out.begin(), approx_vc_2_out.end());
    
    approx_vc_2_mutex = 0;
}

int main(int argc, const char * argv[])
{
    while (true)
    {
        
        pthread_t io_thread;
        pthread_t cnf_sat_thread;
        pthread_t approx_vc_1_thread;
        pthread_t approx_vc_2_thread;
        
        read_write_mutex = 1;
    
        pthread_create (&io_thread, NULL, &read_write, NULL);
        
        while(read_write_mutex == 1) { }
        
        pthread_create (&cnf_sat_thread, NULL, &cnf_sat_vc, NULL);
        pthread_create (&approx_vc_1_thread, NULL, &approx_vc_1, NULL);
        pthread_create (&approx_vc_2_thread, NULL, &approx_vc_2, NULL);
        
//        clockid_t cid;
//        struct timespec ts;
//        
//        pthread_getcpuclockid(cnf_sat_thread, &cid);
//        clock_gettime(cid, &ts);
//        
//        cout<<"CNF-SAT-VC thread CPU Time: ";
//        printf("%4ld.%03ld\n", ts.tv_sec, ts.tv_nsec / 1000000);
//        
//        pthread_getcpuclockid(approx_vc_1_thread, &cid);
//        clock_gettime(cid, &ts);
//        
//        cout<<"APPROX-VC-1 thread CPU Time: ";
//        printf("%4ld.%03ld\n", ts.tv_sec, ts.tv_nsec / 1000000);
//
//        pthread_getcpuclockid(approx_vc_2_thread, &cid);
//        clock_gettime(cid, &ts);
//        
//        cout<<"APPROX-VC-2 thread CPU Time: ";
//        printf("%4ld.%03ld\n", ts.tv_sec, ts.tv_nsec / 1000000);

    }
    return 0;
}

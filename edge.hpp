#ifndef edge_hpp
#define edge_hpp

class Edge
{
private:
    int vertex1;    //first vertex
    int vertex2;    //second vertex
public:
    Edge(int new_vertex1, int new_vertex2); //edge constructor
    int get_vertex1();  //first vertex accessor
    int get_vertex2();  //second vertex accessor
};

#endif

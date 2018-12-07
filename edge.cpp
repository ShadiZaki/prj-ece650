#include "edge.hpp"

Edge::Edge(int new_vertex1, int new_vertex2) //edge constructor
{
    this->vertex1 = new_vertex1;
    this->vertex2 = new_vertex2;
}

int Edge::get_vertex1()   //first vertex accessor
{
    return vertex1;
}

int Edge::get_vertex2()   //second vertex accessor
{
    return vertex2;
}
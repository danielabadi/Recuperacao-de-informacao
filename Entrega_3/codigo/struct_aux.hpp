#ifndef STRUCT_AUX_H
#define STRUCT_AUX_H
#include <list>
#include <tuple>
#include <iostream>
struct struct_aux{
    std::list<std::pair<std::pair<int, int>, std::list<int>>> lista;

    void add(int, int);
    int return_size();
    int appear();
    int size_struct();
};

#endif

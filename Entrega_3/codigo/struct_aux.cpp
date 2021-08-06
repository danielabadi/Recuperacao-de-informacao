#include "struct_aux.hpp"

void struct_aux::add(int num_doc, int pos){
    std::list<std::pair<std::pair<int, int>, std::list<int>>>::iterator it;
    for(it=this->lista.begin(); it!=this->lista.end() ; ++it){
        if(it->first.first == num_doc){
            break;
        }
    }
    if(it!=this->lista.end()){
        it->first.second++;
        it->second.push_back(pos);
    } else {
        std::list<int> child_list;
        child_list.push_back(pos);
        this->lista.push_back({{num_doc, 1}, child_list});
    }
}
int struct_aux::return_size(){
    return this->lista.size();
}
int struct_aux::appear(){
    int amount = 0;
    std::list<std::pair<std::pair<int, int>, std::list<int>>>::iterator it;
    for(it=this->lista.begin(); it!=this->lista.end() ; ++it){
        amount += it->first.second;
    }
    return amount/return_size();
}
int struct_aux::size_struct(){
    int size = 0;
    std::list<std::pair<std::pair<int, int>, std::list<int>>>::iterator it;
    for(it=this->lista.begin(); it!=this->lista.end() ; ++it){
        size += 2; // numero do documento e aparições
        size += it->second.size();
    }
    return size;
}

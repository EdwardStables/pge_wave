#include "wave_store.h"

Var::Var(int width, std::string id, std::string name) :
    id(id),
    name(name),
    width(width)
{}

void Var::add_change(int time, int next_value){
    value.push_back(std::make_tuple(time, next_value));
}

void VarStore::add_key(int width, std::string symbol, std::string name){
    vars[symbol] = new Var(width, symbol, name);
}

void VarStore::add_change(std::string key, int time, int value){
    vars[key]->add_change(time, value);
}    

void VarStore::parse_var(std::vector<std::string> var){
    std::stringstream ss;
    ss << var[1];
    int width;
    ss >> width;
    std::string symbol = var[2];
    std::string name = var[3];
    std::string type = var[0];
    add_key(width, symbol, name);
}

std::ostream& operator<< (std::ostream &out, Var const& data){
    out << data.name << " " << data.width << " " << data.id;
    return out;
}
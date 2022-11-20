#include "wave_store.h"
#include <iostream>

Var::Var(int width, std::string id, std::string name) :
    id(id),
    name(name),
    width(width)
{}

void Var::add_change(int time, int next_value){
    auto tup = std::make_tuple(time, next_value);
    value.push_back(tup);
}

int Var::val_at_time(int time) const{
    int val = 0;
    for (auto &v : value){
        if (std::get<0>(v) > time){
            return val;
        } else {
            val = std::get<1>(v);
        }
    }
    return val;
}

int Var::get_next_time(int time) const{
    for (auto &v : value){
        if (std::get<0>(v) > time) return std::get<0>(v);
    }

    return -1;
}

void VarStore::add_key(int width, std::string symbol, std::string name){
    Var v(width, symbol, name);
    vars.push_back(v);
    var_map[symbol] = vars.size()-1;
}

void VarStore::add_change(std::string key, int time, int value){
    if (var_map.find(key) != var_map.end())
        vars[var_map[key]].add_change(time, value);
    else
        std::cout << "Warning, tried to add symbol '" << key << "', which doesn't exist." << std::endl;
}    

void VarStore::parse_var(std::vector<std::string> var){
    //TODO this is parsing code and should be in the vcd file
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

std::vector<Var> VarStore::get_vars(){
    return vars;
}

Var& VarStore::get_var_by_name(std::string name){
    for(auto &v : vars){
        if (v.name == name) return v;
    }
}
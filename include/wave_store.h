#ifndef __WAVE_STORE_H__
#define __WAVE_STORE_H__

#include <vector>
#include <string>
#include <tuple>
#include <unordered_map>
#include <sstream>

struct Var{
    std::string id;
    std::string name;
    int width;
    int initial;
    std::vector<std::tuple<int,int>> value;
    
    //TODO: apply a mask here to indicate times bits toggle
    std::vector<std::vector<int>> x_mask;

    Var(int width, std::string id, std::string name) :
        id(id),
        name(name),
        width(width)
    {}

    void add_change(int time, int next_value){
        value.push_back(std::make_tuple(time, next_value));
    }
};

std::ostream& operator<< (std::ostream &out, Var const& data);

struct VarStore{
    std::unordered_map<std::string,Var*> vars;

    void add_key(int width, std::string symbol, std::string name){
        vars[symbol] = new Var(width, symbol, name);
    }

    void add_change(std::string key, int time, int value){
        vars[key]->add_change(time, value);
    }    

    void parse_var(std::vector<std::string> var){
        std::stringstream ss;
        ss << var[1];
        int width;
        ss >> width;
        std::string symbol = var[2];
        std::string name = var[3];
        std::string type = var[0];
        add_key(width, symbol, name);
    }
};
#endif
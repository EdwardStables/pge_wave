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

    Var(int width, std::string id, std::string name);
    void add_change(int time, int next_value);
    int val_at_time(int time) const;
    int get_next_time(int time) const;
};

std::ostream& operator<< (std::ostream &out, Var const& data);

struct VarStore{
    std::unordered_map<std::string,int> var_map;
    std::vector<Var> vars;
    std::vector<Var> get_vars();
    void add_key(int width, std::string symbol, std::string name);
    void add_change(std::string key, int time, int value);
    void parse_var(std::vector<std::string> var);
};
#endif
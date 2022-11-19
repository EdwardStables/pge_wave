#include "vcd_parser.h"
#include <fstream>
#include <iostream>
#include <sstream>
#include <vector>
#include <tuple>
#include <unordered_map>

using string = std::string;

enum TOKEN {
    TOKEN_INVALID,    

    //Sections
    SEC_DATE, SEC_VERSION, SEC_COMMENT, SEC_SCOPE,
    SEC_TIMESCALE, SEC_UPSCOPE, SEC_ENDDEF, SEC_VAR,
    SEC_DUMPALL, SEC_DUMPOFF, SEC_DUMPON, SEC_DUMPVARS,
    END_SEC,

   TIMESTAMP,

   VALUE
};

bool is_section_header_token(TOKEN token){
    return token == SEC_DATE || token == SEC_VERSION || token == SEC_COMMENT || 
           token == SEC_SCOPE || token == SEC_TIMESCALE || token == SEC_UPSCOPE || 
           token == SEC_ENDDEF || token == SEC_VAR || token == SEC_DUMPALL || 
           token == SEC_DUMPOFF || token == SEC_DUMPON || token == SEC_DUMPVARS;

}

bool is_value(string str) {
    //single bit value
    if (str.size() == 2 &&
        (str[0] == '0' || str[0] == '1' || str[0] == 'x' || str[0] == 'z') &&
        (str[1] >= 33 && str[1] < 126)) {
        return true;
    }

    //half-arsed attempt at multi-bit value
    if ((str[0] == 'b' || str[0] == 'r')) return true;

    return false;
}

TOKEN tokenize(string str){
    if (str.size() == 0) return TOKEN_INVALID;
    if (str == "$end") return END_SEC;
    if (str == "$date") return SEC_DATE;
    if (str == "$version") return SEC_VERSION;
    if (str == "$comment") return SEC_COMMENT;
    if (str == "$scope") return SEC_SCOPE;
    if (str == "$timescale") return SEC_TIMESCALE;
    if (str == "$upscope") return SEC_UPSCOPE;
    if (str == "$enddefinitions") return SEC_ENDDEF;
    if (str == "$var") return SEC_VAR;
    if (str == "$dumpall") return SEC_DUMPALL;
    if (str == "$dumpoff") return SEC_DUMPOFF;
    if (str == "$dumpon") return SEC_DUMPON;
    if (str == "$dumpvars") return SEC_DUMPVARS;
    if (str[0] == '#') return TIMESTAMP;
    if (is_value(str)) return VALUE;

    return TOKEN_INVALID;
}

enum TIMEBASE {
    TB_S, TB_MS, TB_US, TB_NS, TB_PS, TB_FS
};

struct Timescale {
    int num; //VCD specifies 1, 10, or 100.
    TIMEBASE tb; 

    Timescale() : Timescale("1ns") {}
    Timescale(string ts) :
        tb(get_timebase(ts)),
        num(get_num(ts))
    {}
    
    static TIMEBASE get_timebase(string ts) {
        int s_ind = ts.find('s');

        switch(ts[s_ind]-1){
            case 'm': return TB_MS;
            case 'u': return TB_US;
            case 'n': return TB_NS;
            case 'p': return TB_PS;
            case 'f': return TB_FS;
            default: return TB_S;
        }
    }

    static int get_num(string ts) {
        //this is so hacky
        int ind1 = ts.find_first_of("0");
        int ind2 = ts.find_last_of("0");
        if (ind1 == -1 && ind2 == -1) return 1;
        if (ind1 == ind2) return 10;
        if (ind1 == ind2-1) return 100;
        return 1;
    }

    string get_base_rep() const {
        switch(tb){
            case TB_MS: return "ms";
            case TB_US: return "us";
            case TB_NS: return "ns";
            case TB_PS: return "ps";
            case TB_FS: return "fs";
            default: return "s";
        }
    }

};

std::ostream& operator<< (std::ostream &out, Timescale const& data){
    out << data.num << data.get_base_rep();
    return out;
}

struct VCD_Meta {
    string date;
    string version;
    string comment;
    Timescale timescale;
};

struct Var{
    string id;
    string name;
    int width;
    int initial;
    std::vector<std::tuple<int,int>> value;

    Var(int width, string id, string name) :
        id(id),
        name(name),
        width(width)
    {}

    void add_change(int time, int next_value){
        value.push_back(std::make_tuple(time, next_value));
    }
};

std::ostream& operator<< (std::ostream &out, Var const& data){
    out << data.name << " " << data.width << " " << data.id;
    return out;
}

struct VarStore{
    std::unordered_map<string,Var*> vars;

    void add_key(int width, string symbol, string name){
        vars[symbol] = new Var(width, symbol, name);
    }

    void add_change(string key, int time, int value){
        vars[key]->add_change(time, value);
    }    

    void parse_var(string var){
        std::stringstream ss;
        ss << var;
        int width;
        string symbol, name, type;
        ss >> type >> width >> symbol >> name;
        add_key(width, symbol, name);
    }
};

void dump_parse(int current_time, VarStore &var_store, string data, bool initial=false){
    std::cout << "todo, dumpparse" << std::endl;
}

void section_parse(int current_time, VCD_Meta &metadata, VarStore &var_store, TOKEN section_token, string section_data){
    string sec_str;    

    switch (section_token) {
        case SEC_DATE:      metadata.date = section_data; break;
        case SEC_VERSION:   metadata.version = section_data; break;
        case SEC_COMMENT:   metadata.comment = section_data; break;
        case SEC_TIMESCALE: metadata.timescale = Timescale(section_data); break;
        case SEC_SCOPE:     sec_str = "section scope"; break;
        case SEC_UPSCOPE:   sec_str = "section upscope"; break;
        case SEC_ENDDEF:    sec_str = "section enddef"; break;
        case SEC_VAR:       var_store.parse_var(section_data); break;
        case SEC_DUMPALL:   dump_parse(current_time, var_store, section_data, true); break;
        case SEC_DUMPOFF:   sec_str = "section dumpoff"; break;
        case SEC_DUMPON:    sec_str = "section dumpon"; break;
        case SEC_DUMPVARS:  dump_parse(current_time, var_store, section_data); break;
        case VALUE:         
            std::stringstream ss;
            int val;
            if (section_data.size() == 2){
                ss << section_data[0];
                ss >> val;
            } else {
                ss << section_data.substr(0, section_data.size()-1);
                ss >> val;
            }
            var_store.add_change(section_data.substr(section_data.size()-1), current_time, val);
            break;
    }
}

bool parse(){
    VCD_Meta metadata;
    VarStore var_store;
    int current_time;

    std::ifstream inputFileStream("../../example.vcd");    
    string line;

    bool prelude = true;
    bool in_section = false;
    TOKEN section_token = TOKEN_INVALID;

    string contained_line;//the full contents of a section. to be passed by a section parser. Cleared by $end

    while (std::getline(inputFileStream, line)){
        std::stringstream ss;
        ss << line;

        if (!in_section){
            string sampled_str;
            ss >> sampled_str;
            TOKEN found_token = tokenize(sampled_str);

            if (found_token == TOKEN_INVALID){
                std::cout << "invalid token found " + sampled_str + "." << std::endl;
                return false;
            }

            if (prelude){
                if (!is_section_header_token(found_token)){
                    std::cout << "unexpected token found " + sampled_str + ". Expected section header." << std::endl;
                    return false;
                }
                in_section = true;
                section_token = found_token;
            } else {
                if (is_section_header_token(found_token)){
                    in_section = true;
                    section_token = found_token;
                }
                else if (found_token == TIMESTAMP){
                    string ts = sampled_str.substr(1);
                    std::stringstream tss;
                    tss << ts;
                    tss >> current_time;
                } else if (found_token == VALUE){
                    //note that a value may need the whole line
                    section_parse(current_time, metadata, var_store, found_token, line);
                }
            }
        }

        //assume at point that we're in a section, accumulate data for parsing

        string next_item;
        bool found_endl = false;
        //iterate over the rest of the line
        while (!ss.eof()){
            ss >> next_item;
            if (tokenize(next_item) == END_SEC){
                found_endl = true;
            } else {
                contained_line += " ";
                contained_line += next_item;
            }
        }
        if (found_endl) {
            //do the section specific parsing
            section_parse(current_time, metadata, var_store, section_token, contained_line);

            if (section_token == SEC_ENDDEF){
                prelude = false;
            }

            contained_line = "";
            section_token = TOKEN_INVALID; //later, jump up sections for nested sections
            in_section = false;

        }
    }

    std::cout << metadata.date << std::endl;
    std::cout << metadata.comment << std::endl;
    std::cout << metadata.version << std::endl;
    std::cout << metadata.timescale << std::endl;

    for(auto &v : var_store.vars){
        std::cout << *v.second << std::endl;
        std::cout << "    ";
        for (auto &val : v.second->value){
            std::cout << "(" << std::get<0>(val) << ", " << std::get<1>(val) << ") ";
        }
        std::cout << std::endl;
    }

    return true;
}
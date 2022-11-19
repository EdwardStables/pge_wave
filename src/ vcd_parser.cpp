#include "vcd_parser.h"
#include <fstream>
#include <iostream>
#include <sstream>
#include <vector>
#include <unordered_map>

using string = std::string;

enum TOKEN {
    TOKEN_INVALID,    

    //Sections
    SEC_DATE, SEC_VERSION, SEC_COMMENT, SEC_SCOPE,
    SEC_TIMESCALE, SEC_UPSCOPE, SEC_ENDDEF, SEC_VAR,
    SEC_DUMPALL, SEC_DUMPOFF, SEC_DUMPON, SEC_DUMPVARS,
    END_SEC,

    TIMESTAMP
};

bool is_section_header_token(TOKEN token){
    return token == SEC_DATE || token == SEC_VERSION || token == SEC_COMMENT || 
           token == SEC_SCOPE || token == SEC_TIMESCALE || token == SEC_UPSCOPE || 
           token == SEC_ENDDEF || token == SEC_VAR || token == SEC_DUMPALL || 
           token == SEC_DUMPOFF || token == SEC_DUMPON || token == SEC_DUMPVARS;

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
};

struct VCD_Meta {
    string date;
    string version;
    string comment;
    Timescale timescale;
};

struct var {
    int width;
    string symbol;
    string name;
};

struct module{
    string name;
    std::vector<module*> modules;
    module* parent;

    std::vector<var> vars;   
};

enum states {
    s_top,
    s_scope,
    s_value_dump
};

void section_parse(TOKEN section_token, string section_data){
    string sec_str;    

    switch (section_token) {
        case SEC_DATE:      sec_str = "section date"; break;
        case SEC_VERSION:   sec_str = "section version"; break;
        case SEC_COMMENT:   sec_str = "section comment"; break;
        case SEC_SCOPE:     sec_str = "section scope"; break;
        case SEC_TIMESCALE: sec_str = "section timescale"; break;
        case SEC_UPSCOPE:   sec_str = "section upscope"; break;
        case SEC_ENDDEF:    sec_str = "section enddef"; break;
        case SEC_VAR:       sec_str = "section var"; break;
        case SEC_DUMPALL:   sec_str = "section dumpall"; break;
        case SEC_DUMPOFF:   sec_str = "section dumpoff"; break;
        case SEC_DUMPON:    sec_str = "section dumpon"; break;
        case SEC_DUMPVARS:  sec_str = "section dumpvars"; break;
    }

    std::cout << sec_str + " " + section_data << std::endl;
}


bool parse(){
    std::ifstream inputFileStream("../../example.vcd");    
    string line;

    bool prelude = true;
    bool in_section = false;
    TOKEN section_token = TOKEN_INVALID;

    string contained_line;//the full contents of a section. to be passed by a section parser. Cleared by $end

    int lc = 0;
    while (std::getline(inputFileStream, line)){
        std::stringstream ss;
        ss << line;

        if (!in_section){
            string sec_header;
            ss >> sec_header;
            TOKEN section = tokenize(sec_header);

            if (section == TOKEN_INVALID){
                std::cout << "invalid token found " + sec_header + ". Expected section header." << std::endl;
                return false;
            }
            if (!is_section_header_token(section)){
                std::cout << "unexpected token found " + sec_header + ". Expected section header." << std::endl;
                return false;
            }

            in_section = true;
            section_token = section;
        }

        //assume at point that we're in a section

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
            section_parse(section_token, contained_line);

            if (section_token == SEC_ENDDEF){
                std::cout << "hit enddefinnitions, stopping early" << std::endl;
                break;
            }
            contained_line = "";
            section_token = TOKEN_INVALID; //later, jump up sections for nested sections
            in_section = false;
        }
    }

    return true;
}

void _parse_vcd(){
    module top;
    module *current = &top;
    std::ifstream inputFileStream("../../example.vcd");    
    string line;
    states state;
    
    while (std::getline(inputFileStream, line)){
        std::stringstream ss;
        string start; 
        ss << line;
        ss >> start;
        
        if (start == "$scope"){
            if (current != &top){
                module *new_mod = new module();
                current->modules.push_back(new_mod);
                current = new_mod;
            }
        }

        if (start == "$upscope"){
            if (current != &top){
                current = current->parent;
            }
        }

        if (start == "$var"){
            var v;
            int width;
            string sym, name, wire;
            ss >> wire >> v.width >> v.symbol >> v.name;
            current->vars.push_back(v);
        }

        if (start.size() > 0 && start.at(0) == '#'){
            ss.clear();
            ss << start.substr(1);
            int ts;
            ss >> ts;
            std::cout << ts << std::endl;
        }
    }
}
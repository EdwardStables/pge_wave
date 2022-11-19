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
};

struct VCD_Meta {
    string date;
    string version;
    string comment;
    Timescale timescale;
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
                    std::cout << "timestamp " << sampled_str << std::endl;
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
            section_parse(section_token, contained_line);

            if (section_token == SEC_ENDDEF){
                prelude = false;
            }

            contained_line = "";
            section_token = TOKEN_INVALID; //later, jump up sections for nested sections
            in_section = false;

        }
    }

    return true;
}
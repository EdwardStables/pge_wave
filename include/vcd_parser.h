#ifndef __VCD_PARSER_H__
#define __VCD_PARSER_H__

#include "wave_store.h"

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

enum TIMEBASE {
    TB_S, TB_MS, TB_US, TB_NS, TB_PS, TB_FS
};


struct Timescale {
    int num; //VCD specifies 1, 10, or 100.
    TIMEBASE tb; 
    Timescale();
    Timescale(std::string ts);
    static TIMEBASE get_timebase(std::string ts);
    static int get_num(std::string ts);
    std::string get_base_rep() const;
};

struct VCD_Meta {
    std::string date;
    std::string version;
    std::string comment;
    Timescale timescale;
};

std::ostream& operator<< (std::ostream &out, Timescale const& data);
bool is_value(std::string str);
bool is_section_header_token(TOKEN token);
TOKEN tokenize(std::string str);
void parse_multi_bit_val(int &val, std::string data);
void value_parse(int current_time, VarStore &var_store, std::vector<std::string> data);
void dump_parse(int current_time, VarStore &var_store, std::vector<std::string> data, bool initial=false);
void section_parse(int current_time, VCD_Meta &metadata, VarStore &var_store, TOKEN section_token, std::vector<std::string> section_vec);
bool parse(VarStore &var_store);


#endif
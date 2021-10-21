
#include <string>
#include <vector>
#include <iostream>
#include <filesystem>
#include <conio.h>
#include <fstream>
#include <sstream>
#include <functional>
#include <map>
#include <chrono>
#include <ctime>

// open note with WinAPI (-$ note run: <name>) (look _opennote( ) function)
#include <windows.h>    // exactly in this order
#include <shellapi.h>   // requires <windows.h> before


/* ---------------------- GLOBALS ---------------------- */

// get current path for directory navigate simulation
std::string current_path = std::filesystem::current_path( ).string( );        
std::string copy_path;    // keep copy when try to move through    
char bs = 92;              // backslash for add after path

// keep path to directory with notes
std::string mynotesfilepath { };        // path to .txt file with info, where all notes are
std::string notes_dir { };                // path where all notes are

std::vector<std::string> allcommands =  {
                                            "\texit ......................... exit app           ",
                                            "\tmoveup ....................... move one dir up    ",
                                            "\tmove <relative_path> ......... move to dir        ",
                                            "\tshow <variant> ............... show:              ",
                                            "\t     fil                         files only       ",
                                            "\t     oth                         other objs       ",
                                            "\t     all                         all content      ",
                                            "\t     com                         app commands     ",
                                            "\tifex <variant> <name> ........ if exists:         ",
                                            "\t     curr                        rel. path        ",
                                            "\t     path                        abs. path        ",
                                            "\tmkdir <name> (in) ............ make dir (+ in)    ",
                                            "\tmkfile <name> ------ x ------- make file          ",
                                            "\tdel <name> ................... del dir with all   ",
                                            "\tsndir <absolute_path> ........ mynotes_dir path   ",
                                            "\tnote <varinat> ............... work with notes    ",
                                            "\t     new: #<name>                new note         ",
                                            "\t     del: #<name>                delete note      ",
                                            "\t     add: #<name> <note>         add to note      ",
                                            "\t     run: #<name>                open with win    ",
                                            "\t     list                        show all tags    ",                                                
                                        };

enum class local_ERROR
{
    EXIT,               //  0
    SUCCESS,            //  1
    
    ROOTPATH,           //  2
    EXCESS_ARG,         //  3
    EMPTY_ARG,          //  4
    INVALID_ARG,        //  5    
    DOESNOT_EXIST,      //  6
    CANT_SHOW,          //  7
    INVALID_PATH,       //  8
    NOTE_EXISTS,        //  9
    CANNOT_OPEN,        // 10
};


/* ---------------------- CLASSES ---------------------- */

// recieve user's command, split into single args and send them
class ENTERED    
{
    private:
        
        std::vector<std::string> args;    
        unsigned char args_left = 0;    
        unsigned char arg_index = 0;
        std::string curr_arg = "";

        // remove excess 'spaces'
        void remove_spaces(std::string& command)
        {
            // from begin - from end - doubled
            for(auto el = command.begin( ); el != command.end( ); ) {
                if(*el == ' ') command.erase(el, el + 1);        
                else break;    
            }    
            for(auto el = command.rbegin( ); el != command.rend( ); ++el) {
                if(*el == ' ') command.pop_back( );
                else break;        
            }
            for(auto el = command.begin( ); el != command.end( ) - 1; )    {
                if(*el == ' ' && *(el + 1) == ' ') command.erase(el, el + 1);
                else ++el;
            }                
        }
    
        // count 'spaces' in fact so we now number of args
        unsigned char count_args(std::string& command)
        {
            unsigned char sps_number = 0;
            for(const auto& el : command)
            if (el == ' ') 
                ++sps_number;
                
            return sps_number;            
        }
        
        // separate command into single args
        void separate_command(unsigned char args_number, std::string& command)
        {
            args.resize(args_number);
            args_left = args_number ;
            arg_index = 0;
            curr_arg = "";
            
            std::istringstream iss(command);
            for(char i = 0; i < args_number; ++i)
                iss >> args[i];
        }
                    
    public:
        
        // only 1 class object is enough - this method rewrites all fileds
        void set_args(std::string& command)
        {
            remove_spaces(command);
            unsigned char args_number = count_args(command) + 1;
            separate_command(args_number, command);        
        }
        
        // get current arg
        std::string get_arg( )
        {

            if(args_left)
            {
                curr_arg = args[arg_index];
                ++arg_index;
                --args_left;
                return curr_arg;
            }
            
            else return "";
        }
        
        // get args left
        unsigned char get_left( ) 
        {
            return args_left;
        }
        
};

// keep information about every possible arg meaning
struct ARG         
{
    
    std::string map_id;    // identify std::map<std::string, ARG*> 
    
    enum
    {
        NEXT,
        FUNCT,
        
    } what;    //  not necessary

    std::map<std::string, ARG*>* next;
    std::function<local_ERROR(ENTERED&)> funct;    
    
    ARG( ) : what {ARG::NEXT}, next {nullptr}, funct {nullptr} {     }
    
    ARG(const ARG& obj) : what {obj.what} 
    {
        if(what == ARG::NEXT) next  = obj.next;
        else                   funct = obj.funct; 
    }
    
    ARG& operator= (const ARG& obj)
    {
        what = obj.what; 
        
        if(what == ARG::NEXT) next  = obj.next;
        else                   funct = obj.funct;
        
        return *this;             
    }
    
   ~ARG( ) {  }                            
};



/* ---------------- TERMINAL FUNCTIONS ----------------- */

// get user's commands
void terminal(const std::string& current_path, ENTERED& obj)      
{
    
    std::string command;
    unsigned char max_length = 130;
    char command_length = 0;
    char cur_char = 0;
    
    while(1)
    {
        
        // header
        system("cls");
        std::cout << std::endl;
        std::cout << "\tmynotes_dir  =  "  << notes_dir    << std::endl;
        std::cout << "\tcurrent path =  "  << current_path << std::endl;
        std::cout << "\tlimit: " << max_length - command_length - 2 << std::endl;
        std::cout << "\t~$: " << command;
        
        // checking command length
        if(command_length == max_length - 1)
        {
            command.erase( );
            command_length = 0;
            std::cout << "\n\tlocal_ERROR: unacceptable command length\n" << std::endl;
            _getch( );
            continue;
        }
        
        cur_char = _getche( );
        
        // skip arrows
        if(( cur_char == 72 ||    
             cur_char == 75 ||    
             cur_char == 77 ||
             cur_char == 80 
        ) && command[command_length - 1] == -32 ) 
        {
            command.pop_back( );
            --command_length;
            continue;
        }    
        
        // 'backspace' handle
        if(cur_char == 8)
        {
            if(command_length)
            {
                --command_length;
                std::cout << " " << cur_char;
                command.pop_back( );
                continue;    
            }
            
            else continue;
        }
        
        ++command_length;
                    
        // 'enter' handle
        if(cur_char == 13)
        {
                
            if(command_length == 1)    // handle empty command
            {
                --command_length;    
                continue;
            }
            else
            {
                command_length = 0;
                std::cout << std::endl << std::endl;
                break;                    
            }    
        }
        
        command += cur_char;

    }
    
    obj.set_args(command);
        
}

// execute user's commands
local_ERROR execution(ENTERED& obj, std::map<std::string, ARG*>* map)    
{
        
    // iterator for check map elements
    auto iter = map -> begin( );        
    auto iend = map -> end( );            
    
    std::string str = obj.get_arg( );    // get first arg
    ARG arg;                             // struct for next struct

    while(str.length( ))    // handle arguments 
    {
        
        iend = map -> end( );
        iter = map -> find(str);
        
        if(iter == iend)    return local_ERROR::INVALID_ARG;                
        else arg = *(iter -> second);
    
        if(arg.what == ARG::NEXT)
        {
            map = arg.next;                // get next struct
            str = obj.get_arg( );          // get next arg
            continue;
        }
        
        if(arg.what == ARG::FUNCT)
        {                
            local_ERROR b = arg.funct(obj);    // execute command function
            return b;
        }    
        
    }

    return local_ERROR::SUCCESS;        
}

// test function with valid semantics
local_ERROR XX(ENTERED& obj) { std::cout << "_function" << std::endl; return local_ERROR::SUCCESS; }    

local_ERROR _exitterminal(ENTERED& obj)
{
    if(obj.get_left( )) return local_ERROR::EXCESS_ARG;
    return local_ERROR::EXIT;    
}


/* -------------- MOVEUP and MOVE FUNCTIONS ------------- */

local_ERROR _movedirup(ENTERED& obj)
{    
    if(obj.get_left( )) return local_ERROR::EXCESS_ARG;
    
    // check for rootpath ('C:\' etc. )    and make move up
    if(current_path[(current_path.length( ) - 2)] == ':')     return local_ERROR::ROOTPATH;    
    
    else 
    {
        current_path.pop_back( );
        current_path.erase(current_path.find_last_of(bs) + 1);
        return local_ERROR::SUCCESS;
    }
}

local_ERROR _movedir(ENTERED& obj)
{
    std::string path = obj.get_arg( );
    copy_path = current_path + path;
    
    if(obj.get_left( )) return local_ERROR::EXCESS_ARG;
    if(!path.length( )) return local_ERROR::EMPTY_ARG;
    if(!std::filesystem::exists(copy_path)) return local_ERROR::DOESNOT_EXIST;
    
    current_path = copy_path;
    copy_path = "";
    return local_ERROR::SUCCESS;
}



/* ------------------- SHOW FUNCTIONS ------------------- */

// print directory content, which user want
void print_dir(std::vector<std::filesystem::path>& dir_files, 
               std::vector<std::filesystem::path>& dir_other)
{    
    if(dir_files.size( ))
    {
        std::cout << "\nDIRECTORY FILES: " << std::endl;
        for(const auto& el : dir_files)
            std::cout << el.stem( ).string( )      << "    \t " 
                      << el.extension( ).string( ) << std::endl;                        
    }
    
    if(dir_other.size( ))
    {
        std::cout << "\nDIRECTORY OTHER: " << std::endl;    
        for(const auto& el : dir_other)
            std::cout << el.stem( ).string( ) << std::endl;             
    }
    
    _getch( );        
}

// get dir content and call print_dir( )
bool get_content(const char* what)    
{
    
    std::vector<std::filesystem::path> dir_files, dir_other, empty;
    dir_files.reserve(20);
    dir_other.reserve(20);
    
    for(const auto& el : std::filesystem::directory_iterator(current_path))
    {
        el.path( ).extension( ).string( ).length( )
        ? dir_files.push_back(el.path( ))
        : dir_other.push_back(el.path( ))
        ;          
    }
            
    if(what == "files") print_dir(dir_files, empty);
    if(what == "other") print_dir(empty, dir_other);
    if(what == "allof") print_dir(dir_files, dir_other);
    
    dir_files.clear( );    
    dir_other.clear( );    
    
    return 1;        
    
}

local_ERROR _showfiles(ENTERED& obj)
{
    if(obj.get_left( )) return local_ERROR::EXCESS_ARG;
    bool b = get_content("files");              
    return local_ERROR::SUCCESS;
}

local_ERROR _showother(ENTERED& obj)
{
    if(obj.get_left( )) return local_ERROR::EXCESS_ARG;
    bool b = get_content("other");
    return local_ERROR::SUCCESS;
}    

local_ERROR _showallof(ENTERED& obj)
{
    if(obj.get_left( )) return local_ERROR::EXCESS_ARG;
    bool b = get_content("allof");      
    return local_ERROR::SUCCESS;        
}    

local_ERROR _showcommands(ENTERED &obj)
{
    if(obj.get_left( )) return local_ERROR::EXCESS_ARG;
    for(const auto& el : allcommands)
        std::cout << el << std::endl;
    _getch( );    
    return local_ERROR::SUCCESS;    
}


/* ------------------- IEX FUNCTIONS ------------------- */    

local_ERROR _ifexistscurr(ENTERED& obj)
{
    std::string path = obj.get_arg( );         
    if(obj.get_left( )) return local_ERROR::EXCESS_ARG;        
    bool b = std::filesystem::exists(current_path + path);
    return b ? local_ERROR::SUCCESS : local_ERROR::DOESNOT_EXIST;
}

local_ERROR _ifexistspath(ENTERED& obj)
{
    std::string path = obj.get_arg( );
    if(obj.get_left( )) return local_ERROR::EXCESS_ARG;
    bool b = std::filesystem::exists(path);
    return b ? local_ERROR::SUCCESS : local_ERROR::DOESNOT_EXIST;
}



/* ------------------ MKDIR FUNCTIONS ------------------ */

local_ERROR _makedir(ENTERED& obj)
{
    std::string str1 = obj.get_arg( );    // directory name
    std::string str2 = obj.get_arg( );    // open directory or not
    
    if(str2.length( ) && (str2 != "in")) return local_ERROR::INVALID_ARG;
    if(obj.get_left( ))                  return local_ERROR::EXCESS_ARG;
    
    copy_path = current_path + str1 + bs;
    
    if(std::filesystem::exists(copy_path)) std::cout << "Directory already exists!" << std::endl;
    else std::filesystem::create_directory(copy_path);    

    if(str2 == "in") current_path = copy_path;
    copy_path = "";
    return local_ERROR::SUCCESS;        
}    



/* ------------------- DEL FUNCTIONS ------------------- */

local_ERROR _delete(ENTERED& obj)
{
    
    std::string path = obj.get_arg( );
    copy_path = current_path + path;
    
    if(obj.get_left( )) return local_ERROR::EXCESS_ARG;
    if(!path.length( )) return local_ERROR::EMPTY_ARG;
    if(!std::filesystem::exists(copy_path)) return local_ERROR::DOESNOT_EXIST;
    
    std::filesystem::remove_all(copy_path);
    copy_path = "";
    return local_ERROR::SUCCESS;        
    
}



/* ----------------- SETDIR FUNCTIONS ------------------ */

local_ERROR _setnotesdir(ENTERED& obj) 
{
    
    std::string path = obj.get_arg( );
    if(!path.length( )) return local_ERROR::EMPTY_ARG;
    if(obj.get_left( )) return local_ERROR::EXCESS_ARG;
    
    if(!std::filesystem::exists(path)) return local_ERROR::INVALID_PATH;
    
    std::ofstream write;
    write.open(mynotesfilepath);
    if(write.is_open( )) 
    {
        write << (path += bs);
        notes_dir = path;
        write.close( );        
    }
    
    return local_ERROR::SUCCESS;    
}



/* ------------------ NOTE FUNCTIONS ------------------- */

local_ERROR    _makenewnote(ENTERED& obj)
{
    std::string ext = ".txt";    // for user might enter only note name without .txt
    std::string name = obj.get_arg( );        
    if(!name.length( )) return local_ERROR::EMPTY_ARG;
    if(obj.get_left( )) return local_ERROR::EXCESS_ARG;
    
    if(name[0] != '#')  return local_ERROR::INVALID_ARG;
    else name.erase(0, 1);
    std::string fullname = notes_dir + name + ext;    
    
    if(std::filesystem::exists(fullname)) return local_ERROR::NOTE_EXISTS;
    else
    {
        std::ofstream note;
        note.open(fullname);
        note.close( );
    }
    
    return local_ERROR::SUCCESS;
            
};
    
local_ERROR    _deletenote(ENTERED& obj)
{
    
    std::string ext = ".txt";    
    std::string name = obj.get_arg( );
    if(!name.length( )) return local_ERROR::EMPTY_ARG;
    if(obj.get_left( )) return local_ERROR::EXCESS_ARG;
    
    if(name[0] != '#')  return local_ERROR::INVALID_ARG;
    else name.erase(0, 1);
    std::string fullname = notes_dir + name + ext;
    
    if(!std::filesystem::exists(fullname)) return local_ERROR::DOESNOT_EXIST;
    else std::filesystem::remove_all(fullname);    
    
    return local_ERROR::SUCCESS;
    
};
    
local_ERROR    _addtonote(ENTERED& obj)
{
    
    std::string ext = ".txt";    
    std::string name = obj.get_arg( );
    if(!name.length( )) return local_ERROR::EMPTY_ARG;
    
    if(name[0] != '#')  return local_ERROR::INVALID_ARG;
    else name.erase(0, 1);
    std::string fullname = notes_dir + name + ext;
    std::string write { };
    
    if(!std::filesystem::exists(fullname)) return local_ERROR::DOESNOT_EXIST;
    else
    {
        while(obj.get_left( )) write += (obj.get_arg( ) + " ");
        
        std::ofstream note;
        note.open(fullname, std::ios::app);
        
        auto t_point = std::chrono::system_clock::now( );
        std::time_t sys_time = std::chrono::system_clock::to_time_t(t_point);
        auto print_time = std::put_time(std::localtime(&sys_time), "%d.%m.%y");
        
        note << "#" << name << "\t" << print_time << "\n";
        note << write << "\n\n";
        
    }
    
    return local_ERROR::SUCCESS;
    
};
    
local_ERROR    _opennote(ENTERED& obj)
{
    std::string ext = ".txt";   
    std::string name = obj.get_arg( );
    if(!name.length( )) return local_ERROR::EMPTY_ARG;
    if(obj.get_left( )) return local_ERROR::EXCESS_ARG;    
    
    if(name[0] != '#')  return local_ERROR::INVALID_ARG;
    else name.erase(0, 1);
    std::string fullname = notes_dir + name + ext;
    
    if(!std::filesystem::exists(fullname)) return local_ERROR::DOESNOT_EXIST;
    else
    {
        HINSTANCE hins;
        HWND hwnd = NULL;
        const char* opparams = "open";
        hins = ShellExecute(    
                                hwnd, 
                                opparams, 
                                fullname.c_str( ), 
                                NULL, 
                                0, 
                                SW_SHOWNORMAL
                            );
        
        if(!hins) return local_ERROR::CANNOT_OPEN;
    }            
    
    return local_ERROR::SUCCESS;
};
    
local_ERROR _showalltags(ENTERED& obj)
{
    if(obj.get_left( )) return local_ERROR::EXCESS_ARG;    
    
    std::vector<std::filesystem::path> hashtags;
    hashtags.reserve(20);
    for(const auto& el : std::filesystem::directory_iterator(notes_dir))
    {
        if(el.path( ).extension( ).string( ).length( ))
            hashtags.push_back(el.path( ));    
    }

    for(const auto& el : hashtags)
        std::cout << "\t#" << el.stem( ).string( ) << std::endl;                          

    _getch( );
    return local_ERROR::SUCCESS;            
    
}

    
    
/* ---------------- ARGS and MAPS INIT. ---------------- */

ARG _id_first_args, _id_show_1, _id_ifex_1, _id_note_1;                                // map identificators (must be equal to the number of ARG strings below)
ARG _exit_, _moveup, _move, _show, _ifex, _mkdir, _mkfile, _del, _sndir, _note;        // map = FIRST_ARGS
ARG _showfil, _showoth, _showall, _showcom;                                            // map = _SHOW_1
ARG _ifexcurr, _ifexpath;                                                              // map = _IFEX_1
ARG _newnote, _delnote, _addnote, _runnote, _hashtags;                                 // map = _NOTE_1

std::map<std::string, ARG*> FIRST_ARGS = 
{
    {"id"     , &_id_first_args },
    {"exit"   , &_exit_  },            // exit terminal
    {"moveup" , &_moveup },            // move one directory up
    {"move"   , &_move   },            // move from current path
    {"show"   , &_show   },            // show directory content  ->  _SHOW_1  
    {"ifex"   , &_ifex   },            // check if object exists  ->  _IFEX_1  
    {"mkdir"  , &_mkdir  },            // create directory        
    {"mkfile" , &_mkfile },            //
    {"del"    , &_del    },            // delete dir and all in
    {"sndir"  , &_sndir  },            // set directory with notes
    {"note"    , &_note  },            // using notes functions   -> _NOTE_1
};

std::map<std::string, ARG*> _SHOW_1 = 
{
    {"id"  , &_id_show_1 },            
    {"fil" , &_showfil   },            // show files only
    {"oth" , &_showoth   },            // show other only
    {"all" , &_showall   },            // show all
    
    {"com" , &_showcom   },            // show all commands 
};

std::map<std::string, ARG*> _IFEX_1 = 
{
    {"id"   , &_id_ifex_1 },
    {"curr" , &_ifexcurr  },          // if exists in current directory
    {"path" , &_ifexpath  },          // if exists generally
};

std::map<std::string, ARG*> _NOTE_1 = 
{
    {"id"   , &_id_note_1 },
    {"new:" , &_newnote   },
    {"del:" , &_delnote   },        
    {"add:" , &_addnote   },
    {"run:" , &_runnote   },
    {"list" , &_hashtags  },        
};
    
void ARG_init( )
{    

    /* ----- map id's ----- */
    
    _id_first_args.map_id  = "FIRST_ARGS";
    _id_show_1.map_id      = "_SHOW_1";
    _id_ifex_1.map_id      = "_IFEX_1";
    _id_note_1.map_id      = "_NOTE_1";    
    
    /* ----- FIRST_ARGS ----- */

    _exit_.what = ARG::FUNCT;
    _exit_.funct = &_exitterminal;
    
    _moveup.what = ARG::FUNCT; 
    _moveup.funct = &_movedirup;
    
    _move.what = ARG::FUNCT; 
    _move.funct = &_movedir;
    
    _show.what = ARG::NEXT; 
    _show.next = &_SHOW_1 ;

    _ifex.what = ARG::NEXT; 
    _ifex.next = &_IFEX_1 ;
    
    _mkdir.what = ARG::FUNCT; 
    _mkdir.funct = &_makedir;
    
    _mkfile.what = ARG::FUNCT; 
    _mkfile.funct = &XX;
    
    _del.what = ARG::FUNCT; 
    _del.funct = &_delete;
    
    _sndir.what = ARG::FUNCT;
    _sndir.funct = &_setnotesdir;
    
    _note.what = ARG::NEXT;
    _note.next = &_NOTE_1;
    
    /* ------ _SHOW_1 ------ */
    
    _showfil.what = ARG::FUNCT;
    _showfil.funct = &_showfiles; 
    
    _showoth.what = ARG::FUNCT;
    _showoth.funct = &_showother; 
    
    _showall.what = ARG::FUNCT;
    _showall.funct = &_showallof; 
    
    _showcom.what = ARG::FUNCT;
    _showcom.funct = &_showcommands;
    
    /* ------ _IFEX_1 ------ */    
    
    _ifexcurr.what = ARG::FUNCT;
    _ifexcurr.funct = &_ifexistscurr;
    
    _ifexpath.what = ARG::FUNCT;
    _ifexpath.funct = &_ifexistspath;
    
    /* ------ _NOTE_1 ------ */
    
    _newnote.what = ARG::FUNCT;
    _newnote.funct = &_makenewnote;
    
    _delnote.what = ARG::FUNCT; 
    _delnote.funct = &_deletenote;
    
    _addnote.what = ARG::FUNCT;
    _addnote.funct = &_addtonote;
    
    _runnote.what = ARG::FUNCT;
    _runnote.funct = &_opennote;
    
    _hashtags.what = ARG::FUNCT;
    _hashtags.funct = &_showalltags;    

}


// main task = preparing + working loop 
int main( )
{
    
    current_path += bs;    // valid path format
    ARG_init( );           // commands initialization
    ENTERED obj;           // create obj, that will keep every new users enter
    
    mynotesfilepath = current_path + "[log] mynotes_dir.txt";
    if(!std::filesystem::exists(mynotesfilepath))
    {
        std::cout << "\n\tFile '[log] mynotes_dir.txt' doesnot exist";
        _getch( );
        return 0;
    }
    else
    {
        std::ifstream read;
        read.open(current_path + "[log] mynotes_dir.txt");
        if(read.is_open( )) std::getline(read, notes_dir);    
        else
        {
            std::cout << "\n\tCannot open '[log] mynotes_dir.txt' file";
            _getch( );
            return 0;
        }
        read.close( );
    }

    // main programm loop
    while(1)
    {
        
        // get new command
        terminal(current_path, obj);

        // use got command
        local_ERROR error = execution(obj, &FIRST_ARGS);
        if(error == local_ERROR::EXIT) break;
        else if(error == local_ERROR::SUCCESS) ;
        else
        {
            std::cout << "\terror code: " << static_cast<int>(error) << std::endl;    
            _getch( );
        }
            
    }
            
    std::cout << "\tend" << std::endl;
            
}

/** KEYBOARD TYPING PATTERNS
  *
  * programm collects and logs data:
  *    - every key event (up and down for every key)
  *    - time intervals between events
  *
  * then puts data to the log files:
  *     - EVENTS
  *    - INTERVALS
  *
  **/


#include <chrono>        // for class Timer
#include <iostream>      // console input and output
#include <conio.h>       // read one char input ............ getch( ), getche( )
#include <windows.h>     // Console API
#include <winuser.h>     // block users input .............. BlockInput( )
#include <fstream>       // file input and output
#include <sys\stat.h>    // check if file or folder exist .. stat( ) 
#include <dir.h>         // create folder .................. mkdir( )    
#include <thread>        // multi-thread
#include <cassert>


// function modifying std::cout functionality 
enum class COMMANDS 
{
    NONE  = 0,
    PAUSE = 1,
};
COMMANDS NONE  = COMMANDS::NONE;
COMMANDS PAUSE = COMMANDS::PAUSE;
void _MARK_(COMMANDS if_pause, const char* message = "", int code = -1) 
{
    code == -1
    ? std::cout << message << std::endl
    : std::cout << message << " " << code << std::endl;
    
    if(static_cast<bool>(if_pause))
    {
        getch( );
        std::cout << std::endl;
    }    
}


// function for check if folder or file already exists (and create folder if don't)    
// returns 1 if exist - return 0 if file doesn't exist (or if folder just have been created) 
enum class FILETYPE 
{
    FOLDER = 0,
    FILE   = 1,
};
FILETYPE FOLDER = FILETYPE::FOLDER;
FILETYPE m_FILE = FILETYPE::FILE;        // prefix "m_" because of conflict with <windows.h>    
                                        // (might be not used at all as if_exist( ) has default FILETYPE::FILE parameter)
bool if_exists(const char* path, FILETYPE what = FILETYPE::FILE)
{
    struct stat buf;
    if(!stat(path, &buf)) return 1;
    else
    {
        if(what == FOLDER) mkdir(path); 
        return 0;
    }
}


/** description:    
  *
  * the reason for create this class is possibility of
  * automatically restoring console mode after its changing
  *
  * in this program not necessary,
  * but might be later
  *
  **/
class HANDLER final
{
    private:    HANDLE    h_in, h_out;
                DWORD    old_imode, old_omode, new_mode;
                bool    if_ih_exist {0};
                bool    if_oh_exist {0};    
                 bool    if_ichanged {0};
                 bool    if_ochanged {0};            
                
    public:
    
        HANDLER( ) 
        {  
            GetConsoleMode(h_in , &old_imode); 
            GetConsoleMode(h_out, &old_omode); 
        }
        
        HANDLE get_ihandle( )
        {
            if(!if_ih_exist)
            {
                h_in = GetStdHandle(STD_INPUT_HANDLE);
                if(h_in == INVALID_HANDLE_VALUE) /** add **/ ;    
                if_ih_exist = 1;
            }    
            
            return h_in;                  
        }    
        
        HANDLE get_ohandle( )
        {
            if(!if_oh_exist)
            {
                h_out = GetStdHandle(STD_OUTPUT_HANDLE);
                if(h_out == INVALID_HANDLE_VALUE) /** add **/ ;    
                if_oh_exist = 1;
            }
            
            return h_out;                  
        }            

};


// create std::ofstream object, link with the file, automatically close file + override operator<<
class LOG_FILE final
{
    private:    std::ofstream m_file;
                bool is_open {0};
    
    public:
        
        // default ctor, copy ctor, overloaded operator= are forbidden
        LOG_FILE( )                                 = delete;
        LOG_FILE(const LOG_FILE& obj)                 = delete;
        LOG_FILE& operator= (const LOG_FILE& obj)     = delete;
        
        // create std::fstream object - choose file write mode - open feli to write 
        LOG_FILE(const char* name)
        {
            if(if_exists(name))   ;            // open with std::ios::app flag (temporary skipped)
            else m_file.open(name);
            
            if(m_file.is_open( )) is_open = 1;
        }
    
        ~LOG_FILE( ) {  if(is_open) m_file.close( );  }        // close file in dtor
        
        // overloading operator<< for comfortable using
        friend LOG_FILE& operator<< (LOG_FILE& file, WORD ker);
        friend LOG_FILE& operator<< (LOG_FILE& file, const char* message);
        friend LOG_FILE& operator<< (LOG_FILE& file, std::ostream&(*param)(std::ostream&));    
        friend LOG_FILE& operator<< (LOG_FILE& file, std::ios_base& manip(std::ios_base& str));            
};

// overloading operator<< for LOG_FILE class
LOG_FILE& operator<< (LOG_FILE& file, WORD ker)                                  {
    file.m_file << ker;
    return file;        
}
LOG_FILE& operator<< (LOG_FILE& file, const char* message)                          {
    file.m_file << message;
    return file;        
}
LOG_FILE& operator<< (LOG_FILE& file, std::ostream&(*endl)(std::ostream&))       {
    file.m_file << endl;
    return file;        
}
LOG_FILE& operator<< (LOG_FILE& file, std::ios_base& manip(std::ios_base& str))  {
    file.m_file << manip;
    return file;        
} 

  
// class for getting time_points of every console event
template <size_t _SIZE>
class TIMER final
{
    using clock_t = std::chrono::high_resolution_clock;
    using milliseconds = std::chrono::milliseconds;
    
    public:         static const int max_number {_SIZE};                // max number of any key_events (including pressing and releasing as well)
    private:     int count {0};                                        // counter for add_point( ) method
                 clock_t::time_point time_points[max_number];        // array for keeping time_point of every key_event
    
    public:
        
        void add_point( ) 
        {  
            time_points[count] = clock_t::now( );                    // just adding time_point - all of time_intervals computings will be after the collecting data
            ++count;
        }
        
        void log_intervals(LOG_FILE& file)    
        {
            for(int i = 0; i < max_number - 1; ++i)
            {
                std::chrono::duration<double, std::milli> interval = time_points[i + 1] - time_points[i];
//                std::cout << interval.count( ) << std::endl;    
                file << interval.count( ) << std::endl;
            }                            
        }

};  
  
  
// info format(std::dex, separated by a space):  bKeyDown wVirtualKeyCode dwControlKeyState    
void key_event_actions(LOG_FILE& file, KEY_EVENT_RECORD &ker) {  
    file << std::hex << ker.bKeyDown << " " << ker.wVirtualKeyCode << " " << ker.dwControlKeyState << std::endl;
} 

// ring buffer (just for exercise)
template <uint8_t _SIZE, class _TYPE>
class RBUFFER 
{
    
    static_assert((_SIZE & (_SIZE - 1)) == 0, "class BUFFER: SIZE is not a power of 2");
    using INDEX = uint8_t;        // from 0 to 255
    
    protected:
        
        const INDEX mask  {_SIZE - 1};        // e.g. _SIZE = 8 = 0...1000, mask = 7 = 0...0111 -> applying mask = always in array's range
        _TYPE buffer[_SIZE]       { };        
        INDEX w_count {0}; 
        INDEX r_count {0};

    public:
        
        bool write(_TYPE &value)
        {
            // taking overflow into account
            if((INDEX(w_count - r_count) & INDEX(~mask)) == 0)        
            {
                // write value + increment counter
                buffer[w_count++ & mask] = value;    
                return 1;
            }
    
            return 0;
        }
        
        bool read( )
        {
            if(w_count != r_count)
            {
                // read value + increment counter
                buffer[r_count++ & mask];    
                return 1;
            }

            return 0;
        }
        
};

template <uint8_t _SIZE, class _TYPE>
class RBUFFER_file : public RBUFFER<_SIZE, _TYPE>
{    };

template <uint8_t _SIZE>
class RBUFFER_file<_SIZE, KEY_EVENT_RECORD> : public RBUFFER<_SIZE, KEY_EVENT_RECORD>
{
    public:
        
        int read(LOG_FILE& file)
        {
            if((*this).w_count != (*this).r_count)
            {    
                if((*this).buffer[(*this).r_count & (*this).mask].uChar.AsciiChar == 13)
                    return 13;
                
                file << std::hex << (*this).buffer[(*this).r_count & (*this).mask].bKeyDown          << " " <<    
                                    (*this).buffer[(*this).r_count & (*this).mask].wVirtualKeyCode   << " " <<
                                    (*this).buffer[(*this).r_count & (*this).mask].dwControlKeyState << std::endl;
                
                ++(*this).r_count;    
                return 1;
            }

            return 0;
        }    
        
};

template <uint8_t _SIZE>    
void log_to_file(RBUFFER_file<_SIZE, KEY_EVENT_RECORD>* buffer, LOG_FILE* file)
{
    int stop = 0;
    while(stop != 13)
        stop = buffer -> read(*file);
}


int main( )
{
    
    const int num_of_symbols = 200;
    const int num_of_events  = num_of_symbols * 3;
    const uint8_t rbuf_size  = 8;
    
    TIMER<num_of_events> time; 
    LOG_FILE file_events("EVENTS.txt");
    RBUFFER_file<rbuf_size, KEY_EVENT_RECORD> buffer;
    
    std::thread th(log_to_file<rbuf_size>, &buffer, &file_events);
    
    HANDLER hndl;
    HANDLE  out_hndl = hndl.get_ohandle( );
    HANDLE   in_hndl = hndl.get_ihandle( );
    
    DWORD           num_of_records_read, num_of_records_written;                    
    DWORD           input_bufsize {1} ;
    INPUT_RECORD    input_buffer[input_bufsize];
    INPUT_RECORD    rec;
    char            backspace[] = {0x08, 0x00, 0x08}; 
    
    BOOL& key_down = rec.Event.KeyEvent.bKeyDown;
    CHAR& ascii    = rec.Event.KeyEvent.uChar.AsciiChar;
    
    while(ascii != 13)
    {
        
        // read every pressed key
        ReadConsoleInput        
        (
            in_hndl,                
            &rec,                    
            1,                        
            &num_of_records_read    
        );    
        
        // get timepoint information
        time.add_point( );    
        buffer.write(rec.Event.KeyEvent);            
        
        // echo printing characters 
        if(key_down and ((ascii > 0x1F) or (ascii < 0x00)))        
        WriteConsole
        (
            out_hndl, 
            &ascii,     
            1, 
            nullptr, 
            NULL
        );    

        // echo backspace key
        else if (key_down and ascii == 0x08)          
        WriteConsole 
        (
            out_hndl, 
            &backspace, 
            3, 
            nullptr, 
            NULL
        );
                    
    }    
    
    LOG_FILE file_intervals("INTERVALS.txt");
    time.log_intervals(file_intervals);
    std::cout << std::endl << std::endl << "END OF LOGGING" << std::endl;

    
    th.join( );
    return 0;

}
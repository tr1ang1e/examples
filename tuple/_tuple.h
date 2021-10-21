
/** my own TUPLE 
  *    
  * used elements:
  *    - recursive function
  *    - variadic templates
  *
  * main implementation idea:
  *    - place elements into std::vector<char> byte by byte
  *    - keep information about element's index, type and size in std::map
  *    - get element using reinterpret_cast< >( )
  *
  * to get element from tuple:
  *    _tuple t = { *elements* };
  *    T value = t.get(value, index);
  * 'value' as an argumnet is for using RTTI
  * (might be improve by using concepts)
  *
  * if get( ) failures:
  *    - or element has wrong size 
  *    - or incorrect index in args
  *    - or element has wrong type
  * get( ) returns T( )
  * else get returns right element
  *
  **/

#pragma once

#include <vector>
#include <map>
#include <typeinfo>
#include <typeindex>
#include <string>

class _tuple final
{
    private:
        
        // container - all data is in the order
        const size_t _bytes;                           // sum of all input arguments    
        std::vector<char> _storage { };                // char - because it takes 1 byte
        size_t _index {0};                             // needed for calling place( ) method
                
        // info about indexes and relevant data type
        std::map<std::pair<size_t, size_t>, std::pair<size_t, std::string>> _types { };
        size_t _mapindex {0};                // needed for calling place( ) method        
            
        // method for place input data contiguously in memory
        template<typename T>
        void place(T& arg)
        {
            // add information to map
            _types[{_mapindex, sizeof(T)}] = {typeid(arg).hash_code( ), typeid(arg).name( )};
            ++_mapindex;
            
            // place next argument to vector byte by byte
            size_t size {sizeof(arg)};
            char* ptr = reinterpret_cast<char*>(&arg);
            for(size_t s = 0; s < size; ++s)
            {
                _storage[_index] = *(ptr + s);
                ++_index;                        
            }    
        }
        
    public:
    
        _tuple( )                 = delete;
        _tuple(const _tuple&)     = delete;
    
        // template ctor
        template<typename ... Types>
        _tuple(Types ... args) : _bytes{(sizeof(args) + ...)}
        {    
            _storage.resize(_bytes);        // set appropriate vector size
            (place(args), ...);             // 'fold_expression' = place(arg1); place(arg2); and so on    
        }
    
        // if there is an error anywhere in calling - returns default object of type T
        template<typename T>
        T get(T sample, size_t index)
        {
            
            T temp { };    // return value
            auto mend = _types.end( );                            
            auto elem = _types.find({index, sizeof(T)});
            
            if(elem == mend) ;    // index doesnot exist or element has another size
            else
            {
                if(typeid(T).hash_code( ) != elem -> second.first) ;    // if element has another type (hash_codes guarantee)
                else
                {
                    // establish correspondance between (index from map) and (vector[index])
                    size_t shift { };
                    for(auto iter = _types.begin( ); iter != elem; ++iter)
                        shift += iter -> first.second;
                    
                    T* ptr = reinterpret_cast<T*>(&_storage[shift]);
                    temp = *ptr;
                }
            }
            
            return temp;
        }
    
        // print all of tuple elements (for test)
        void pelems( )
        {
            std::cout << " INDEX SIZE TYPE" << std::endl << std::endl;
            for(auto [key, value] : _types)
                std::cout << "    " << key.first 
                          << "    " << key.second
                          << "    " << value.second << std::endl;
            std::cout << std::endl;
            std::cout << " args size = " << _bytes        << std::endl;
            std::cout << " obj. size = " << sizeof(*this)    << std::endl;        
        }
        
        size_t gbytes( ) { return _bytes; }
    
};
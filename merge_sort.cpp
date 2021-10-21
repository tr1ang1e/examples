
/** MERGE SORT
  * 
  * - array must be contiguous 
  *
  * - there are three appropriate STL containers:
  *       std::string
  *       std::array
  *       std::vector
  *   for calling sorting algorithm with one of these 
  *   containers use syntax: _sort<condition>(container);
  *       condition is optional (to_greater by default)
  *       auto checking if container is appropriate
  *
  * - algorithm might also be calling with pointer
  *   to the first element of sorting array - 
  *   programmer is responsible for data is contiguous
  *   syntax: _sort<condition>(ptr, size)
  *       ptr is pointer to the first element
  *       size is number of sorting elements
  *
  **/

#include <iostream>
#include <vector>
#include <memory>
#include <concepts>
#include <iterator>

using namespace std;
using _st = std::size_t;

// protection from using not contiguous data
template<typename T>
concept contiguous = 
    std::contiguous_iterator<typename T::iterator>
    or std::is_pointer<T>::value 
;

enum _condition
{ 
    to_less, 
    to_greater,
};

template<typename T, _condition C>
bool _comparator(T a, T b)
{
    return C ? (a < b) : (a > b) ; 
}

// results will be placed into the same contsiner
template<typename T, _condition C>
void _merge(T* container, _st cont_size)
{
    
    _st l_size = cont_size / 2;
    _st r_size = cont_size - l_size;
    
    auto temp = std::make_unique<T[ ]>(cont_size);
    
    _st l_idx = 0;
    _st r_idx = l_size;
    _st t_idx = 0;
    
    while (l_idx < l_size && r_idx < cont_size)
    {

        temp[t_idx++] = _comparator<T, C>(container[l_idx], container[r_idx])
                      ? container[l_idx++] 
                      : container[r_idx++] 
                      ;
    }    
    
    _st rem_num = (l_size - l_idx) + (cont_size - r_idx);    
    _st rem_idx =  l_idx == l_size 
                ?  r_idx 
                :  l_idx
                ;
    
    for (_st i = 0; i < rem_num; ++i) 
        temp[t_idx++] = container[rem_idx++];
        
    for (_st i = 0; i < cont_size; ++i)
        container[i] = temp[i];
    
}


// recursive separation
template<typename T, _condition C>
void _separate(T* container, _st cont_size)
{
    
    _st l_size = cont_size / 2;
    _st r_size = cont_size - l_size;
    
    if (cont_size > 1)
    {
        
        _separate<T, C>(&container[0]      , l_size);
        _separate<T, C>(&container[l_size] , r_size);
                
        _merge<T, C>(&container[0], cont_size);    
    
    }            
}


template<_condition C = _condition::to_greater, contiguous T>
void _sort(T& container, _st size = 0)
{
    // check if arg is pointer or container as is
    if constexpr (std::is_pointer<T>::value) 
    {
        _separate<T, C>(container, size);    
    }
    else
    {
        _separate<typename T::value_type, C>(&container[0], container.size( ));    
    }
}


int main( ) 
{

    std::vector<int> container = {17, 1, 2, 12, 37, 120, 63, 29, 81, 69, 43, 56, 9, 19, 33, 42, 77};
    _sort<to_greater>(container);

    for (const auto& el : container)
        cout << el << ", ";
    cout << endl;    

}
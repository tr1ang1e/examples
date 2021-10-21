
/** ZIP
  *
  * exercise to make class
  * which would create the pair of iterators
  *
  **/

#pragma once

template<typename T, typename S>
struct zip
{
    
    zip(T& c1, S& c2) 
    : it(c1.begin( ), c2.begin( ), c1.end( )) 
    {  }
    
   ~zip( ) = default;
    
    struct iter
    {
        
        typename T::iterator fir;
        typename S::iterator sec;
        typename T::iterator end;
        
        struct iboth
        {
            typename T::value_type fir_value;
            typename S::value_type sec_value;
            
        } value ;    
        
        iter(typename T::iterator i1, typename S::iterator i2, typename T::iterator i3) 
        : fir {i1} , sec {i2} , end {i3}
        {    }
        
        ~iter( ) = default;
        
        iter& operator++ ( )
        {
            ++fir;
            ++sec;
            return *this;
        }
        
        bool operator== (std::default_sentinel_t)
        {
            return fir == end ;
        }
        
        iboth operator* ( )
        {
            value.fir_value = *fir;
            value.sec_value = *sec;
            return value;    
        }
                
    } it;
    
    iter begin( ) { return it; }
    std::default_sentinel_t end( ) { return { }; }
    
};
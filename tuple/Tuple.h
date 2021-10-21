
/** TUPLE as an exercise (habr example)
  *    
  * used elements:
  *    - recursive templates
  *    - variadic templates
  *
  * main implementation idea:
  *    - create the chain of instantiated classes
  *    - every instantiation has:
  *         additional pointer to base class
  *         additional member with value 
  *
  * when user tries to get value
  * tuple firstly takes access to the 
  * one of the pointers to the base class in tuple - 
  * then returns value of this class. All of
  * value members have the same names, but
  * get( ) returns right because search begins
  * from the class was derived last/
  *
  **/

#pragma once

//    main template - never istantiates but base for others
template<typename ... Args>
struct Tuple;

// recursion
template<typename Head, typename ... Tail>
struct Tuple<Head, Tail ...>  : Tuple<Tail ...>
{
    typedef Tuple<Tail ...> base_type;        
    base_type& base = static_cast<base_type&>(*this);
    
    typedef Head head_type;
    Head head_;
    
    // ctor    firstly goes to the bottom of recursion and then initializes head_ 
    Tuple(Head h, Tail ... t) 
    : Tuple<Tail ...>(t ...), head_ {h}
    {    }        
};

// base case
template< >
struct Tuple< >
{    };    

// getter - for having partial function spasialization oportunity
template<size_t Index, typename Head, typename ... Tail>
struct getter
{
    // typedef to use 'head_type' typename in getter<0, Head, Tail ...> despite of 'return_type'
    typedef typename getter<Index - 1, Tail ...>::return_type return_type;
    static return_type get(Tuple<Head, Tail ...> t)
    {
        
        return getter<Index - 1, Tail ...>::get(t);
    }
        
};

// recursion bottom
template<typename Head, typename ... Tail>
struct getter<0, Head, Tail ...>
{
    
    typedef typename Tuple<Head, Tail ...>::head_type return_type;
    static return_type get(Tuple<Head, Tail ...> t)
    {
        return t.head_;
    }    
    
};

template<size_t Index, typename Head, typename ... Tail>
typename getter<Index, Head, Tail ...>::return_type get(Tuple<Head, Tail ...> t)
{
    return getter<Index, Head, Tail ...>::get(t);
}
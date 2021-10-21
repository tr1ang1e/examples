
#include <iostream>
#include "_tuple.h"
#include "Tuple.h"

struct mytype 
{    
};

int main( )
{
    
    // ********* testing my tuple ********* //
    
    int i = 2; 
    int* p = &i;
    mytype my;
    long long ll = 89;
    
    _tuple _t(1, p, 3.14, 'A', my, ll);
    _t.pelems( );
    
    // test_1, 3.14 should be printed
    double test_1 = _t.get(test_1, 2);
    std::cout << std::endl << " test_1 = " << test_1;
    
    // test_2, 0 should be printed because of get( ) failure
    int test_2 = _t.get(test_2, 1);
    std::cout << std::endl << " test_2 = " << test_2 << std::endl;
    
    
    
    // ***** testing exercise tuple ***** //
    
    Tuple<int, double, char> t {1, 3.14, 'A'};
    auto test_3 = get<1>(t);
    std::cout << std::endl << " test_3 = " << test_3 << std::endl;
    

}



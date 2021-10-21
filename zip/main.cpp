        
#include <iostream>
#include <vector>
#include <list>
#include "zip.h"

int main( ) 
{

    std::vector<int> a {0, 2, 4, 6, 8};
    std::list<int> b {1, 3, 5, 7, 9};
    int res = 0;
    
    for(const auto& [x, y] : zip(a, b)) 
        res += x + y;    
    std::cout << res << std::endl;    

}        
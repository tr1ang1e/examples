
#include <iostream>
#include <initializer_list>
#include <memory>    
#include "neuron.h"    

class Timer
{
    using clock_t = std::chrono::high_resolution_clock;
    using microseconds = std::chrono::microseconds;
    
    public:
    Timer( ) : start_(clock_t::now( ))            // start timer 
    {    }
    
    ~Timer( )        // end timer - count result - print result
    {
        const auto finish = clock_t::now( );
        const auto us = std::chrono::duration_cast<microseconds> (finish - start_).count( );
        std::cout << us << " micro_seconds (us)" << std::endl;
    }
    
    private:
    const clock_t::time_point start_;
};



int main( )
{

    Timer t;
    
    {    
        
        try
        {
            std::unique_ptr<NEURON> obj ( new NEURON {{{1, 0, 0}, {1, 0, 1}, {0, 1, 1}, {0, 0, 0}}, {1, 1, 0, 0}} );
            std::cout << "START POINT" << std::endl;
            obj->print_info(1, 1, 1);
            std::cout << std::endl;
            
            std::cout << "WEIGHTS after 10k EPOCHs" << std::endl;
            obj->learn(10000, "sigmoid");
            obj->print_info(1);
            std::cout << std::endl;
            
            int answer_0 = obj->use({0, 1, 0});
            int answer_1 = obj->use({1, 1, 0});
            std::cout << "answer (exp. 0) = " << answer_0 << std::endl << "answer (exp. 1) = " << answer_1 << std::endl;
        }
        catch (const std::exception& exc)
        {
            std::cout << "Exception generated: " << exc.what( ) << std::endl;
        }

    }
    
    std::cout << std::endl;    
    return 0;
        
}
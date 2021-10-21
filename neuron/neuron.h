
/** NEURON description
  *
  * - only one neurone
  * - matric imitation - dynamic allocation of double* inspite of double** 
  * - all of matrix elements are located contiguously
  * - available activation functions:
  *     none (linear)
  *     sigmoid
  *     relu
  *
  **/    
  
#pragma once

#include <random>                // create random weights at the start
#include <chrono>                // for class Timer 
#define _USE_MATH_DEFINES        // necessary for some compilers to use constants
#include <cmath> 

class NEURON
{
    using   LIST = std::initializer_list<double>;
    using L_LIST = std::initializer_list<LIST>;
    
/* ================================ FIELDS ================================ */    

    private:
    
        size_t dim_0;                // number of strings in data_matrix
        size_t dim_1;                // number of columns in data_matrix
        
        double* data_matrix;         // input X = default data_matrix
        double* results_vector;      // input Y = default outputs
        
        double* learn_results;       // while_learning Y
        double* mismatch_vector;     // differense between input and while_learning Y
            
        double*  weights_vector;     // input W
        enum activ_funct             // for keeping users choice of activation function
        {
            NONE,
            SIGMOID,
            RELU,
        };
        activ_funct current_funct = NONE;

/* ====================== CONSTRUCTORS and DESTRUCTOR ====================== */
    
    private:
    
        // method for data_matrix and data_t memory allocating
        void create_data(size_t dim_0, size_t dim_1)
        {
            // allocate memory for data_matrix
            // all elements will be allocated contiguously
            data_matrix = new double[dim_0 * dim_1];
        }

        // method for fill data_matrix with data    
        void fill_data(const L_LIST &list_data)
        {
            size_t num = 0;                                
            for(auto &list_el : list_data)
            {
                for(auto &el : list_el)
                {
                    data_matrix[num] = el;
                    ++num;
                }
            }
        }
        
        // method for results_vector memory allocate    
        void create_results(size_t dim_0)
        {
            // allocate memory for results_vector
            results_vector = new double[dim_0];
            
            // allocate memory for learn_results
            learn_results = new double[dim_0];
            
            // allocate memory for mismatch_vector    
            mismatch_vector = new double[dim_0];    
        }
    
        // method for fill results_vector with results
        void fill_results(const LIST &list_results)
        {
            int y = 0;
            for(auto &el : list_results)
            {
                results_vector[y] = el;
                ++y;
            }
        }
    
        // method for weights_vector memory allocate
        void create_weights( )
        {
            weights_vector = new double[dim_1];
        }
    
        // method for fill weights_vector with random doubles
        void fill_weights( )
        {
            float min_start = -1.0;
            float max_start =  1.0;
            std::random_device first;
            std::mt19937 rand(first( ));
            std::uniform_int_distribution<> dist(0, RAND_MAX);    
            for(size_t i = 0; i < dim_1; ++i)
                weights_vector[i] = min_start + static_cast<double>(dist(rand)) / (static_cast<double>(RAND_MAX/(max_start - min_start)));    
        }
    
    public:    
    
        NEURON( ) = delete;        // can't define data_matrix' size
        
        // matrix filled by given constants
        NEURON(size_t y, size_t x, int el = 0) : dim_0{y}, dim_1{x}
        {
            // checking if matrix' size is invalid
            if(!(dim_0 && dim_1))  throw std::runtime_error("wrong size");
            
            create_data(dim_0, dim_1);        // create data_matrix
            create_results(dim_0);            // create results_vector
            
            // fill result_vector with given value (by default = 0)
            for(size_t i = 0; i < dim_0; ++i)
                results_vector[i] = el;

            // fill data_matrix with given value (by default = 0)
            for(size_t i = 0; i < dim_0 * dim_1; ++i)
                data_matrix[i] = el;    
            
            // create and fill weights_matrix
            create_weights( );
            fill_weights( );            
        }
        
        NEURON(const L_LIST &list_data, const LIST &list_results = {})
        {
            // checking if matrix is empty
            dim_0 = list_data.size( );
            if (!dim_0) 
                throw std::runtime_error("null matrix");
            
            // checking if matrix' columns have no_or_different number of elements
            auto list_el = list_data.begin( );                
            dim_1 = list_el->size( );
            if (!dim_1) throw std::runtime_error("null matrix");
            for(auto &list_el : list_data)
                if(list_el.size( ) != dim_1) 
                    throw std::runtime_error("wrong number of elements");
            
            // checking if number of results is the same as dim_0
            if ((list_results.size( ) != 0) && (list_results.size( ) != dim_0))
                throw std::runtime_error("wrong number of results");
            
            create_data(dim_0, dim_1);        // create data_matrix
            create_results(dim_0);            // create results_vector
            
            // fill data_matrix and data_t with given value
            fill_data(list_data);
            
            // fill results_vector with 0 or given data
            if (list_results.size( ) == 0)
                for(size_t i = 0; i < dim_0; ++i)
                    results_vector[i] = 0;
            else
                fill_results(list_results);
                
            // create weights_matrix
            create_weights( );
            fill_weights( );
        }
        
        ~NEURON( )
        {
            delete[ ] data_matrix;    
            delete[ ] results_vector;
            delete[ ] learn_results;
            delete[ ] mismatch_vector;
            delete[ ] weights_vector;
        }
        
/* ========================= ACTIVATION FUNCTIONS ========================== */    

private: double sigmoid (double input) {  return 1 / (1 + pow(M_E, input));  }
         double relu    (double input) {  return input > 0 ? input : 0;      }

            

/* ============================ PUBLIC METHODS ============================= */    
        
    public:
    
        // print matrix to console
        void print_info(bool wei = 0, bool data = 0, bool res = 0)
        {
            
            if(data)    // print data_matrix
            {
                std::cout << "data = ";
                size_t step = 0;        // step for matrix imitation
                for(size_t i = 0; i < dim_0; ++i)
                {
                    for(size_t j = 0; j < dim_1; ++j)
                        std::cout << data_matrix[j + step] << " ";
                    std::cout << std::endl << "       ";
                    step += dim_1;        // increase step so the next line of matrix is using    
                }    
                std::cout << std::endl;
            }
            
            if(res)        // print results_vector
            {
                std::cout << "results = ";
                for(size_t i = 0; i < dim_0; ++i)
                    std::cout << results_vector[i] << " ";
                std::cout << std::endl;
                std::cout << std::endl;
            }
            
            if(wei)        // print weights_vector
            {
                std::cout << "weights = ";
                for(size_t i = 0; i < dim_1; ++i)
                {
                    std::cout << (weights_vector[i] >= 0 ? " " : "");
                    std::cout << weights_vector[i] << std::endl;
                    std::cout << "          ";
                }
            }
        }
    
        // set data_matrix value    
        void set_data(const L_LIST &list)
        {
            // checking if data size is wrong
            if(list.size( ) != dim_0)              
                throw std::runtime_error("wrong number of strings");
            
            auto list_el = list.begin( );
            size_t new_dim_1 = list_el->size( );            
            
            if(new_dim_1 != dim_1) 
                throw std::runtime_error("wrong number of columns");
            
            for(auto &list_el : list)
                if(list_el.size( ) != new_dim_1) 
                    throw std::runtime_error("wrong number of elements");
            
            // fill data_matrix and data_t with given value 
            fill_data(list);
        }
    
        // set results_vector value    
        void set_results(const LIST &list)
        {
            // checking if data size is wrong
            if(list.size( ) != dim_0) 
                throw std::runtime_error("wrong number of results");
        
            fill_results(list);    
        }    
    
        // reset weights_vector
        void reset_weights( )
        {
            fill_weights( );
        }
    
        // learning neuron
        void learn(size_t epoch, const char* f_activ = "sigmoid")
        {
            
            size_t step = 0;        
            for(size_t i = 0; i < epoch; ++i)
            {
                
                // weighted sum
                for(size_t i = 0; i < dim_0; ++i)
                {
                    learn_results[i] = 0;
                    for(size_t j = 0; j < dim_1; ++j) 
                        learn_results[i] += data_matrix[j + step] * weights_vector[j];
                    step += dim_1;
                }
                step = 0;        // reset step for using later
                
                // activation
                if(f_activ != "none")
                {
                    for(size_t i = 0; i < dim_0; ++i)
                    {
                        if(f_activ == "sigmoid")    
                        {
                            learn_results[i] = sigmoid(learn_results[i]);
                            current_funct = SIGMOID;    
                        }    
                        
                        else if(f_activ == "relu")    
                        {
                            learn_results[i] = relu(learn_results[i]);
                            current_funct = RELU;    
                        }
                        
                        // FOR FUTURE - other functions ...
                    }    
                }
                    
                // mismatch calculation
                for(size_t i = 0; i < dim_0; ++i)
                    mismatch_vector[i] = learn_results[i] - results_vector[i];
                    
                // weights update calculation [MOST MYSTERIOUS MOMENT] + updating
                for(size_t i = 0; i < dim_1 ; ++i)
                {
                    step = 0;
                    double upd_weight = 0;
                    for(size_t j = 0; j < dim_0; ++j)
                    {
                        upd_weight += data_matrix[i + step] * (  mismatch_vector[j] * (learn_results[j] * (1 - learn_results[j]))  );    
                        step += dim_1;    
                    }
                        
                    weights_vector[i] += upd_weight;    
                }
                step = 0;
            }
        }
    
        // using learned neuron
        int use(const LIST &list)
        {
            if(list.size( ) != dim_1) 
                throw std::runtime_error("wrong number of data");
            
            double answer = 0;
            size_t x = 0;
            
            // weighted sum
            for(auto &el : list)
            {
                answer += el * weights_vector[x];
                ++x;    
            }
            
            // activation
            if(current_funct != NONE)
            {
                
                switch (current_funct)
                {
                    case SIGMOID:
                        answer = sigmoid(answer);
                        break;
                    case RELU:
                        answer = relu(answer);
                        break;
                    default:
                        break;    
                }

            }
                        
            return answer >= 0.5 ? 1 : 0;
        }
        
};
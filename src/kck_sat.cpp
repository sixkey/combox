

#include "kck_sat.hpp"

void add_cnf( std::shared_ptr< sat_solver > solver, cnf_t cnf )
{
    for ( auto &clause : cnf )
    {
        for ( int lit : clause ) {
            solver->add( lit );
            //std::cout << lit << ", ";
        }
        //std::cout << "\n";
        solver->add( 0 );
    }
}

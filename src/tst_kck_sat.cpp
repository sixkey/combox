#include "kck_sat.hpp"
#include "kck_form.hpp"
#include "kck_cnf.hpp" 

using var_t = std::string;
using lit_t = kck::literal< var_t >;
using forms = kck::forms_t< lit_t >;
using cnf_t = kck::cnf_t< lit_t >;

lit_t labeler( int i ) { return { "X_" + std::to_string( i ), true }; }

void test_case( forms::form_p form, bool t )
{
    auto [ cnf_tran, mapping ] = kck::to_int_cnf< lit_t >( kck::cnf_leaf( form->to_cnf( labeler ) ) );

    kck::sat_solver_t solver;
    kck::add_cnf( solver, cnf_tran );

    int res = solver.solve();

    assert( res != SAT_U );
    assert( ( res == SAT_Y ) == t );
}

int main()
{
    test_case( forms::f_and( { forms::f_lit( { "A", true } )
                             , forms::f_lit( { "B", true } ) } )
             , true );
    test_case( forms::f_or( { forms::f_lit( { "A", true } )
                            , forms::f_lit( { "A", false } ) } )
             , true );
    test_case( forms::f_and( { forms::f_lit( { "A", true } )
                             , forms::f_lit( { "A", false } ) } )
             , false );
    test_case( forms::f_and( { forms::f_or( { forms::f_lit( { "A", true } )
                                            , forms::f_lit( { "B", true } ) } )
                             , forms::f_or( { forms::f_lit( { "A", false } )
                                            , forms::f_lit( { "B", true } ) } )
                             , forms::f_or( { forms::f_lit( { "A", true } )
                                            , forms::f_lit( { "B", false } ) } )
                             , forms::f_or( { forms::f_lit( { "A", false } )
                                            , forms::f_lit( { "B", false } ) } ) } )
             , false );
}

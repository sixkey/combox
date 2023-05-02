#include <memory>
#include <iostream>
#include <string>

#include "kck_form.hpp"
#include "kck_cnf.hpp"

using namespace kck;


using lit_t = literal< std::string >;

lit_t str_labeler( int i )
{
    return { "x_" + std::to_string( i ), true };
}


kck::formula_ptr< lit_t > s_lit( std::string v 
                               , bool pos ) 
{
    return make_lit< lit_t >( { v, pos } );
}

int main()
{
    formula_ptr< lit_t > node = make_and< lit_t >( 
        { make_or< lit_t >( { s_lit( "A", false )
                            , s_lit( "A", false ) } )
        , make_or< lit_t >( { s_lit( "A", true )
                            , s_lit( "A", true ) } ) } );

    cnf_t< lit_t > cnf = node->to_cnf( str_labeler );

    std::cout << node->to_string() << std::endl;
}

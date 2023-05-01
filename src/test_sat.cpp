#include <string>
#include "to_cnf.hpp"
#include "sat.hpp"

using form_t = form_ptr< std::string >;

void test_case( form_t form, bool sat )
{
    auto res = sat_solve( form ); 
    if (  res.res == SAT_U 
       || res.res != ( sat ? SAT_Y : SAT_N ) ) {
        std::cout << form->to_string() << " expected " << sat << " got " 
                  << res.res << " with " << res.to_string();

    }
}


int main()
{
    test_case( make_lit< std::string >( "A" ), true );
    test_case( make_lit< std::string >( "A", false ), true );
    test_case( make_and< std::string >( { make_lit< std::string >( "A", true )
                                        , make_lit< std::string >( "A", false ) } )
             , false );

    test_case( make_or< std::string >( { make_lit< std::string >( "A", true )
                                       , make_lit< std::string >( "A", false ) } )
             , true );

    test_case( make_and< std::string >( 
                { make_or< std::string >( { make_lit< std::string >( "A", true )
                                          , make_lit< std::string >( "B", true ) } )
                , make_or< std::string >( { make_lit< std::string >( "A", false )
                                          , make_lit< std::string >( "B", true ) } )
                , make_or< std::string >( { make_lit< std::string >( "A", true )
                                          , make_lit< std::string >( "B", false ) } )
                , make_or< std::string >( { make_lit< std::string >( "A", false )
                                          , make_lit< std::string >( "B", false ) } ) } )
             , false );
}

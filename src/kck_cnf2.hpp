#include <variant>
#include <memory>
#include <vector>

#include <boost/bimap.hpp> 

namespace kck {

template < typename lit_t >
struct cnf_rose;

template < typename lit_t >
struct cnf_leaf;

template < typename lit_t >
using cnf_t = std::variant< cnf_rose< lit_t >, cnf_leaf< lit_t > >;
template < typename lit_t >
using cnf_ptr_t = std::unique_ptr< cnf_t< lit_t > >;

template < typename lit_t >
struct to_sat_cnf_state 
{
    boost::bimap< typename lit_t::var_t, int > mapping;
    int label_counter = 1;

    int get_int_var( const lit_t &lit ) 
    {
        auto it = mapping.left.find( lit.var );
        if ( it == mapping.left.end() )
        {
            int id = label_counter++;
            mapping.insert( { lit.var, id } );
            return id;
        } 
        return it->second;
    }
};

template < typename lit_t >
struct cnf_rose
{
    std::vector< cnf_t< lit_t > > children;

    cnf_rose< int > to_sat_cnf_go( to_sat_cnf_state< lit_t > &state )
    {
        std::vector< cnf_t< int > > res_children;
        for ( auto &child : children )
        {
            res_children.push_back( std::visit( [&]( auto &c ) 
            {
                return c.to_sat_cnf_go( state );
            }, child ) );
        }
        return { res_children }; 
    }
};

template < typename lit_t >
struct cnf_leaf
{
    std::vector< std::vector< lit_t > > clauses;

    cnf_leaf< int > to_sat_cnf_go( to_sat_cnf_state< lit_t > &state )
    {
        std::vector< std::vector< int > > res_cnf;

        for ( auto &clause : clauses )
        {
            std::vector< int > res_clause;
            for ( const auto &lit : clause )
                res_clause.push_back( state.get_int_var( lit ) );
            res_cnf.push_back( res_clause );
        }

        return { res_cnf }; 
    }
};

template < typename var_t_ >
struct literal
{
    using var_t = var_t_;

    var_t v;
    bool pos;
};

template < typename lit_t >
cnf_t< int > to_sat_cnf( cnf_t< lit_t > cnf )
{
    to_sat_cnf_state< lit_t > state;
    return std::visit( [&]( auto &c ) 
    { 
        return to_sat_cnf_go( state );
    }, cnf );
}

}

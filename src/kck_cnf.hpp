#pragma once

#include <boost/dynamic_bitset/dynamic_bitset.hpp>
#include <variant>
#include <memory>
#include <vector>

#include <boost/bimap.hpp> 

namespace kck {

template< typename var_t_ >
struct literal 
{
    using var_t = var_t_;

    var_t var;
    bool pos;

    literal( var_t var, bool pos ) : var( var ), pos( pos ) {}

    literal< var_t > operator-()
    {
        return { var, !pos };
    }
};

template < typename var_t >
std::ostream& operator<<( std::ostream& o, const literal< var_t >& l )
{
    if ( ! l.pos ) o << "! ";
    return o << l.var;
}

template < typename lit_t >
using cnf_clause_t = std::vector< lit_t >;

template < typename lit_t >
using cnf_t = std::vector< cnf_clause_t< lit_t > >;

template < typename lit_t >
struct cnf_rose;

template < typename lit_t >
struct cnf_rose_masked;

template < typename lit_t >
struct cnf_leaf;

template < typename lit_t >
using cnf_tree_t = std::variant< cnf_rose< lit_t >
                               , cnf_rose_masked< lit_t >
                               , cnf_leaf< lit_t > >;

using boost::bimap;

template < typename lit_t >
struct to_int_cnf_state 
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
    std::vector< cnf_tree_t< lit_t > > children;

    cnf_rose( std::vector< cnf_tree_t< lit_t > > children ) 
            : children( std::move( children ) )
    {}

    cnf_rose< int > to_int_cnf_go( to_int_cnf_state< lit_t > &state )
    {
        std::vector< cnf_tree_t< int > > res_children;
        for ( auto &child : children )
        {
            res_children.push_back( std::visit( [&]( auto &c ) 
            {
                return c.to_int_cnf_go( state );
            }, child ) );
        }
        return { res_children }; 
    }
};

template < typename lit_t >
struct cnf_rose_masked 
{
    std::vector< cnf_tree_t< lit_t > > children;
    std::shared_ptr< boost::dynamic_bitset<> > mask;

    cnf_rose_masked( std::vector< cnf_tree_t< lit_t > > children 
                   , std::shared_ptr< boost::dynamic_bitset<> > mask )
                   : children( std::move( children ) )
                   , mask( std::move( mask ) )
    {}

    cnf_rose_masked< int > to_int_cnf_go( to_int_cnf_state< lit_t > &state )
    {
        std::vector< cnf_t< int > > res_children;
        for ( auto &child : children )
        {
            res_children.push_back( std::visit( [&]( auto &c ) 
            {
                return c.to_int_cnf_go( state );
            }, child ) );
        }
        return cnf_rose_masked( children, mask );
    }
};

template < typename lit_t >
struct cnf_leaf
{
    cnf_t< lit_t > clauses;

    cnf_leaf( cnf_t< lit_t > clauses ) : clauses( clauses ) {};

    cnf_leaf< int > to_int_cnf_go( to_int_cnf_state< lit_t > &state )
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

template < typename lit_t >
std::pair< cnf_tree_t< int >, bimap< typename lit_t::var_t, int > > 
to_int_cnf( cnf_tree_t< lit_t > cnf )
{
    to_int_cnf_state< lit_t > state;
    return std::visit( [&]( auto &c ) 
    { 
        return to_int_cnf_go( state );
    }, cnf );
}

}

struct cnf_stats
{
    int clauses = 0;
    int var_count = 0;
    size_t max_clause_size = 0;
};

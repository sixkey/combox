#pragma once

#include "kck_str.hpp"
#include <boost/dynamic_bitset/dynamic_bitset.hpp>
#include <variant>
#include <memory>
#include <vector>

#include <boost/bimap.hpp>

namespace kck {

//// literal //////////////////////////////////////////////////////////////////

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
std::ostream& operator <<( std::ostream& o, const literal< var_t >& l )
{
    if ( ! l.pos ) o << "! ";
    return o << l.var;
}

//// cnf data /////////////////////////////////////////////////////////////////

template < typename lit_t >
using cnf_clause_t = std::vector< lit_t >;

template < typename lit_t >
std::ostream& operator <<( std::ostream& o, const cnf_clause_t< lit_t >& c )
{
    o << "[";
    for ( auto it = c.begin(); it < c.end(); it++ )
    {
        if ( it != c.begin() ) o << ", ";
        o << *it;
    }
    return o << "]";
}

template < typename lit_t >
using cnf_t = std::vector< cnf_clause_t< lit_t > >;

template < typename lit_t >
std::ostream& operator <<( std::ostream& o, const cnf_t< lit_t >& c )
{
    for ( auto it = c.begin(); it < c.end(); it++ )
    {
        if ( it != c.begin() ) o << "\n";
        o << *it;
    }
    return o;
}

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
struct cnf_rose
{
    std::vector< cnf_tree_t< lit_t > > children;

    cnf_rose() = default;

    cnf_rose( std::vector< cnf_tree_t< lit_t > > children )
            : children( std::move( children ) )
    {}

};

template < typename lit_t >
struct cnf_rose_masked
{
    std::vector< cnf_tree_t< lit_t > > children;
    std::shared_ptr< boost::dynamic_bitset<> > mask;

    cnf_rose_masked() = default;

    cnf_rose_masked( std::vector< cnf_tree_t< lit_t > > children
                   , std::shared_ptr< boost::dynamic_bitset<> > mask )
                   : children( std::move( children ) )
                   , mask( std::move( mask ) )
    {}
};

template < typename lit_t >
struct cnf_leaf
{
    cnf_t< lit_t > clauses;

    cnf_leaf( cnf_t< lit_t > clauses ) : clauses( clauses ) {};
};

//// to_int_cnf ///////////////////////////////////////////////////////////////

template < typename lit_t >
struct to_int_cnf_state
{
    boost::bimap< typename lit_t::var_t, int > mapping;
    int label_counter = 1;

    int get_int_var( const lit_t &lit )
    {
        auto it = mapping.left.find( lit.var );

        int var = 0;
        if ( it == mapping.left.end() )
        {
            var = label_counter++;
            mapping.insert( { lit.var, var } );
        }
        else
        {
            var = it->second;
        }
        return lit.pos ? var : -var;
    }
};


template < typename lit_t >
cnf_t< int > to_int_cnf_go( const cnf_t< lit_t > &cnf
                          , to_int_cnf_state< lit_t > &state )
{
    std::vector< std::vector< int > > res_cnf;

    for ( auto &clause : cnf )
    {
        std::vector< int > res_clause;
        for ( const auto &lit : clause )
            res_clause.push_back( state.get_int_var( lit ) );
        res_cnf.push_back( res_clause );
    }

    return res_cnf;
}

template < typename lit_t >
cnf_tree_t< int > to_int_cnf_go( const cnf_leaf< lit_t > &node
                               , to_int_cnf_state< lit_t > &state )
{
    return cnf_leaf< int >( to_int_cnf_go( node.clauses, state ) );
}

template < typename lit_t >
cnf_tree_t< int > to_int_cnf_go( const cnf_rose_masked< lit_t > &node
                               , to_int_cnf_state< lit_t > &state )
{
    std::vector< cnf_tree_t< int > > res_children;
    for ( auto &child : node.children )
        res_children.push_back( to_int_cnf_go( child, state ) );
    return cnf_rose_masked( std::move( res_children )
                          , std::move( node.mask ) );
}

template < typename lit_t >
cnf_tree_t< int > to_int_cnf_go( const cnf_rose< lit_t > &node
                               , to_int_cnf_state< lit_t > &state )
{
    std::vector< cnf_tree_t< int > > res_children;
    for ( auto &child : node.children )
        res_children.push_back( to_int_cnf_go( child, state ) );
    return cnf_rose( std::move( res_children ) );
}

template < typename lit_t >
cnf_tree_t< int > to_int_cnf_go( const cnf_tree_t< lit_t > &tree
                               , to_int_cnf_state< lit_t > &state )
{
    return std::visit( [&]( auto &c )
    {
        return to_int_cnf_go( c, state );
    }, tree );
}

template < typename lit_t >
std::pair< cnf_tree_t< int >, bimap< typename lit_t::var_t, int > >
to_int_cnf( const cnf_tree_t< lit_t > &cnf )
{
    to_int_cnf_state< lit_t > state;
    cnf_tree_t< int > res = std::visit( [&]( auto &c )
    {
        cnf_tree_t< int > r = { to_int_cnf_go( c, state ) };
        return r;
    }, cnf );

    return { res, state.mapping };
}

template < typename lit_t >
std::pair< cnf_t< int >, bimap< typename lit_t::var_t, int > >
to_int_cnf( const cnf_t< lit_t > &cnf )
{
    to_int_cnf_state< lit_t > state;
    auto res = to_int_cnf_go( cnf, state );
    return { res, state.mapping };
}

//// << ///////////////////////////////////////////////////////////////////////

template < typename lit_t >
std::ostream& show( std::ostream& os
                  , const cnf_tree_t< lit_t > &cnf
                  , bool active_only = false
                  , int depth = 0 );

template < typename lit_t >
std::ostream& show( std::ostream& os
                  , const cnf_leaf< lit_t > &cnf
                  , bool active_only = false
                  , int depth = 0 )
{
    return str_indent( os, depth * 2 ) << cnf.clauses << std::endl;
}

template < typename lit_t >
std::ostream& show( std::ostream& os
                  , const cnf_rose< lit_t > &cnf
                  , bool active_only = false
                  , int depth = 0 )
{
    os << "Rose" << std::endl;
    for ( const auto &c : cnf.children )
        show( os, c, active_only, depth + 1 );
    return os;
}

template < typename lit_t >
std::ostream& show( std::ostream& os
                  , const cnf_rose_masked< lit_t > &cnf
                  , bool active_only = false
                  , int depth = 0 )
{
    os << "Rose Masked" << std::endl;
    int counter = 0;
    for ( const auto &c : cnf.children ) {
        bool active = (*cnf.mask)[ counter ];
        if ( ! active_only || active )
            show( os << ( active ? "Y " : "N " ), c, active_only, depth + 1 );
        counter += 1;
    }
    return os;
}

template < typename lit_t >
std::ostream& show( std::ostream& os
                  , const cnf_tree_t< lit_t > &cnf
                  , bool active_only
                  , int depth )
{
    std::visit( [&]( const auto &c )
                {
                    show( os, c, active_only, depth );
                }
              , cnf );
    return os;
}

template < typename lit_t >
std::ostream& operator<<( std::ostream& os, const cnf_tree_t< lit_t >& c )
{
    return show( os, c );
}

//// stats ////////////////////////////////////////////////////////////////////

struct cnf_stats
{
    int clauses = 0;
    int var_count = 0;
    size_t max_clause_size = 0;
};

}

#pragma once

#include <set>

#include "cbx_sim.hpp"
#include "kck_log.hpp"
#include "kck_str.hpp"

namespace cbx {

using hyperedge_t = std::tuple< int, int, int >;

struct hypergraph_t
{
    int n;
    std::set< std::set< int > > edges;

    hypergraph_t( int n ) : n( n ) {}

    hypergraph_t( int n, std::set< std::set< int > > edges ) 
        : n( n )
        , edges( std::move( edges ) )
    {}
};

std::ostream& operator<<( std::ostream& os, const std::set< int > &e );

std::ostream& operator<<( std::ostream& os, const cbx::hypergraph_t& h );

template < typename hg_t >
using hg_pred = bool ( hg_t& );

template < typename hg_t >
using hg_coll = void ( hg_t& );

template < typename hg_t >
void trav_3hg_lat_go( hg_t &h 
                    , int edge_index
                    , bool test
                    , hg_pred< hg_t > break_fun
                    , hg_pred< hg_t > yield_fun
                    , hg_coll< hg_t > collect_fun )
{

    if ( test ) 
    {
        if ( break_fun( h ) )
            return;

        if ( yield_fun( h ) ) {
            collect_fun( h );
            return;
        }
    }

    if ( edge_index >= cbx::choose( h.n, 3 ) ) 
        return;

    h.remove_edge( edge_index );
    trav_3hg_lat_go< hg_t >( h
                           , edge_index + 1
                           , false
                           , break_fun
                           , yield_fun
                           , collect_fun );

    h.add_edge( edge_index );
    trav_3hg_lat_go< hg_t >( h
                           , edge_index + 1
                           , true
                           , break_fun
                           , yield_fun
                           , collect_fun );
    h.remove_edge( edge_index );
}

template < typename hg_t >
void trav_3hg_lat( hg_t &h
                 , hg_pred< hg_t > break_fun
                 , hg_pred< hg_t > yield_fun
                 , hg_coll< hg_t > collect_fun )
{
    trav_3hg_lat_go< hg_t >( h, 0, true, break_fun, yield_fun, collect_fun );
}

}

// TODO: - finish creating clauses
//       - one triangle will have more clauses, is bitmask really useful?
//       - it won't be compiled over and over again, but sat will be 
//         probably the bottleneck
//
// REM: - it may be hard to index the bitset as we are using combinations

#include "kck_str.hpp"
#include <Discreture/Combinations.hpp>
#include <boost/dynamic_bitset/dynamic_bitset.hpp>
#include <utility>
#include <vector>
#include <discreture.hpp>
#include <map>

#define SPDLOG_ACTIVE_LEVEL SPDLOG_LEVEL_DEBUG

#include "spdlog/spdlog.h"
#include "kck_log.hpp"

#include "cbx_3unihg.hpp"
#include "cbx_sim.hpp"
#include "cbx_utils.hpp"  

#include "main.hpp"

namespace kck {

std::ostream& operator<<( std::ostream& os, const std::vector< int > &v )
{
    os << "[";
    for ( std::size_t i = 0; i < v.size(); i++ )
    {
        os << v[ i ];
        if ( i < v.size() - 1 ) os << ", ";
    }
    return os << "]";
}

std::ostream& operator<<( std::ostream& os, const std::set< int > &s )
{
    os << "{";
    for ( auto it = s.begin(); it != s.end(); it++ )
    {
        if ( it != s.begin() ) os << ", ";
        os << *it;
    }
    return os << "}";
}

}

using namespace kck;

//// Application //////////////////////////////////////////////////////////////

lit_t labeler( int i ) { return { { 'X', { i } }, true }; }

formula_ptr< lit_t > llit( var_t v, bool pos )
{
    return forms::f_lit( { v, pos } );
}

var_t arc_color( int i, int j, int color )
{
    return { 'c', { i, j, color } };
}

cnf_t< lit_t > get_palette_cnf( palette_t palette
                              , int i, int j, int k
                              , cnf_builder< lit_t > &builder )
{
    assert( i < j && j < k );
    forms::or_t triangle_cond;
    for ( auto &p : palette )
        triangle_cond.push( forms::f_and( 
            { llit( arc_color( i, j, p[ 0 ] ), true )
            , llit( arc_color( j, k, p[ 1 ] ), true )
            , llit( arc_color( i, k, p[ 2 ] ), true ) } ) );
    //trace( "tria", i, j, k, triangle_cond );
    return triangle_cond.to_cnf( builder );
}

cnf_t< lit_t > get_coloring_cnf( int n, int colors )
{
    cnf_t< lit_t > cnf;
    for ( auto &&x : discreture::combinations( n, 2 ) )
    {
        int i = x[ 0 ], j = x[ 1 ];
        assert( i < j );

        // each edge gets at least one color
        cnf_clause_t< lit_t > clause;
        for ( int c = 0; c < colors; c++ ) 
            clause.push_back( { arc_color( i, j, c ), true } );
        cnf.push_back( clause );

        // each edge gets no more than two colors
        for ( auto &&cc : discreture::combinations( colors, 2 ) ) {
            if ( cc[ 0 ] == cc[ 1 ] ) continue;
            cnf.push_back( { { arc_color( i, j, cc[ 0 ] ), false } 
                           , { arc_color( i, j, cc[ 1 ] ), false } } );
        }
    }
    return cnf;
}

cnf_tree_t< lit_t > build_coloring_tree( int n, int colors
                                       , const palette_t &palette
                                       , std::shared_ptr< boost::dynamic_bitset<> > mask )
{
    std::vector< cnf_tree_t< lit_t > > formulas;

    formulas.push_back( cnf_leaf( get_coloring_cnf( n, colors ) ) );

    // Edge requirements 

    std::vector< cnf_tree_t< lit_t > > edge_forms;
    cnf_builder< lit_t > builder( labeler );
    for ( auto &&x : discreture::combinations( n, 3 ) )
    {
        int i = x[ 0 ], j = x[ 1 ], k = x[ 2 ];
        auto cnf = get_palette_cnf( palette, i, j, k, builder );
        edge_forms.push_back( cnf );
    }

    formulas.push_back( cnf_rose_masked< lit_t >( edge_forms, mask ) );

    return cnf_rose< lit_t >( std::move( formulas ) );
}

palette_t blue_palette = { { 1, 2, 3 }, { 4, 1, 5 }, { 6, 7, 1 } };

struct hypergraph_t 
{
    int n;
    std::set< std::set< int > > edges;
};

struct lat_hypergraph_t
{
    int n;
    std::shared_ptr< boost::dynamic_bitset<> > edge_set;

    int counter_graph_entered = 0;
    int counter_blue_colorable = 0;

    bimap< var_t, int > translation;

    cnf_tree_t< int > blue_translated;
    cnf_tree_t< lit_t > blue_orig;

    lat_hypergraph_t( int n ) 
        : n( n )
        , edge_set( std::make_shared< boost::dynamic_bitset<> >( cbx::choose( n, 3 ) ) )
    {
        blue_orig = build_coloring_tree( n, 7, blue_palette, edge_set );

        //trace( "blue", blue_orig );

        auto [ translated, translation ] = to_int_cnf( blue_orig );
        blue_translated = std::move( translated );
        this->translation = translation;
    }

    void add_edge( int index )
    {
        ( *edge_set )[ index ] = true;
    }

    void remove_edge( int index )
    {
        ( *edge_set )[ index ] = false;
    }

    hypergraph_t to_hypergraph() const
    {

        std::set< std::set< int > > edges;
        int counter = 0;
        for ( auto &x : discreture::combinations( n, 3 ) )
        {
            if ( ( *edge_set )[ counter ] ) 
                edges.insert( { x[ 0 ], x[ 1 ], x[ 2 ] } );
            counter++;
        }

        return { n, std::move( edges ) };
    }

};

std::ostream& operator<<( std::ostream& os, const hypergraph_t& h )
{
    os << "Hypergraph[" << h.n << "] {";
    for ( auto &e : h.edges )
        str_indent( os, 2 ) << e << std::endl;
    return os;
}

std::ostream& operator<<( std::ostream& os, const lat_hypergraph_t& h )
{
    return os << h.to_hypergraph();
}

bool is_perm_colorable( const lat_hypergraph_t &h, const std::vector< int > &perm )
{
    int n = h.n;

    std::vector< unsigned int > roles( h.n * h.n, 0 );

    int counter = 0;

    // This uses the fact, that the edge_set is created using the very same
    // discreture::combinations this is rather ugly and should be fixed.
    for ( auto &&p : discreture::combinations( n, 3 ) )
    {
        int i = p[ 0 ], j = p[ 1 ], k = p[ 2 ];

        if ( ( *h.edge_set )[ counter ] )
        {
            int i = perm[ p[ 0 ] ], j = perm[ p[ 1 ] ], k = perm[ p[ 2 ] ];
            cbx::sort_i( i, j, k );
            // The arc serves as left edge
            roles[ cbx::inc_i( n, i, j ) ] |= 1;
            // The arc serves as right edge
            roles[ cbx::inc_i( n, j, k ) ] |= 2;
            // The arc serves as top edge
            roles[ cbx::inc_i( n, i, k ) ] |= 4;
        }
        counter += 1;
    }

    for ( auto v : roles )
        // If any arc serves as both left, top, right
        if ( v == 7 ) 
            return false;
    return true;
}


bool is_r_uncolorable( lat_hypergraph_t &h )
{
    for ( auto &&perm : discreture::permutations( h.n ) )
        if ( is_perm_colorable( h, perm ) ) return false;
    return true;
}

using coloring_t = std::map< std::pair< int, int >, int >;

coloring_t get_coloring_solution( const lat_hypergraph_t &h, int colors, sat_solver_t &solver )
{
    coloring_t coloring;
    
    int n = h.n;

    const bimap< var_t, int > &translation = h.translation;

    for ( auto &a : discreture::combinations( n, 2 ) )
        for ( int c = 0; c < colors; c++ )
            if ( solver.val( translation.left.at( arc_color( a[ 0 ], a[ 1 ], c ) ) ) > 0 )
                coloring.insert( { { a[ 0 ], a[ 1 ] }, c } );

    return std::move( coloring );
}

void print_graph( lat_hypergraph_t &h )
{
    std::cout << h.to_hypergraph() << std::endl;

    sat_solver_t solver;
    add_cnf( solver, h.blue_translated );

    int res = solver.solve();

    assert( res == SAT_Y );

    int n = h.n;
    
    coloring_t coloring = get_coloring_solution( h, 7, solver );

    //show( std::cout, h.blue_orig, true );

    for ( const auto &[ key, item ] : coloring ) 
    {
        std::cout << key << "->" << item << std::endl;
    }
}

bool is_b_uncolorable( lat_hypergraph_t &h )
{
    if ( h.counter_graph_entered % 10000 == 0 )
    {
        trace( "sts", h.counter_graph_entered, h.counter_blue_colorable );
    }
    h.counter_graph_entered++;

    //trace( "edges", *h.edge_set );

    sat_solver_t solver;
    add_cnf( solver, h.blue_translated );
    int res = solver.solve();
    if ( res == SAT_U ) throw std::runtime_error( "sat does not know" );
    
    if ( res == SAT_Y )
        h.counter_blue_colorable ++;

    return res == SAT_N; 
}

int main( int arc, char** argv )
{
    int n = 7;

    assert( cbx::choose( 5, 3 ) == 10 );
    assert( cbx::choose( 7, 2 ) == 21 );

    lat_hypergraph_t h( n );

    cbx::trav_3hg_lat( h
                     , is_b_uncolorable
                     , is_r_uncolorable
                     , print_graph );

    trace( "count", "graphs visited", h.counter_graph_entered );
    trace( "count", "graphs blue colorable", h.counter_blue_colorable );

}

// TODO: - finish creating clauses
//       - one triangle will have more clauses, is bitmask really useful?
//       - it won't be compiled over and over again, but sat will be 
//         probably the bottleneck

#include <Discreture/Combinations.hpp>
#include <boost/dynamic_bitset/dynamic_bitset.hpp>
#include <utility>
#include <vector>
#include <discreture.hpp>

#include "kck_cnf.hpp"
#include "kck_form.hpp" 
#include "kck_sat.hpp"

using namespace kck;

using pattern = std::array< int, 3 >;
using palette_t = std::vector< pattern >;

using var_t = std::pair< char, std::vector< int > >;
using lit_t = literal< var_t >;
using nlit_t = node_literal< lit_t >;
using and_t = node_and< lit_t >;
using or_t = node_or< lit_t >;
using form_t = formula_ptr< lit_t >;

using discreture::operator<<;

std::ostream& operator<<( std::ostream& os, std::vector< int > v )
{
    os << "[";
    for ( int i = 0; i < v.size(); i++ )
    {
        os << v[ i ];
        if ( i < v.size() - 1 ) os << ", ";
    }
    return os << "]";
}

std::ostream& operator<<( std::ostream& os, var_t var )
{
    return os << "(" << var.first << "," << var.second << ")";
}

constexpr int choose( int n, int k ) 
{
    int nom = 1; 
    int den = 1;
    for ( int i = 0; i < k; i++ )
    {
        nom *= n--;
        den *= k--;
    }
    return nom / den;
}



//// 3 Uniform Hypergraph Lattice Explorer ////////////////////////////////////

template < typename hg_t >
using hg_pred = bool *( const hg_t& );

template < typename hg_t >
using hg_coll = void *( const hg_t& );

template < typename hg_t, int n > 
void trav_3hg_lat_go( hg_t &h 
                    , int edge_index
                    , hg_pred< hg_t > break_fun
                    , hg_pred< hg_t > yield_fun
                    , hg_pred< hg_t > collect_fun )
{
    if ( edge_index > choose( n, 3 ) )
        return;

    if ( break_fun( h ) )
        return;

    if ( yield_fun( h ) ) {
        collect_fun( h );
        return;
    }
    trav_3hg_lat_go< hg_t, n >( h, edge_index + 1 );
    h.add_edge( edge_index );
    trav_3hg_lat_go< hg_t, n >( h, edge_index + 1 );
    h.remove_edge( edge_index );
}

template < typename hg_t, int n >
void trav_3hg_lat( hg_pred< hg_t > break_fun
                 , hg_pred< hg_t > yield_fun
                 , hg_pred< hg_t > collect_fun )
{
    hg_t h;
    trav_3hg_lat_go< hg_t, n >( h, 0, break_fun, yield_fun, collect_fun );
}

//// Application //////////////////////////////////////////////////////////////

lit_t labeler( int i ) { return { { 'X', { i } }, true }; }

formula_ptr< lit_t > llit( char c, std::vector< int > indcs, bool pos )
{
    return make_lit< lit_t >( { { c, indcs }, pos } );
}

cnf_t< lit_t > get_palette_cnf( int n, int colors, palette_t palette
                              , int i, int j, int k, cnf_builder< lit_t > &builder )
{
    or_t triangle_cond;
    for ( auto &p : palette )
        triangle_cond.push( make_and< lit_t >( 
            { llit( 'c', { i, j, p[ 0 ] }, true )
            , llit( 'c', { i, j, p[ 0 ] }, true )
            , llit( 'c', { i, j, p[ 0 ] }, true ) } ) );
    return triangle_cond.to_cnf( builder );
}

cnf_t< lit_t > get_coloring_cnf( int n, int colors )
{
    cnf_t< lit_t > cnf;
    for ( auto &&x : discreture::combinations( n, 2 ) )
    {
        int i = x[ 0 ], j = x[ 1 ];

        // each edge gets at least one color
        cnf_clause_t< lit_t > clause;
        for ( int c = 0; c < colors; c++ ) 
            clause.push_back( { { 'c', { i, j, c } }, true } );
        cnf.push_back( clause );

        // each edge gets no more than two colors
        for ( auto &&cc : discreture::combinations( colors, 2 ) ) {
            if ( cc[ 0 ] == cc[ 1 ] ) continue;
            cnf.push_back( { { { 'c', { i, j, cc[ 0 ] } }, false } 
                           , { { 'c', { i, j, cc[ 1 ] } }, false } } );
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


    std::vector< cnf_tree_t< lit_t > > edge_forms;
    cnf_builder< lit_t > builder( labeler );
    for ( auto &&x : discreture::combinations( n, 3 ) )
    {
        int i = x[ 0 ], j = x[ 1 ], k = x[ 2 ];
        edge_forms.push_back( get_palette_cnf( n, colors, palette, i, j, k, builder) );
    }

    formulas.push_back( cnf_rose_masked< lit_t >( edge_forms, mask ) );

    return cnf_rose< lit_t >( formulas );
}

template < int n >
using edge_set_t = std::bitset< choose( n, 3 ) >;

palette_t blue_palette = { { 1, 2, 3 }, { 4, 1, 5 }, { 6, 7, 1 } };

template < int vertex_count > 
struct hypergraph_t
{
    int n = vertex_count;
    std::shared_ptr< edge_set_t< vertex_count > > edge_set;

    cnf_tree_t< lit_t > blue_coloring;

    hypergraph_t() : edge_set( choose( n, 3 ) )
                   , blue_coloring( build_coloring_tree( n, 7, blue_palette, edge_set ) )
    {
    }

    void add_edge( int index )
    {
        edge_set[ index ] = true;
    }

    void remove_edge( int index )
    {
        edge_set[ index ] = false;
    }
};



template < int n >
bool is_r_colorable( hypergraph_t< n > h )
{
}


int main( int arc, char** argv )
{
}

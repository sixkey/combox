// TODO: - finish creating clauses
//       - one triangle will have more clauses, is bitmask really useful?
//       - it won't be compiled over and over again, but sat will be 
//         probably the bottleneck


#include <boost/dynamic_bitset/dynamic_bitset.hpp>
#include <utility>
#include <vector>
#include <discreture.hpp>

#include "kck_form.hpp" 
#include "kck_sat.hpp"

using namespace kck;

using pattern = std::array< int, 3 >;
using palette_t = std::vector< pattern >;

using var_t = std::pair< char, std::vector< int > >;
using lit_t = node_literal< var_t >;
using and_t = node_and< var_t >;
using or_t = node_or< var_t >;
using form_t = formula_ptr< var_t >;

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

struct hypergraph_formula
{
    int n = 0;

    hypergraph_formula( int n ) : n( n ) {};

    var_t arc_col_var( int i, int j, int c )
    {
        assert( i < j );
        return { 'l', { i, j, c } };
    }

    var_t edge_in_var( int i, int j, int k )
    {
        assert( i < j && j < k );
        return { 'e', { i, j, k } };
    }

    form_t graph_is_colored( int colors )
    {
        // Arc get exacly one color
        and_t colored;
        for ( auto &&x : discreture::combinations( n, 2 ) )
        {
            int i = x[ 0 ], j = x[ 1 ]; 

            // Each arc gets one color 
            or_t color_clauses;
            for( int c = 0; c < colors; c++ )
                color_clauses.push( make_lit( arc_col_var( i, j, c ) ) );
            colored.push( std::move( color_clauses ) );

            // No arc receives two colors
            for( auto && cc : discreture::combinations( colors , 2 ) )
            {
                if ( cc[ 0 ] == cc[ 1 ] ) continue;
                colored.push( make_or< var_t >( 
                    { make_lit( arc_col_var( i, j, cc[ 0 ] ), false )
                    , make_lit( arc_col_var( i, j, cc[ 1 ] ), false ) } ) );
            }
        }
        return std::make_shared< and_t >( colored );
    }

    form_t coloring_respects_palette( int colors, palette_t palette )
    {
        and_t triangles_valid;
        for ( auto && x : discreture::combinations( n, 3 ) )
        {
            // every triangle has one of okay patterns
            int i = x[ 0 ], j = x[ 1 ], k = x[ 2 ];

            or_t triangle_cond;
            for ( auto &p : palette )
                triangle_cond.push( make_and< var_t >( 
                    { make_lit( arc_col_var( i, j, p[ 0 ] ) )
                    , make_lit( arc_col_var( j, k, p[ 1 ] ) )
                    , make_lit( arc_col_var( i, k, p[ 2 ] ) ) } ) );
            // e_ijk => triangle needs to be correctly colored
            //triangles_valid.push( 
                //make_or< var_t >( { make_lit( edge_in_var( i, j, k ), false ) 
                                  //, std::make_shared< or_t >( triangle_cond ) } ) );
            triangles_valid.push( triangle_cond );
        }
        return std::make_shared< and_t >( triangles_valid );
    }

    form_t coloring_formula( int colors, palette_t palette )
    { 
        and_t formula;

        formula.push( graph_is_colored( colors ) );
        formula.push( coloring_respects_palette( colors, palette ) );

        return std::make_shared< and_t >( formula );
    }
};


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

template < int n >
using edge_set_t = std::bitset< choose( n, 3 ) >;


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

var_t labeler( int i ) { return { 'X', { i } }; }

cnf_t< var_t > get_palette_cnf( int n, int colors, palette_t palette
                              , int i, int j, int k )
{
    or_t triangle_cond;
    for ( auto &p : palette )
        triangle_cond.push( make_and< var_t >( 
            { make_lit< var_t >( { 'c', { i, j, p[ 0 ] } }, true )
            , make_lit< var_t >( { 'c', { i, j, p[ 0 ] } }, true )
            , make_lit< var_t >( { 'c', { i, j, p[ 0 ] } }, true ) } ) );
    return triangle_cond.to_cnf( labeler );
}

cnf_t< var_t > get_palette_cnf( int n, int colors, palette_t palette )
{
    and_t triangles_valid;
    for ( auto && x : discreture::combinations( n, 3 ) )
    {
        // every triangle has one of okay patterns
        int i = x[ 0 ], j = x[ 1 ], k = x[ 2 ];

        or_t triangle_cond;
        for ( auto &p : palette )
            triangle_cond.push( make_and< var_t >( 
                { make_lit< var_t >( { 'c', { i, j, p[ 0 ] } }, true )
                , make_lit< var_t >( { 'c', { i, j, p[ 0 ] } }, true )
                , make_lit< var_t >( { 'c', { i, j, p[ 0 ] } }, true ) } ) );
        triangles_valid.push( triangle_cond );
    }

    return triangles_valid.to_cnf( labeler ); 
}

cnf_t< var_t > get_coloring_cnf( int n, int colors )
{
    cnf_t< var_t > cnf;
    for ( auto &&x : discreture::combinations( n, 2 ) )
    {
        int i = x[ 0 ], j = x[ 1 ];

        // each edge gets at least one color
        cnf_clause_t< var_t > clause;
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

template < int vertex_count > 
struct hypergraph_t
{
    int n = vertex_count;
    edge_set_t< vertex_count > edge_set;

    cnf_comp_mask< var_t > red_coloring;
    cnf_comp_mask< var_t > blue_coloring;

    hypergraph_t()
    {
        cnf_lab_t< var_t > blue_cnf;
        cnf_lab_t< var_t > red_cnf;

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

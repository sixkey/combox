#include "../inc/cadical.hpp"
#include <Discreture/Combinations.hpp>
#include <utility>
#include <vector>
#include <discreture.hpp>
#include "to_cnf.hpp" 
#include "sat.hpp"

using pattern = std::array< int, 3 >;
using palette_t = std::vector< pattern >;

using var_t = std::pair< char, std::vector< int > >;
using lit_t = node_literal< var_t >;
using and_t = node_and< var_t >;
using or_t = node_or< var_t >;
using form_t = form_ptr< var_t >;

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

struct hypergraph 
{
    int n = 0;

    hypergraph( int n ) : n( n ) {};

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

    form_t no_perm_triple_arc()
    {
            
    }

    form_t perm_triple_arc( std::vector< int > permutation )
    {
         
    }

    
};


void test() 
{
    int n = 6;
    int colors = 2;

    hypergraph h( n );
    palette_t palette = { { 0, 0, 0 }
                        , { 0, 0, 1 }
                        , { 0, 1, 0 }
                        , { 0, 1, 1 }
                        , { 1, 0, 0 }
                        , { 1, 0, 1 }
                        , { 1, 1, 0 } };

    form_t coloring_formula = h.coloring_formula( colors, palette );
    auto res = sat_solve( coloring_formula );  

    std::cout << coloring_formula->to_string() << std::endl;

    std::cout << res.res << std::endl;


    for ( auto &&x : discreture::combinations( n, 3 ) )
    {
        std::cout << x << "->" << res.val( h.edge_in_var( x[ 0 ], x[ 1 ], x[ 2 ] ) ) << std::endl;
    }

    for ( auto &&x : discreture::combinations( n, 2 ) )
    {
        for ( int c = 0; c < colors; c++ ) {
            std::cout << x[ 0 ] << "-" << x[ 1 ] << ":" << c << "->" << res.val( h.arc_col_var( x[ 0 ], x[ 1 ], c ) ) << std::endl;
        }
    }

}

int main( int arc, char** argv )
{
    //test();

    int n = 8;
    int colors = 3;
    palette_t palette = { { 0, 1, 2 }, { 3, 0, 4 }, { 5, 6, 0 } };

    hypergraph h( n );
    std::cout << "Generating formula" << std::endl;
    form_t coloring_formula = h.coloring_formula( colors, palette );

    std::cout << "Satting" << std::endl;
    auto res = sat_solve( coloring_formula );  
    std::cout << res.res << std::endl;


}

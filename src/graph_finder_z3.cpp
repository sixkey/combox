#include <Discreture/Combinations.hpp>
#include <algorithm>
#include <string>
#include <vector>
#include <map>

#include <discreture.hpp>
#include <z3++.h>
#include <z3_api.h>

#include "cbx_3unihg.hpp"
#include "kck_log.hpp"

#include "cbx_turan.hpp"
#include "cbx_utils.hpp"


std::string edge_var( std::string graph_name, int i, int j, int k )
{
    return graph_name
          + "e_" + std::to_string( i )
          + "," + std::to_string( j )
          + "," + std::to_string( k );
}

struct graph
{
    std::string name;

    int n;

    z3::expr_vector edges;

    std::map< std::tuple< int, int, int >, int > edge_indices;

    z3::expr edge( int i, int j, int k )
    {
        assert( i < j && j < k ); return edges[ edge_indices.at( { i, j, k } ) ];
    }

    z3::expr edge( const std::vector< int >& e )
    {
        assert( e.size() == 3 );
        return edge( e[ 0 ], e[ 1 ], e[ 2 ] );
    }

    graph( z3::context &c, int n, std::string name )
        : n( n )
        , name( name )
        , edges( c )
    {
        int counter = 0;
        for ( auto &x : discreture::combinations( n, 3 ) ) {
            edge_indices.insert( { { x[ 0 ], x[ 1 ], x[ 2 ] }
                                 , counter } );
            edges.push_back( c.bool_const( edge_var( name, x[ 0 ], x[ 1 ], x[ 2 ] ).c_str() ) );
            counter++;
        }
    }

    cbx::hypergraph_t read( z3::model m )
    {
        cbx::hypergraph_t res( n );
        for ( auto &x : discreture::combinations( n, 3 ) )
            if ( m.eval( edge( x[ 0 ], x[ 1 ], x[ 2 ] ) ).is_true() )
                res.edges.insert( { x[ 0 ], x[ 1 ], x[ 2 ] } );
        return res;
    }

    z3::expr_vector &vars()
    {
        return edges;
    }

};


std::ostream& operator<<( std::ostream &os, const std::pair< int, int > &p )
{
    return os << "(" << p.first << ", " << p.second << ")";
}
std::ostream& operator<<( std::ostream &os
                        , const std::map< std::pair< int, int >, int >& col  )
{
    os << "{\n";
    for ( auto &[ k, v ] : col )
        os << k << " -> " << v << "\n";
    return os << "}";
}

char const * const color_names[ 16 ] = 
    { "c_0"
    , "c_1"
    , "c_2"
    , "c_3"
    , "c_4"
    , "c_5"
    , "c_6"
    , "c_7"
    , "c_8"
    , "c_9"
    , "c_10"
    , "c_11"
    , "c_12"
    , "c_13"
    , "c_14"
    , "c_15" };

struct int_sort 
{
    int n; 

    int_sort( z3::context &c
             , int n
             , std::string name ) : n( n ) 
    {
    }

    z3::expr val( z3::context &c, int i )
    {
        return c.int_val( i );
    }

    z3::expr mk_const( z3::context &c, const std::string &s )
    {
        return c.int_const( s.c_str() );
    }
};

struct enum_sort 
{
    int n;
    std::string name;

    z3::func_decl_vector enum_consts;
    z3::func_decl_vector enum_testers;
    z3::sort sort;

    enum_sort( z3::context &c
             , int n
             , std::string name )
             : n( n )
             , name( name )
             , enum_consts( c )
             , enum_testers( c )
             , sort( c.enumeration_sort( name.c_str()
                                       , n
                                       , color_names
                                       , enum_consts
                                       , enum_testers ) )
    {
        assert( n <= 16 );
    }

    z3::expr val( z3::context &c, unsigned int i )
    {
        assert( i <= 16 );
        return enum_consts[ i ]();
    }

    z3::expr mk_const( z3::context &c, const std::string &s )
    {
        return c.constant( s.c_str(), sort );
    }
};

template < typename color_sort_t >
struct coloring
{

    z3::expr_vector colors;

    std::map< std::pair< int, int >, int > color_indices;
    int n;
    std::string name_;

    color_sort_t color_sort;

    coloring( z3::context &c, int n, int no_colors, std::string name_ ) 
        : n( n )
        , colors( c )
        , name_( name_ )
        , color_sort( c, no_colors, name_ )
    {
        int counter = 0;
        for ( auto &x : discreture::combinations( n, 2 ) ) {
            color_indices.insert( { { x[ 0 ], x[ 1 ] }, counter } );
            colors.push_back(
                color_sort.mk_const( c, color_var( x[ 0 ], x[ 1 ] ) )
            );
            counter++;
        }
    }

    std::string color_var( int i, int j )
    {
        return name_ + "_c_" + std::to_string( i ) + "," + std::to_string( j );
    }

    z3::expr color( int i, int j )
    {
        assert( i < j );
        return colors[ color_indices.at( { i, j } ) ];
    }

    z3::expr respects_palette( z3::context &c
                             , cbx::palette_t &palette )
    {
        z3::expr_vector triangle_colorings( c );
        for ( auto &e : discreture::combinations( n, 3 ) )
        {
            int i = e[ 0 ], j = e[ 1 ], k = e[ 2 ];
            z3::expr_vector triangle_coloring( c );
            for ( auto &p : palette )
                triangle_coloring.push_back( 
                    ( color( i, j ) == color_sort.val( c, p[ 0 ] )
                   && color( j, k ) == color_sort.val( c, p[ 1 ] )
                   && color( i, k ) == color_sort.val( c, p[ 2 ] ) ).simplify() );
            triangle_colorings.push_back( z3::mk_or( triangle_coloring ) );
        }
        return z3::mk_and( triangle_colorings );
    }

    // TODO: There has to be a nicer decomposition.
    z3::expr respects_palette( z3::context &c
                             , graph &g 
                             , cbx::palette_t &palette )

    {
        z3::expr_vector triangle_colorings( c );
        for ( auto &e : discreture::combinations( n, 3 ) )
        {
            int i = e[ 0 ], j = e[ 1 ], k = e[ 2 ];
            z3::expr_vector triangle_coloring( c );
            for ( auto &p : palette )
                triangle_coloring.push_back( 
                    ( color( i, j ) == color_sort.val( c, p[ 0 ] )
                   && color( j, k ) == color_sort.val( c, p[ 1 ] )
                   && color( i, k ) == color_sort.val( c, p[ 2 ] ) ).simplify() );
            triangle_colorings.push_back( 
                z3::implies( g.edge( i, j, k )
                           , z3::mk_or( triangle_coloring ) ) 
            );
        }
        return z3::mk_and( triangle_colorings );

    }

    std::map< std::pair< int, int >, int > read( const z3::model &m )
    {
        std::map< std::pair< int, int >, int > res;
        for ( auto &a : discreture::combinations( n, 2 ) )
            res.insert( { { a[ 0 ], a[ 1 ] }
                        , m.eval( color( a[ 0 ], a[ 1 ] ) ).get_numeral_int() } );
        return res;
    }

    z3::expr_vector &vars()
    {
        return colors;
    }
};

z3::expr one_of( z3::context &c, z3::expr_vector &v )
{
    z3::expr_vector combs( c );
    for ( auto &c : discreture::combinations( v.size(), 2 ) )
        combs.push_back( ! v[ c[ 0 ] ] || !v[ c[ 1 ] ] );
    return z3::mk_or( v ) && z3::mk_and( combs );
}

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

struct permutation
{

    // Row major matrix.
    z3::expr_vector links;

    int n;

    std::string name_;

    std::string name() const
    {
        return "p[" + std::to_string( n ) + ", " + name_ + "]";
    }

    z3::expr link( int i, int j )
    {
        return links[ n * i + j ];
    }

    std::string link_var( int i, int j )
    {
        return name() + "-l_" + std::to_string( i ) + "," + std::to_string( j );
    }

    permutation( z3::context &c, int n, std::string name )
        : n( n ), name_( std::move( name ) ), links( c )
    {
        for ( int i = 0; i < n; i++ )
            for ( int j = 0; j < n; j++ )
                links.push_back( c.bool_const( link_var( i, j ).c_str() ) );
    }

    z3::expr maps_to( z3::context &c
                    , const std::vector< int > &a
                    , const std::vector< int > &b )
    {
        assert( a.size() == b.size() );

        z3::expr_vector any_perm( c );
        for ( auto &&p : discreture::permutations( ( int ) a.size() ) )
        {
            z3::expr_vector one_perm( c );
            for ( int i = 0; i < a.size(); i++ )
                one_perm.push_back( link( a[ i ], b[ p[ i ] ] ) );
            any_perm.push_back( z3::mk_and( one_perm ) );
        }
        return z3::mk_or( any_perm );
    }

    z3::expr formula( z3::context &c )
    {
        z3::expr_vector perm_form( c );
        for ( int i = 0; i < n; i++ )
        {
            z3::expr_vector il( c );
            z3::expr_vector ir( c );
            for ( int j = 0; j < n; j++ )
            {
                il.push_back( link( i, j ) );
                ir.push_back( link( j, i ) );
            }
            perm_form.push_back( one_of( c, il ) );
            perm_form.push_back( one_of( c, ir ) );
        }
        return z3::mk_and( perm_form );
    }

    std::vector< int > read( z3::model m )
    {
        std::vector< int > res;
        for ( int i = 0; i < n; i++ )
            for ( int j = 0; j < n; j++ )
                if ( m.eval( link( i, j ) ).is_true() )
                {
                    res.push_back( j );
                    break;
                }
        return res;
    }

    z3::expr_vector &vars()
    {
        return links;
    }
};

z3::expr mk_eq( z3::context &c, z3::expr a, z3::expr b )
{
    return z3::to_expr( c, Z3_mk_eq( c, a, b ) );
}

struct graph_iso
{
    int n;
    permutation p;
    std::string name;

    graph_iso( z3::context &c, int n, std::string name )
        : n( n ), name( name ), p( c, n, name + "_p" )
    {}

    z3::expr iso( z3::context &c
                , graph &g 
                , graph &h )
    {
        assert( g.n == h.n );
        z3::expr_vector form( c );

        for ( auto &x : discreture::combinations( g.n, 3 ) )
        {
            for ( auto &y : discreture::combinations( g.n, 3 ) )
            {
                auto atom = z3::implies( p.maps_to( c, x, y )
                                       , ( g.edge( x ) == h.edge( y ) ) );
                form.push_back( atom );
            }
        }
        return p.formula( c ) && z3::mk_and( form );
    }

    z3::expr_vector &vars()
    {
        return p.vars();
    }
};

void test_perm()
{
    z3::context c;
    z3::solver s( c );

    permutation p( c, 5, "p" );
    s.add( p.formula( c ) );
    s.add( p.maps_to( c, { 0, 1, 2 }, { 0, 1, 2 } ) );
    s.add( p.maps_to( c, { 2, 3 }, { 1, 4 } ) );

    bool sat = s.check();
    z3::model m = s.get_model();

    assert( sat );

    std::vector< int > perm = p.read( m );
    assert( perm[ 2 ] == 1 );
    assert( perm[ 3 ] == 4 );
    assert( perm[ 0 ] == 0 || perm[ 0 ] == 2 );
    assert( perm[ 0 ] == 2 || perm[ 0 ] == 0 );
    assert( perm[ 4 ] == 3 );
}

void test_perm_2()
{
    z3::context c;
    z3::solver s( c );

    permutation p( c, 5, "p" );
    s.add( p.formula( c ) );
    s.add( p.maps_to( c, { 0, 1, 2 }, { 0, 1, 2 } ) );
    s.add( p.maps_to( c, { 2, 3 }, { 3, 4 } ) );

    bool sat = s.check();
    assert( ! sat );
}

void test_graph()
{
    z3::context c;
    z3::solver s( c );

    graph g( c, 4, "n" );


    z3::expr_vector v( c );
    for ( auto &x : discreture::combinations( g.n, 3 ) )
        v.push_back( g.edge( x ) );
    s.add( mk_or( v ) );
    s.add( ! g.edge( 0, 1, 2 ) );

    assert( s.check() );
    auto model = s.get_model();

    auto model_g = g.read( model );

    assert( model_g.edges.find( { 0, 1, 2 } ) == model_g.edges.end() );

    bool has_edge = false;
    for ( auto &x : discreture::combinations( g.n, 3 ) )
        has_edge |= model_g.edges.find( { x[ 0 ], x[ 1 ], x[ 2 ] } ) != model_g.edges.end();
    assert( has_edge );
}

void test_iso()
{
    z3::context c;
    z3::solver s( c );

    int n = 4;

    graph g( c, n, "g" );
    graph h( c, n, "h" );

    auto f = graph_iso( c, n, "phi" );

    s.add( f.iso( c, g, h ) );
    s.add( g.edge( 0, 1, 2 ) );
    s.add( f.p.maps_to( c, { 0, 1, 2 }, { 1, 2, 3 } ) );
    s.add( ! h.edge( 1, 2, 3 ) );

    auto res = s.check();
    assert( ! res );
}

void test_coloring_5()
{
    z3::context c;
    z3::solver s( c );

    coloring< int_sort > col( c, 5, 2, "col" );
    cbx::palette_t palette = { { 0, 0, 1 }
                             , { 0, 1, 0 }
                             , { 0, 1, 1 }
                             , { 1, 0, 0 }
                             , { 1, 0, 1 }
                             , { 1, 1, 0 } };
    s.add( col.respects_palette( c, palette ) );

    // This should be possible, because there is K_3-nonmonochromatic coloring
    // of K_5.
    assert( s.check() );
}

void test_coloring_6()
{
    z3::context c;
    z3::solver s( c );

    std::shared_ptr< graph > g = std::make_shared< graph >( c, 6, "g" );
    coloring< int_sort > col( c, 6, 2, "col" );
    cbx::palette_t palette = { { 0, 0, 1 }
                             , { 0, 1, 0 }
                             , { 0, 1, 1 }
                             , { 1, 0, 0 }
                             , { 1, 0, 1 }
                             , { 1, 1, 0 } };
    s.add( col.respects_palette( c, palette ) );

    // This should not be possible, because of Ramsey.
    assert( ! s.check() );
}


void tests()
{
    test_perm();
    test_perm_2();
    test_graph();
    test_iso();
    test_coloring_5();
    test_coloring_6();
}


int main( int arc, char** argv )
{

    tests();

    int n = std::stoi( argv[ 1 ] );

    cbx::palette_t blue_palette = { { 0, 1, 2 }
                                  , { 3, 0, 4 }
                                  , { 5, 6, 0 } };

    // | a a b |
    // | b c c |
    cbx::palette_t red_palette = { { 0, 0, 1 }
                                 , { 0, 0, 2 }
                                 , { 0, 2, 1 }
                                 , { 0, 2, 2 }
                                 , { 1, 0, 1 }
                                 , { 1, 0, 2 }
                                 , { 1, 2, 1 }
                                 , { 1, 2, 2 } };

    z3::context c;
    z3::solver s = ( z3::tactic( c, "simplify" )
                   & z3::tactic( c, "dt2bv" )
                   & z3::tactic( c, "bit-blast" )
                   & z3::tactic( c, "qsat" ) ).mk_solver();

    graph g( c, n, "g" );
    graph h( c, n, "h" );
    coloring< enum_sort > blue_coloring( c, n, 7, "blue" );
    coloring< enum_sort > red_coloring( c, n, 3, "red" );
    graph_iso f( c, n, "phi" );

    // There exists a g-coloring such that it respects blue palette.
    auto blue_formula =
            blue_coloring.respects_palette( c, g, blue_palette );
    s.add( blue_formula );

    // for all h, such that they are iso to f, there is no red coloring

    // g and h have to be iso

    // For all h : hyper( n, 3 ) . h ~= g =>
    auto red_formula =
        z3::forall(
            h.vars()
            , ( z3::forall( f.vars()
                          , ! f.iso( c, g, h ) )
             || z3::forall( red_coloring.vars()
                          , ! red_coloring.respects_palette( c
                                                           , h
                                                           , red_palette ) ) )
        );

    kck::trace( "f_red", red_formula );

    s.add( red_formula );

    bool sat = s.check();
    kck::trace( "sat", sat );

    kck::trace( "stats", s.statistics() );

    if ( sat )
    {
        auto model = s.get_model();
        kck::trace( "g", g.read( model ) );
        kck::trace( "h", h.read( model ) );
    }

}

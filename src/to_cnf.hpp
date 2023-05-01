#pragma once

#include <cstddef>
#include <memory>
#include <vector> 
#include <boost/bimap.hpp>

using clause = std::vector< int >;
using cnf_t = std::vector< clause >;

template < typename var_t >
using var_to_id_t = boost::bimap< var_t, int >;

struct cnf_stats
{
    int clauses = 0;
    int var_count = 0;
    size_t max_clause_size = 0;
};

cnf_stats cnf_get_stats( cnf_t cnf );

std::ostream& operator <<( std::ostream& os, cnf_stats s );

template < typename var_t >
struct cnf_builder
{
    cnf_t output;

    // Nodes of the formula get odd indeces
    int node_counter = 1;
    // Literals get even indeces 
    int lit_counter = 2;

    var_to_id_t< var_t > var_to_id;

    int get_node_id() 
    { 
        int res = node_counter;
        node_counter += 2;
        return res;
    }

    int get_var_id( var_t literal )   
    {
        auto it = var_to_id.left.find( literal );
        if ( it == var_to_id.left.end() ) {
            int lit_id = lit_counter;
            lit_counter += 2;
            var_to_id.insert( { literal, lit_id } );
            return lit_id;
        }
        return it->second;
    }

    void push( std::vector< int > v ) { output.push_back( std::move( v ) ); };
};

template < typename var_t >
struct form 
{
    virtual int to_cnf_go( cnf_builder< var_t >& builder ) = 0;
    virtual std::ostream &to_string_go( std::ostream &ss, int level ) = 0;

    std::pair< cnf_t, var_to_id_t< var_t > > to_cnf()
    {
        cnf_builder< var_t > builder;
        int top_id = this->to_cnf_go( builder );
        builder.push( { top_id } );
        return { std::move( builder.output )
               , std::move( builder.var_to_id ) }; 
    }

    std::string to_string() 
    {
        std::stringstream ss;
        this->to_string_go( ss, 0 );
        return ss.str();
    }
};

template < typename var_t > 
using form_ptr = std::shared_ptr< form< var_t > >;

std::ostream& str_indent( std::ostream& ss, int level );

//// Not ////

template < typename var_t >
struct node_not : public form< var_t > 
{
    form_ptr< var_t > child;

    node_not( form_ptr< var_t > child ) 
        : child( child ){};

    int to_cnf_go( cnf_builder< var_t >& builder ) override 
    {
        int node_id = builder.get_node_id();
        int child_id = child->to_cnf_go( builder );
        builder.push( { -node_id, -child_id } );
        builder.push( { node_id, child_id } );
        return node_id;
    }

    int to_string_go( std::ostream &ss, int level ) override 
    {
        str_indent( ss, level * 2 ) << "not\n";
        return child->to_string_go( ss, level + 1 );
    }
};

template < typename var_t >
form_ptr< var_t > make_not( form_ptr< var_t > child )
{
    return std::make_shared< node_not >( child );
}


template < typename var_t >
struct nnary_node : form< var_t >
{
    std::vector< form_ptr< var_t > > children;

    nnary_node( std::vector< form_ptr< var_t > > children ) 
        : children( std::move( children ) ){};

    protected: 
    std::vector< int > export_children( cnf_builder< var_t >& builder )
    {
        std::vector< int > children_ids;
        for ( auto &c : children ) 
            children_ids.push_back( c->to_cnf_go( builder ) );
        return std::move( children_ids );
    }

    std::ostream& print_children( std::ostream& ss, int level )
    {
        for ( auto &c : children )
            c->to_string_go( ss, level );
        return ss;
    }

    public:
    void push( form_ptr< var_t > f ) { 
        children.push_back( f ); 
    };

    template< typename T >
    void push( T f ) { 
        children.push_back( std::make_shared< T >( std::move( f ) ) ); 
    };
};

//// And ////

template < typename var_t >
struct node_and : public nnary_node< var_t >
{

    node_and() : nnary_node< var_t >( {} ) {};

    node_and( std::vector< form_ptr< var_t > > children ) 
        : nnary_node< var_t >( std::move( children ) ) {};

    std::ostream& to_string_go( std::ostream &ss, int level ) override 
    {
        str_indent( ss, level * 2 ) << "and\n";
        this->print_children( ss, level + 1 );
        return ss;
    }

    int to_cnf_go( cnf_builder< var_t > &builder ) override 
    {
        int node_id = builder.get_node_id();
        auto children_ids = this->export_children( builder );

        for ( auto &i : children_ids ) {
            builder.push( { i, -node_id } );
            i *= -1;
        }
        children_ids.push_back( node_id );
        builder.push( std::move( children_ids ) );
        return node_id;
    };
};

template < typename var_t >
form_ptr< var_t > make_and()
{
    return std::make_shared< node_and< var_t > >();
}

template < typename var_t >
form_ptr< var_t > make_and( std::vector< form_ptr< var_t > > children )
{
    return std::make_shared< node_and< var_t > >( children );
}

//// Or //// 

template < typename var_t >
struct node_or : public nnary_node< var_t >
{
    node_or() 
        : nnary_node< var_t >( {} ) {};

    node_or( std::vector< form_ptr< var_t > > children ) 
        : nnary_node< var_t >( std::move( children ) ){};

    std::ostream& to_string_go( std::ostream &ss, int level ) override 
    {
        str_indent( ss, level * 2 ) << "or\n";
        this->print_children( ss, level + 1 );
        return ss;
    }

    int to_cnf_go( cnf_builder< var_t >& builder ) override 
    {
        int node_id = builder.get_node_id();
        auto children_ids = this->export_children( builder );
        for ( auto &i : children_ids )
            builder.push( { -i, node_id } );
        children_ids.push_back( -node_id );
        builder.push( std::move( children_ids ) );
        return node_id;
    };
};

template < typename var_t >
form_ptr< var_t > make_or()
{
    return std::make_shared< node_or< var_t > >();
}

template < typename var_t >
form_ptr< var_t > make_or( std::vector< form_ptr< var_t > > children )
{
    return std::make_shared< node_or< var_t > >( children );
}

template < typename var_t >
form_ptr< var_t > make_imp( form_ptr< var_t > premise
                          , form_ptr< var_t > conclusion )
{
    std::vector< form_ptr< var_t > > elements;
    elements.push_back( make_not( premise ) );
    elements.push_back( conclusion );
    return std::make_shared< node_or< var_t > >( elements );
}

template < typename var_t >
form_ptr< var_t > make_imp( std::vector< form_ptr< var_t > > premises
                          , form_ptr< var_t > conclusion )
{
    std::vector< form_ptr< var_t > > elements;
    for ( auto &p : premises )
        elements.push_back( make_not( p ) );
    elements.push_back( conclusion );
    return std::make_shared< node_or< var_t > >( elements );
}

//// Variable ////

template< typename var_t >
struct node_literal : form< var_t >
{
    var_t var;

    bool pos;

    node_literal( var_t var, bool pos ) 
        : var( var )
        , pos( pos )
    {};

    std::ostream& to_string_go( std::ostream &ss, int level ) override 
    {
        str_indent( ss, level * 2 ); 
        if ( !pos ) ss << "! ";
        ss << var << "\n";
        return ss;
    }

    int to_cnf_go( cnf_builder< var_t >& builder ) override 
    {
        int var_id = builder.get_var_id( var );
        return pos ? var_id : - var_id;
    };
};

template< typename var_t >
form_ptr< var_t > make_lit( var_t var, bool pos = true )
{
    return std::make_shared< node_literal< var_t > >( var, pos );
}

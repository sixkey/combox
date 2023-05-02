#pragma once

#include <cstddef>
#include <memory>
#include <vector> 
#include <boost/bimap.hpp>
#include <boost/dynamic_bitset.hpp>

#include "kck_str.hpp"
#include "kck_cnf.hpp"

namespace kck {

template < typename var_t >
using labeler_t = var_t (*)( int );

template < typename var_t >
struct cnf_builder
{
    cnf_t< var_t > output;
    labeler_t< var_t > labeler;
    int label_counter = 0;

    cnf_builder( labeler_t< var_t > labeler ) 
    {
        this->labeler = labeler;
    }

    int get_node_id() { return label_counter++; }

    var_t get_help_var() { return labeler( get_node_id() ); }

    void push( cnf_clause_t< var_t > clause )
    {
        output.push_back( clause );
    }

    void push( var_t v ) 
    {
        output.push_back( { v } );
    }
};

template < typename var_t >
struct formula
{
    virtual lit_t< var_t > to_cnf_go( cnf_builder< var_t >& builder ) = 0;
    virtual std::ostream &to_string_go( std::ostream &ss, int level ) = 0;

    cnf_t< var_t > to_cnf( labeler_t< var_t > labeler )
    {
        cnf_builder< var_t > builder( labeler );
        auto top_lit = this->to_cnf_go( builder );
        builder.push( { top_lit } );
        return builder.output;
    }

    std::string to_string() 
    {
        std::stringstream ss;
        this->to_string_go( ss, 0 );
        return ss.str();
    }
};

template < typename var_t > 
using formula_ptr = std::shared_ptr< formula< var_t > >;

std::ostream& str_indent( std::ostream& ss, int level );

//// Not ////

template < typename var_t >
struct node_not : public formula< var_t > 
{
    formula_ptr< var_t > child;

    node_not( formula_ptr< var_t > child ) 
        : child( child ){};

    lit_t< var_t > to_cnf_go( cnf_builder< var_t >& builder ) override 
    {
        var_t node_var = builder.get_help_var();
        lit_t< var_t > child_lit = child->to_cnf_go( builder );
        builder.push( { { node_var, false }, -child_lit } );
        builder.push( { { node_var, true  }, +child_lit } );
        return { node_var, true };
    }

    int to_string_go( std::ostream &ss, int level ) override 
    {
        str_indent( ss, level * 2 ) << "not\n";
        return child->to_string_go( ss, level + 1 );
    }
};

template < typename var_t >
formula_ptr< var_t > make_not( formula_ptr< var_t > child )
{
    return std::make_shared< node_not >( child );
}


template < typename var_t >
struct nnary_node : formula< var_t >
{
    std::vector< formula_ptr< var_t > > children;

    nnary_node( std::vector< formula_ptr< var_t > > children ) 
        : children( std::move( children ) ){};

    protected: 
    std::vector< lit_t< var_t > > export_children( cnf_builder< var_t >& builder )
    {
        std::vector< lit_t< var_t > > children_ids;
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
    void push( formula_ptr< var_t > f ) { 
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

    node_and( std::vector< formula_ptr< var_t > > children ) 
        : nnary_node< var_t >( std::move( children ) ) {};

    std::ostream& to_string_go( std::ostream &ss, int level ) override 
    {
        str_indent( ss, level * 2 ) << "and\n";
        this->print_children( ss, level + 1 );
        return ss;
    }

    lit_t< var_t > to_cnf_go( cnf_builder< var_t > &builder ) override 
    {
        var_t node_var = builder.get_help_var();
        auto children_ids = this->export_children( builder );
        for ( auto &i : children_ids ) {
            builder.push( { i, { node_var, false } } );
            i = -i;
        }
        children_ids.push_back( { node_var, true } );
        builder.push( std::move( children_ids ) );
        return { node_var, true };
    };
};

template < typename var_t >
formula_ptr< var_t > make_and()
{
    return std::make_shared< node_and< var_t > >();
}

template < typename var_t >
formula_ptr< var_t > make_and( std::vector< formula_ptr< var_t > > children )
{
    return std::make_shared< node_and< var_t > >( children );
}

//// Or //// 

template < typename var_t >
struct node_or : public nnary_node< var_t >
{
    node_or() 
        : nnary_node< var_t >( {} ) {};

    node_or( std::vector< formula_ptr< var_t > > children ) 
        : nnary_node< var_t >( std::move( children ) ){};

    std::ostream& to_string_go( std::ostream &ss, int level ) override 
    {
        str_indent( ss, level * 2 ) << "or\n";
        this->print_children( ss, level + 1 );
        return ss;
    }

    lit_t< var_t > to_cnf_go( cnf_builder< var_t >& builder ) override 
    {
        var_t node_var = builder.get_help_var();
        auto children_ids = this->export_children( builder );
        for ( auto &i : children_ids )
            builder.push( { -i, { node_var, true } } );
        children_ids.push_back( { node_var, false } );
        builder.push( std::move( children_ids ) );
        return { node_var, true };
    };
};

template < typename var_t >
formula_ptr< var_t > make_or()
{
    return std::make_shared< node_or< var_t > >();
}

template < typename var_t >
formula_ptr< var_t > make_or( std::vector< formula_ptr< var_t > > children )
{
    return std::make_shared< node_or< var_t > >( children );
}

template < typename var_t >
formula_ptr< var_t > make_imp( formula_ptr< var_t > premise
                          , formula_ptr< var_t > conclusion )
{
    std::vector< formula_ptr< var_t > > elements;
    elements.push_back( make_not( premise ) );
    elements.push_back( conclusion );
    return std::make_shared< node_or< var_t > >( elements );
}

template < typename var_t >
formula_ptr< var_t > make_imp( std::vector< formula_ptr< var_t > > premises
                          , formula_ptr< var_t > conclusion )
{
    std::vector< formula_ptr< var_t > > elements;
    for ( auto &p : premises )
        elements.push_back( make_not( p ) );
    elements.push_back( conclusion );
    return std::make_shared< node_or< var_t > >( elements );
}

//// Variable ////

template< typename var_t >
struct node_literal : formula< var_t >
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

    lit_t< var_t > to_cnf_go( cnf_builder< var_t >& builder ) override 
    {
        return { var, pos };
    };
};

template< typename var_t >
formula_ptr< var_t > make_lit( var_t var, bool pos = true )
{
    return std::make_shared< node_literal< var_t > >( var, pos );
}

}

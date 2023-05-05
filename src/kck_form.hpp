#pragma once

#include <cstddef>
#include <memory>
#include <vector> 
#include <boost/bimap.hpp>
#include <boost/dynamic_bitset.hpp>

#include "kck_str.hpp"
#include "kck_cnf.hpp"

namespace kck {

template < typename lit_t >
using labeler_t = lit_t (*)( int );

template < typename lit_t >
struct cnf_builder
{
    using var_t = typename lit_t::var_t;

    cnf_t< lit_t > output;
    labeler_t< lit_t > labeler;
    int label_counter = 0;

    cnf_builder( labeler_t< lit_t > labeler ) 
    {
        this->labeler = labeler;
    }

    int get_node_id() { return label_counter++; }

    // Gives a positive literal
    lit_t get_help_lit() { return labeler( get_node_id() ); }

    void push( cnf_clause_t< lit_t > clause )
    {
        output.push_back( clause );
    }

    void push( lit_t v ) 
    {
        output.push_back( { v } );
    }
};

template < typename lit_t >
struct formula
{
    using var_t = typename lit_t::var_t;
    virtual lit_t to_cnf_go( cnf_builder< lit_t >& builder ) const = 0;
    
    virtual std::ostream &to_string_go( std::ostream &ss, int level ) const = 0;

    cnf_t< lit_t > to_cnf( cnf_builder< lit_t > &builder )
    {
        auto top_lit = this->to_cnf_go( builder );
        builder.push( { top_lit } );
        return std::move( builder.output );
    }

    cnf_t< lit_t > to_cnf( labeler_t< lit_t > labeler )
    {
        cnf_builder< lit_t > builder( labeler );
        return to_cnf( builder );
    }

    std::string to_string() const
    {
        std::stringstream ss;
        this->to_string_go( ss, 0 );
        return ss.str();
    }
};

template < typename lit_t >
std::ostream& operator<<( std::ostream& os, const std::shared_ptr< formula < lit_t > > &form )
{
    return os << form->to_string();
}

template < typename lit_t >
std::ostream& operator<<( std::ostream& os, const formula< lit_t > &form )
{
    return os << form.to_string();
}

template < typename lit_t > 
using formula_ptr = std::shared_ptr< formula< lit_t > >;

std::ostream& str_indent( std::ostream& ss, int level );

//// Not ////

template < typename lit_t >
struct node_not : public formula< lit_t > 
{
    using var_t = typename lit_t::var_t;
    formula_ptr< lit_t > child;

    node_not( formula_ptr< lit_t > child ) 
        : child( child ){};

    lit_t to_cnf_go( cnf_builder< var_t >& builder ) const override 
    {
        lit_t node_lit = builder.get_help_lit();
        lit_t child_lit = child->to_cnf_go( builder );
        builder.push( { -node_lit, -child_lit } );
        builder.push( {  node_lit,  child_lit } );
        return node_lit;
    }

    int to_string_go( std::ostream &ss, int level ) const override 
    {
        str_indent( ss, level * 2 ) << "not\n";
        return child->to_string_go( ss, level + 1 );
    }
};


template < typename lit_t >
struct nnary_node : formula< lit_t >
{
    std::vector< formula_ptr< lit_t > > children;

    nnary_node( std::vector< formula_ptr< lit_t > > children ) 
        : children( std::move( children ) ){};

    protected: 
    std::vector< lit_t > export_children( cnf_builder< lit_t >& builder ) const
    {
        std::vector< lit_t > children_ids;
        for ( auto &c : children ) 
            children_ids.push_back( c->to_cnf_go( builder ) );
        return children_ids;
    }

    std::ostream& print_children( std::ostream& ss, int level ) const
    {
        for ( auto &c : children )
            c->to_string_go( ss, level );
        return ss;
    }

    public:
    void push( formula_ptr< lit_t > f ) { 
        children.push_back( f ); 
    };

    template< typename T >
    void push( T f ) { 
        children.push_back( std::make_shared< T >( std::move( f ) ) ); 
    };
};

//// And ////

template < typename lit_t >
struct node_and : public nnary_node< lit_t >
{
    node_and() : nnary_node< lit_t >( {} ) {};

    node_and( std::vector< formula_ptr< lit_t > > children ) 
        : nnary_node< lit_t >( std::move( children ) ) {};

    std::ostream& to_string_go( std::ostream &ss, int level ) const override 
    {
        str_indent( ss, level * 2 ) << "and\n";
        this->print_children( ss, level + 1 );
        return ss;
    }

    lit_t to_cnf_go( cnf_builder< lit_t > &builder ) const override 
    {
        lit_t node_lit = builder.get_help_lit();
        auto children_ids = this->export_children( builder );
        for ( auto &i : children_ids ) {
            builder.push( { i, -node_lit } );
            i = -i;
        }
        children_ids.push_back( node_lit );
        builder.push( std::move( children_ids ) );
        return node_lit;
    };
};


//// Or //// 

template < typename lit_t >
struct node_or : public nnary_node< lit_t >
{
    node_or() 
        : nnary_node< lit_t >( {} ) {};

    node_or( std::vector< formula_ptr< lit_t > > children ) 
        : nnary_node< lit_t >( std::move( children ) ){};

    std::ostream& to_string_go( std::ostream &ss, int level ) const override 
    {
        str_indent( ss, level * 2 ) << "or\n";
        this->print_children( ss, level + 1 );
        return ss;
    }

    lit_t to_cnf_go( cnf_builder< lit_t >& builder ) const override 
    {
        lit_t node_lit = builder.get_help_lit();
        auto children_ids = this->export_children( builder );
        for ( auto &i : children_ids )
            builder.push( { -i, node_lit } );
        children_ids.push_back( -node_lit );
        builder.push( std::move( children_ids ) );
        return node_lit;
    };
};


//// Variable ////

template< typename lit_t >
struct node_literal : formula< lit_t >
{
    lit_t lit;

    node_literal( lit_t lit ) : lit( lit )
    {};

    std::ostream& to_string_go( std::ostream &ss, int level ) const override 
    {
        str_indent( ss, level * 2 ); 
        ss << lit << "\n";
        return ss;
    }

    lit_t to_cnf_go( cnf_builder< lit_t >& builder ) const override 
    {
        return lit;
    };
};


template< typename lit_t_ >
struct forms_t
{

    using lit_t = lit_t_;

    using or_t = node_or< lit_t >;
    using and_t = node_and< lit_t >;
    using not_t = node_not< lit_t >;
    using nlit_t = node_literal< lit_t >;
    using form_p = formula_ptr< lit_t >;

    static formula_ptr< lit_t > f_lit( lit_t lit )
    {
        return std::make_shared< node_literal< lit_t > >( lit );
    }

    static formula_ptr< lit_t > f_and()
    {
        return std::make_shared< node_and< lit_t > >();
    }

    static formula_ptr< lit_t > f_and( std::vector< formula_ptr< lit_t > > children )
    {
        return std::make_shared< node_and< lit_t > >( children );
    }

    static formula_ptr< lit_t > f_or()
    {
        return std::make_shared< node_or< lit_t > >();
    }

    static formula_ptr< lit_t > f_or( std::vector< formula_ptr< lit_t > > children )
    {
        return std::make_shared< node_or< lit_t > >( children );
    }

    static formula_ptr< lit_t > f_imp( formula_ptr< lit_t > premise
                                     , formula_ptr< lit_t > conclusion )
    {
        std::vector< formula_ptr< lit_t > > elements;
        elements.push_back( not( premise ) );
        elements.push_back( conclusion );
        return std::make_shared< node_or< lit_t > >( elements );
    }

    static formula_ptr< lit_t > f_imp( std::vector< formula_ptr< lit_t > > premises
                                        , formula_ptr< lit_t > conclusion )
    {
        std::vector< formula_ptr< lit_t > > elements;
        for ( auto &p : premises )
            elements.push_back( f_not( p ) );
        elements.push_back( conclusion );
        return std::make_shared< node_or< lit_t > >( elements );
    }
};
}

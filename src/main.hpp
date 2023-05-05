#include <iostream>
#include <vector>

#include "kck_cnf.hpp"
#include "kck_form.hpp" 
#include "kck_sat.hpp"

using pattern = std::array< int, 3 >;
using palette_t = std::vector< pattern >;
using var_t = std::pair< char, std::vector< int > >;

namespace kck {

std::ostream& operator<<( std::ostream& os, const std::vector< int > &v );
std::ostream& operator<<( std::ostream& os, const std::set< int > &s );

template < typename L, typename R >
std::ostream& operator<<( std::ostream& os, const std::pair< L, R > &var )
{
    return os << "(" << var.first << ", " << var.second << ")";
}

}

using lit_t = kck::literal< var_t >;
using forms = kck::forms_t< lit_t >;

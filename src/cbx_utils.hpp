#pragma once

namespace cbx {

//// Incidence functions //////////////////////////////////////////////////////

template < int n >
constexpr int adj_i( int i, int j ) { return i * n + j; }

constexpr int adj_i( int n, int i, int j ) { return i * n + j; }

//// Sorting //////////////////////////////////////////////////////////////////

void sort_i ( int& a, int& b );

void sort_i ( int& a, int& b, int& c );

}

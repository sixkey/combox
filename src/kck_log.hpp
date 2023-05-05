#pragma once

#include <iostream>

#ifdef TRACING

#define TRACE(...) trace(...)

#else

#define TRACE(...) 

#endif


namespace kck 
{

void trace_go();

template < typename T, typename ...Ts >
void trace_go( const T& t, Ts... ts )
{
    std::cout << " " << t;
    trace_go( ts... );
}

template < typename T, typename ...Ts >
void trace( const char* message, const T& t, Ts... ts )
{
    std::cout << "[" << message << "]";
    trace_go( t, ts... );
}

template < typename T >
void trace( const T& t )
{
    trace( "trc", t );
}

}


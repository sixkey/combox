
template < typename T >
struct bar
{
    using var_t = typename T::var_t;

    int hello() {};
};

template < typename T >
struct foo 
{
    foo< int > int_version()
    {
        bar< T > b;
        b.hello();
    }
};

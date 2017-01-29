#pragma once

#include <string>
#include <memory>
#include <exception>

#include "math_algorithms.h"

namespace currency_convertion
{

class Converter;

class Money
{
public:
    typedef double ValueType;
    typedef std::shared_ptr< const Converter > ConverterPtr;
    
    struct Exception;

    Money( ValueType value, const std::string& name );
    Money( ValueType&& value, std::string&& name );

    Money( ValueType value, const std::string& name, ConverterPtr converter );
    Money( ValueType&& value, std::string&& name, ConverterPtr converter );

    Money( const Money& moneyFrom, const std::string& name );

    const std::string& GetName() const;
    ValueType GetValue() const;

    void Swap( Money& lhs );

    Money& operator+=( const Money& rhs );
    template < class T
        , class = typename std::enable_if< 
            std::is_arithmetic< typename std::decay< T >::type
            >::value
        >::type >
    Money& operator+=( T&& rhs )
    {
        value_ += std::forward< T >( rhs );
        return *this;
    }

    Money& operator-=( const Money& rhs );
    template < class T
        , class = typename std::enable_if<
            std::is_arithmetic< typename std::decay< T >::type
            >::value
        >::type >
        Money& operator-=( T&& rhs )
    {
        value_ -= std::forward< T >( rhs );
        return *this;
    }

    Money& operator=( const Money& rhs );
    template < class T
        , class = typename std::enable_if<
        std::is_arithmetic< typename std::decay< T >::type
        >::value
    >::type >
    Money& operator=( T&& rhs )
    {
        value_ = std::forward< T >( rhs );
        return *this;
    }

    bool operator==( const Money& rhs ) const;
    template < class T >
    bool operator==( T rhs ) const
    {
        return isEqualRounded( value_, rhs, 5 );
    }

private:
    ValueType value_;
    std::string name_;
    ConverterPtr converter_;
};

template < class T
    , class = typename std::enable_if<
    std::is_same< typename std::decay< T >::type, Money >::value
    >::type > inline
    Money operator+( T&& lhs, const Money& rhs )
{
    Money tmp( std::forward< T >( lhs ) );
    return tmp += rhs;
}

template < class M, class T
    , class = typename std::enable_if<
    std::is_same< typename std::decay< M >::type, Money >::value &&
    std::is_arithmetic< typename std::decay< T >::type >::value
>::type > inline
Money operator+( M&& lhs, T&& rhs )
{
    M tmp( std::forward< M >( lhs ) );
    return tmp += std::forward< T >( rhs );
}

template < class T
    , class = typename std::enable_if<
    std::is_same< typename std::decay< T >::type, Money >::value
    >::type > inline
    Money operator-( T&& lhs, const Money& rhs )
{
    Money tmp( std::forward< T >( lhs ) );
    return tmp -= rhs;
}

template < class M, class T
    , class = typename std::enable_if<
    std::is_same< typename std::decay< M >::type, Money >::value &&
    std::is_arithmetic< typename std::decay< T >::type >::value
    >::type > inline
    Money operator-( M&& lhs, T&& rhs )
{
    M tmp( std::forward< M >( lhs ) );
    return tmp -= std::forward< T >( rhs );
}

}
#pragma once

#include <string>
#include <memory>
#include <exception>

class Converter;

class Money
{
public:
    typedef double ValueType;
    typedef std::shared_ptr< Converter > ConverterPtr;
    typedef std::domain_error MoneyException;

    Money( ValueType value, const std::string& name );
    Money( ValueType value, const std::string& name, ConverterPtr converter );

    const std::string& GetName() const;
    ValueType GetValue() const;

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

private:
    ValueType value_;
    std::string name_;
    std::shared_ptr< Converter > converter_;
};

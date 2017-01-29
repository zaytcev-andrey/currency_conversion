#include "stdafx.h"

#include "money.h"
#include "converter.h"

#include <utility>
#include <exception>

namespace currency_convertion
{ 

struct Money::Exception : std::runtime_error
{
    explicit Exception( const std::string& str )
        : std::runtime_error( str )
    {
    }

    explicit Exception( const char* str )
        : std::runtime_error( str )
    {
    }
};

Money::Money( ValueType value, const std::string& name )
    : value_( value )
    , name_( name )
{
}

Money::Money( ValueType&& value, std::string&& name )
    : value_( std::move( value ) )
    , name_( std::move( name ) )
{
}


Money::Money( ValueType value, const std::string& name, ConverterPtr converter )
    : value_( value )
    , name_( name )
    , converter_( converter )
{
}

Money::Money( ValueType&& value, std::string&& name, ConverterPtr converter )
    : value_( std::move( value ) )
    , name_( std::move( name ) )
    , converter_( converter )
{
}

Money::Money( const Money& moneyFrom, const std::string& name )
    : value_( 0 )
    , name_( name )
    , converter_( moneyFrom.converter_ )
{
    if ( moneyFrom.name_ == name_ )
    {
        value_ += moneyFrom.value_;
    }
    else
    {
        if ( !converter_ )
        {
            throw Exception( "Can not convert \"" + name_ + "\"->\"" + name + "\"" + " without converter" );
        }

        value_ = converter_->Convert( moneyFrom, name );
    }
}

void Money::Swap( Money& lhs )
{
    std::swap( value_, lhs.value_ );
    std::swap( name_, lhs.name_ );
    std::swap( converter_, lhs.converter_ );
}

const std::string& Money::GetName() const
{
    return name_;
}

Money::ValueType Money::GetValue() const
{
    return value_;
}

Money& Money::operator+=( const Money& rhs )
{
    ConverterPtr converter = converter_ ? converter_ : rhs.converter_;

    if ( rhs.name_ == name_ )
    {
        value_ += rhs.value_;
    }
    else if ( converter )
    {
        value_ += converter->Convert( rhs, name_ );
    }
    else
    {
        throw Exception( "Can not convert \"" + name_ + "\"->\"" + rhs.name_ + "\"" + " without converter" );
    }

    return *this;
}

Money& Money::operator-=( const Money& rhs )
{
    ConverterPtr converter = converter_ ? converter_ : rhs.converter_;
    
    if ( rhs.name_ == name_ )
    {
        value_ -= rhs.value_;
    }
    else if ( converter )
    {
        value_ -= converter->Convert( rhs, name_ );
    }
    else
    {
        throw Exception( "Can not convert \"" + name_ + "\"->\"" + rhs.name_ + "\"" + " without converter" );
    }

    return *this;
}

Money& Money::operator=( const Money& rhs )
{
    Money tmp( rhs, name_ );
    Swap( tmp );

    return *this;
}

bool Money::operator==( const Money& rhs ) const
{
    if ( name_ == rhs.name_ )
    {
        return isEqualRounded( value_, rhs.value_, 5 );
    }
    else
    {
        ConverterPtr converter = converter_ ? converter_ : rhs.converter_;

        if ( converter )
        {
            return value_ == converter->Convert( rhs, name_ );
        }
        else
        {
            throw Exception( "Can not convert \"" + name_ + "\"->\"" + rhs.name_ + "\"" + " without converter" );
        }
    }
}

}
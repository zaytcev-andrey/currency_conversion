#include "stdafx.h"

#include "money.h"
#include "converter.h"

Money::Money( ValueType value, const std::string& name )
    : value_( value )
    , name_( name )
{

}

Money::Money( ValueType value, const std::string& name, ConverterPtr converter )
    : value_( value )
    , name_( name )
    , converter_( converter )
{

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
    if ( rhs.name_ == name_ )
    {
        value_ += rhs.value_;
    }
    else if ( converter_ )
    {

    }
    else
    {
        throw MoneyException( "another type without converter" );
    }

    return *this;
}

Money& Money::operator-=( const Money& rhs )
{
    if ( rhs.name_ == name_ )
    {
        value_ -= rhs.value_;
    }
    else if ( converter_ )
    {

    }
    else
    {
        throw MoneyException( "another type without converter" );
    }

    return *this;
}
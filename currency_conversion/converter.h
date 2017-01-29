#pragma once

#include "instrument.h"
#include "instrument_dependency.h"
#include "money.h"

#include <string>
#include <memory>

namespace currency_convertion
{

class Converter : public std::enable_shared_from_this< Converter >
{
public:
    struct Exception : std::runtime_error
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

    template < class InstrumentContainer >
    explicit Converter( const InstrumentContainer& instruments )
        : instrumentDependency_( instruments )
    {}

    Money::ValueType Convert( const Money& moneyFrom, const std::string& nameTo ) const
    {
        const auto nameFrom = moneyFrom.GetName();
        Money::ValueType value = moneyFrom.GetValue();

        if ( nameFrom != nameTo )
        {
            const auto instrumentChain = instrumentDependency_.GetInstrumentChain( nameFrom, nameTo );

            if ( instrumentChain.empty() )
            {
                std::string mess = "There is no conversion \"" + moneyFrom.GetName() + "\"->\"" + nameTo + "\"";
                throw Exception( mess );
            }

            for ( const auto& instrument : instrumentChain )
            {                
                value *= instrument.GetRatio();
            }
        }

        return value;
    }

    void NewQuote( const std::string& name, Instrument::RatioType bid, Instrument::RatioType ask )
    {
        try
        {
            instrumentDependency_.NewQuote( name, bid, ask );
        }
        catch ( const std::out_of_range& err )
        {
            throw std::runtime_error( err.what() );
        }        
    }

    template < class T, class S
        , class = typename std::enable_if<
            std::is_arithmetic< typename std::decay< T >::type >::value            
        >::type >
    Money MakeMoney( T&& value, S&& name )
    {
        return Money( std::forward< T >( value ), std::forward< S >( name ), shared_from_this() );
    }

    template < class S >
    Money MakeMoney( const Money& money, S&& name )
    {
        if ( money.GetName() == name )
        {
            return Money( money.GetValue(), std::forward< S >( name ), shared_from_this() );
        }

        return { Convert( money, name ), name, shared_from_this() };
    }

private:
    InstrumentDependency instrumentDependency_;
};

}
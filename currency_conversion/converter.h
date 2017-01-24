#pragma once

#include "instrument.h"
#include "instrument_dependency.h"
#include "money.h"

#include <string>

class Converter : public std::enable_shared_from_this< Converter >
{
public:
    Converter( const std::set< Instrument >& instruments )
        : instrumentDependency_( instruments )
    {}

    Money Convert( const Money& moneyFrom, const std::string nameTo ) const
    {
        const auto nameFrom = moneyFrom.GetName();

        if ( nameFrom != nameTo )
        {
            const auto instrumentChain = instrumentDependency_.GetInstrumentChain( nameFrom, nameTo );

            for ( const auto& instrument : instrumentChain )
            {                
                Money( instrument.GetRatio(), instrument.GetDestCurrency() );
            }
        }

        return moneyFrom;
    }

    void NewQuote( const std::string& name, Instrument::RatioType bid, Instrument::RatioType ask )
    {
        instrumentDependency_.NewQuote( name, bid, ask );
    }

    template < class T
        , class = typename std::enable_if<
            std::is_arithmetic< typename std::decay< T >::type
            >::value
        >::type >
    Money MakeMoney( T&& value, const std::string& name )
    {
        return Maney( std::forward< T >( value ), name, shared_from_this() );
    }

    Money MakeMoney( const Money& money, const std::string& name )
    {
        if ( money.GetName() == name )
        {
            return Money( money.GetValue(), name, shared_from_this() );
        }

        
    }

private:
    InstrumentDependency instrumentDependency_;
};

#pragma once

#include "instrument_details.h"

#include <string>

namespace currency_convertion
{

class DirectedInstrument
{
public:
    DirectedInstrument(
        const std::string& name
        , double ratio )
        : name_( name )
        , sourceCurrencyName_( details::ExtractFirstCurrency( name ) )
        , destCurrencyName_( details::ExtractSecondCurrency( name ) )
        , ratio_( ratio )
    {
    }

    const std::string& GetName() const
    {
        return name_;
    }

    const std::string& GetSourceCurrency() const
    {
        return sourceCurrencyName_;
    }

    const std::string& GetDestCurrency() const
    {
        return destCurrencyName_;
    }

    double GetRatio() const
    {
        return ratio_;
    }

private:
    std::string name_;
    std::string sourceCurrencyName_;
    std::string destCurrencyName_;
    double ratio_;
};

template< class Instrument >
std::pair< DirectedInstrument, DirectedInstrument >
SplitInstrument( Instrument&& instrument )
{
    DirectedInstrument straight( instrument.GetName(), instrument.GetBid() );
    DirectedInstrument reverse( details::ReverseInstrumentName( instrument.GetName() ), 1 / instrument.GetAsk() );

    return std::make_pair( std::move( straight ), std::move( reverse ) );
}

}
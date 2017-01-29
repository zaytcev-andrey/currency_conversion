#pragma once

#include "instrument_details.h"

#include <string>
#include <exception>

namespace currency_convertion
{

class Instrument
{
public:
    typedef double RatioType;

    Instrument(
        const std::string& name
        , RatioType bid
        , RatioType ask )
        : name_( name )
        , baseCurrencyName_( details::ExtractFirstCurrency( name ) )
        , qoutedCurrencyName_( details::ExtractSecondCurrency( name ) )
        , bid_( bid )
        , ask_( ask )
    {
    }

    const std::string& GetName() const
    {
        return name_;
    }

    const std::string& GetBaseCurrency() const
    {
        return baseCurrencyName_;
    }

    const std::string& GetQoutedCurrency() const
    {
        return qoutedCurrencyName_;
    }

    RatioType GetBid() const
    {
        return bid_;
    }

    RatioType GetAsk() const
    {
        return ask_;
    }

    bool operator < ( const Instrument& rhs ) const
    {
        return name_ < rhs.name_;
    }

private:
    std::string name_;
    std::string baseCurrencyName_;
    std::string qoutedCurrencyName_;
    RatioType bid_;
    RatioType ask_;
};

}
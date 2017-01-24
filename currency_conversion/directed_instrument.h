#pragma once

#include "instrument_details.h"

#include <string>

class DirectedInstrument
{
public:
    DirectedInstrument(
        const std::string& name
        , double ratio )
        : name_( name )
        , sourceCurrencyName_( ExtractFirstCurrency( name ) )
        , destCurrencyName_( ExtractSecondCurrency( name ) )
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
#pragma once

#include <string>

inline std::string ExtractFirstCurrency( const std::string& instrumentName )
{
    const auto found = instrumentName.find_first_of( '/' );

    if (found != std::string::npos)
    {
        const auto base = instrumentName.substr( 0, found );
        if (!base.empty())
        {
            return base;
        }
    }

    throw std::invalid_argument( "ExtractFirstCurrency: invalid argument" );
}

inline std::string ExtractSecondCurrency( const std::string& instrumentName )
{
    const auto found = instrumentName.find_first_of( '/' );

    if (found != std::string::npos)
    {
        const auto qouted = instrumentName.substr( found + 1, instrumentName.size() - found - 1 );
        if (!qouted.empty())
        {
            return qouted;
        }
    }

    throw std::invalid_argument( "ExtractSecondCurrency: invalid argument" );
}

inline std::string ReverseInstrumentName( const std::string& instrumentName )
{
    const auto first = ExtractFirstCurrency( instrumentName );
    const auto second = ExtractSecondCurrency( instrumentName );

    return second + "/" + first;
}

inline std::string MergeInstrumentNames( const std::string& nameFirst, const std::string& nameSecond )
{
    return nameFirst + "/" + nameSecond;
}
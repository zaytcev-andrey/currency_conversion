#pragma once

#include "instrument.h"
#include "instrument_details.h"
#include "directed_instrument.h"

#include <utility>

inline std::pair< DirectedInstrument, DirectedInstrument > 
SplitInstrument( const Instrument& instrument )
{
    DirectedInstrument straight( instrument.GetName(), instrument.GetBid() );
    DirectedInstrument reverse( ReverseInstrumentName( instrument.GetName() ), instrument.GetAsk() );

    return std::make_pair( straight, reverse );
}

template< class Straight, class Reverse > 
inline std::unique_ptr< DirectedInstrument >
GetDirectedInstrument( const std::string& name, const Straight& straight, const Reverse& reverse )
{
    auto itr = straight.find( name );
    if ( itr != straight.end() )
    {
        return std::unique_ptr< DirectedInstrument >( new DirectedInstrument( itr->second ) );
    }

    itr = reverse.find( name );
    if ( itr != reverse.end())
    {
        return std::unique_ptr< DirectedInstrument >( new DirectedInstrument( itr->second ) );
    }

    return nullptr;
}
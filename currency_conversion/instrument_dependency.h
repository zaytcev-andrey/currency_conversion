#pragma once

#include "instrument.h"
#include "directed_instrument.h"
#include "path_explorer.h"
#include "graph_utils.h"

#include <unordered_map>
#include <exception>
#include <iostream>

#include "boost/graph/adjacency_list.hpp"
#include "boost/graph/breadth_first_search.hpp"
#include "boost/graph/visitors.hpp"

namespace currency_convertion
{

class InstrumentDependency
{
    typedef boost::adjacency_list
        <
        boost::vecS                                         
        , boost::vecS                                        
        , boost::undirectedS                                 
        , boost::property<boost::vertex_name_t, std::string>
        , boost::property<boost::edge_weight_t, int>        
        >   InstrumentGraph;

    typedef boost::graph_traits< InstrumentGraph >::vertex_descriptor Vertex;
    typedef boost::graph_traits< InstrumentGraph >::edge_descriptor Edge;
    typedef std::unordered_map< std::string, Vertex > VertexNameMap;

    typedef std::unordered_map< std::string, DirectedInstrument > DirectedInstrumentMap;

public:
    typedef std::vector< DirectedInstrument > InstrumentChain;
    typedef std::runtime_error Exception;

    template < class InstrumentContainer >
    explicit InstrumentDependency( const InstrumentContainer& instruments )
    {
        for ( const auto& itr : instruments )
        {
            const auto& first = itr.GetBaseCurrency();
            const auto& second = itr.GetQoutedCurrency();

            const Vertex vertexFirst = CreateVertexByName( first );
            const Vertex vertexSecond = CreateVertexByName( second );

            boost::add_edge( vertexFirst, vertexSecond, instrumentGraph_ );

            const auto splited = SplitInstrument( itr );
            straigthInstruments_.insert( { splited.first.GetName(), splited.first } );
            reverseInstruments_.insert( { splited.second.GetName(), splited.second } );
        }

        instrumentPath_.reset( new PathExplorer< InstrumentGraph >( instrumentGraph_ ) );
    }

    InstrumentChain GetInstrumentChain( const std::string& from, const std::string& to ) const
    {       
        using namespace details;

        const auto vertexFrom = GetVertexByName( from );
        const auto vertexTo = GetVertexByName( to );

        if (vertexFrom == vertexTo)
        {
            std::vector< Vertex > vertexPath{ vertexFrom };
            std::vector< std::string > strPath;
            strPath.reserve( vertexPath.size() );
            VertexPathToStringPath( instrumentGraph_, vertexPath.begin(), vertexPath.end(), std::back_inserter( strPath ) );

            const auto name = MergeInstrumentNames( strPath[ 0 ], strPath[ 0 ] );
            auto&& instrument = GetDirectedInstrument( name );

            return { instrument };
        }
        
        const auto vertexPath = instrumentPath_->GetPath( vertexFrom, vertexTo );
        std::vector< std::string > strPath;
        strPath.reserve( vertexPath.size() );
        VertexPathToStringPath( instrumentGraph_, vertexPath.begin(), vertexPath.end(), std::back_inserter( strPath ) );

        InstrumentChain instrumentChain;

        if ( !strPath.empty() )
        {
            std::vector< std::string >::const_iterator itrFrom = strPath.cbegin();
            std::vector< std::string >::const_iterator itrTo = itrFrom + 1;
            while (itrFrom != strPath.cend() && itrTo != strPath.cend())
            {
                const auto name = MergeInstrumentNames( *itrFrom, *itrTo );
                auto&& instrument = GetDirectedInstrument( name );

                instrumentChain.push_back( instrument );
                ++itrFrom;
                ++itrTo;
            }
        }

        return instrumentChain;
    }

    void NewQuote( const std::string& name, Instrument::RatioType bid, Instrument::RatioType ask )
    {
        Instrument instrument( name, bid, ask );

        const auto splited = SplitInstrument( instrument );

        try
        {
            straigthInstruments_.at( splited.first.GetName() ) = splited.first;
            reverseInstruments_.at( splited.second.GetName() ) = splited.second;
        }
        catch ( const std::out_of_range& )
        {
            throw std::runtime_error( "Can not find instrument \"" + name + "\". Adding new instrument in run time does not implemented."  );
        }    
    }
private:

    Vertex GetVertexByName( const std::string& vertexName ) const
    {
        const auto vIter = vertexMap_.find( vertexName );
        if (vIter != vertexMap_.end())
        {
            return vIter->second;
        }

        throw Exception( "GetVertexByName: cannot get vertex by name \"" + vertexName + "\"" );
    }

    Vertex CreateVertexByName( const std::string& vertexName )
    {
        const auto vIter = vertexMap_.find( vertexName );
        if ( vIter != vertexMap_.end() )
        {
            return vIter->second;
        }

        Vertex v = boost::add_vertex( InstrumentGraph::vertex_property_type( vertexName ), instrumentGraph_ );
        vertexMap_[vertexName] = v;
        return v;
    }

    DirectedInstrument GetDirectedInstrument( const std::string& name ) const
    {
        auto itr = straigthInstruments_.find( name );
        if (itr != straigthInstruments_.end())
        {
            return itr->second;
        }

        itr = reverseInstruments_.find( name );
        if (itr != reverseInstruments_.end())
        {
            return itr->second;
        }

        if ( details::ExtractFirstCurrency( name ) == details::ExtractSecondCurrency( name ))
        {
            return{ name, 1 };
        }

        throw Exception( "Cannot find directed instrument \"" + name + "\"" );
    }


private:
    InstrumentGraph instrumentGraph_;
    VertexNameMap vertexMap_;

    DirectedInstrumentMap straigthInstruments_;
    DirectedInstrumentMap reverseInstruments_;
    
    std::unique_ptr< PathExplorer< InstrumentGraph > > instrumentPath_;
};

}
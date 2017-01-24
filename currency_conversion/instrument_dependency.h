#pragma once

#include "instrument.h"
#include "directed_instrument.h"
#include "instrument_utils.h"
#include "path_explorer.h"
#include "graph_utils.h"

#include <unordered_map>
#include <set>
#include <exception>
#include <iostream>

#include "boost/graph/adjacency_list.hpp"
#include "boost/graph/breadth_first_search.hpp"
#include "boost/graph/visitors.hpp"

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

public:
    typedef std::vector< DirectedInstrument > InstrumentChain;

    explicit InstrumentDependency( const std::set< Instrument >& instruments )
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
        const auto vertexFrom = GetVertexByName( from );
        const auto vertexTo = GetVertexByName( to );
        
        auto vertexPath = instrumentPath_->GetPath( vertexFrom, vertexTo );
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
                const auto instrument = GetDirectedInstrument( name, straigthInstruments_, reverseInstruments_ );

                instrumentChain.push_back( *instrument );
                ++itrFrom;
                ++itrTo;
            }
        }

        return instrumentChain;
    }

    void NewQuote( const std::string& name, Instrument::RatioType bid, Instrument::RatioType ask )
    {

    }
private:

    Vertex GetVertexByName( const std::string& vertexName ) const
    {
        const auto vIter = vertexMap_.find( vertexName );
        if (vIter != vertexMap_.end())
        {
            return vIter->second;
        }

        throw std::runtime_error( "GetVertexByName: cannot get vertex" );
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

private:
    InstrumentGraph instrumentGraph_;
    std::unordered_map< std::string, Vertex > vertexMap_;

    std::unordered_map< std::string, DirectedInstrument > straigthInstruments_;
    std::unordered_map< std::string, DirectedInstrument > reverseInstruments_;

    std::unique_ptr< PathExplorer< InstrumentGraph > > instrumentPath_;
};
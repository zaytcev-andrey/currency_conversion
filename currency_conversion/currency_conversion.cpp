#include "stdafx.h"

#include "instrument.h"
#include "path_explorer.h"
#include "graph_utils.h"
#include "directed_instrument.h"
#include "instrument_utils.h"
#include "instrument_dependency.h"
#include "money.h"
#include "converter.h"

#include <assert.h>
#include <string>
#include <iostream>
#include <utility>
#include <algorithm>
#include <map>
#include <unordered_map>

#include "boost/graph/adjacency_list.hpp"

using namespace boost;
typedef property<vertex_name_t, std::string> VertexNameProperty;
typedef boost::adjacency_list<listS, vecS, undirectedS, VertexNameProperty> mygraph;

int main()
{
    // instrument
    const std::string base = ExtractFirstCurrency( "EUR/USD" );
    assert( base == "EUR" );

    const std::string qouted = ExtractSecondCurrency( "EUR/USD" );
    assert( qouted == "USD" );

    {
        Instrument instrument( "EUR/USD", 1.3, 1.3 );
        assert( instrument.GetName() == "EUR/USD" );
        assert( instrument.GetBaseCurrency() == "EUR" );
        assert( instrument.GetQoutedCurrency() == "USD" );
    }

    // path explorer
    typedef boost::adjacency_list
        <
        boost::vecS                                        //! edge list 
        , boost::vecS                                        //! vertex list
        , boost::undirectedS                                 //! undirected graph  
        , boost::property<boost::vertex_name_t, std::string> //! vertex properties : name                
        , boost::property<boost::edge_weight_t, int>         //! edge properties : weight 
        >   InstrumentGraph;

    //descriptors
    typedef boost::graph_traits< InstrumentGraph >::vertex_descriptor Vertex;
    typedef boost::graph_traits< InstrumentGraph >::edge_descriptor Edge;

    InstrumentGraph currencyGraph;

    boost::property_map< InstrumentGraph,
        boost::vertex_name_t>::type vertexName = boost::get( boost::vertex_name, currencyGraph );
    boost::property_map< InstrumentGraph,
        boost::vertex_index_t>::type vertexIndx = boost::get( boost::vertex_index, currencyGraph );

    Vertex eur = boost::add_vertex( InstrumentGraph::vertex_property_type( "EUR" ), currencyGraph );
    Vertex usd = boost::add_vertex( InstrumentGraph::vertex_property_type( "USD" ), currencyGraph );
    Vertex rur = boost::add_vertex( InstrumentGraph::vertex_property_type( "RUR" ), currencyGraph );
    Vertex cad = boost::add_vertex( InstrumentGraph::vertex_property_type( "CAD" ), currencyGraph );
    Vertex dkk = boost::add_vertex( InstrumentGraph::vertex_property_type( "DKK" ), currencyGraph );

    boost::add_edge( eur, usd, currencyGraph );
    boost::add_edge( usd, rur, currencyGraph );
    boost::add_edge( eur, cad, currencyGraph );
    boost::add_edge( eur, dkk, currencyGraph );

    PathExplorer< InstrumentGraph > explorer( currencyGraph );
    std::vector< PathExplorer< InstrumentGraph >::Vertex > eur_rur = explorer.GetPath( eur, rur );
    std::copy( eur_rur.begin(), eur_rur.end(), std::ostream_iterator< PathExplorer< InstrumentGraph >::Vertex >( std::cout, "->" ) );
    std::cout << std::endl;

    std::vector< std::string > eur_rur_str;    
    VertexPathToStringPath( currencyGraph, eur_rur.begin(), eur_rur.end(), std::back_inserter( eur_rur_str ) );
    std::copy( eur_rur_str.begin(), eur_rur_str.end(), std::ostream_iterator< std::string >( std::cout, "->" ) );
    std::cout << std::endl;

    std::vector< PathExplorer< InstrumentGraph >::Vertex > rur_dkk = explorer.GetPath( rur, dkk );
    std::copy( rur_dkk.begin(), rur_dkk.end(), std::ostream_iterator< PathExplorer< InstrumentGraph >::Vertex >( std::cout, " " ) );
    std::cout << std::endl;

    std::vector< std::string > rur_dkk_str;
    VertexPathToStringPath( currencyGraph, rur_dkk.begin(), rur_dkk.end(), std::back_inserter( rur_dkk_str ) );
    std::copy( rur_dkk_str.begin(), rur_dkk_str.end(), std::ostream_iterator< std::string >( std::cout, "->" ) );
    std::cout << std::endl;

    // spliting instrument into directed
    const auto directedInstruments = SplitInstrument( { "EUR/USD", 1.3, 1.3 } );
    assert( directedInstruments.first.GetSourceCurrency() == "EUR" );
    assert( directedInstruments.first.GetDestCurrency() == "USD" );

    assert( directedInstruments.second.GetSourceCurrency() == "USD" );
    assert( directedInstruments.second.GetDestCurrency() == "EUR" );

    // money simple test

    Money money_eur( 100, "EUR" );
    const Money eur100( 100, "EUR" );
    money_eur += eur100;
    money_eur += 100;

    // Instrument dependency

    std::set< Instrument > instruments{ 
        { "EUR/USD", 1.3, 1.3 }
        , { "EUR/DM", 1.3, 1.3 }
        , { "USD/RUR", 30, 30 }
        , { "RUR/TUG", 100, 100 }
        , { "JPY/SCR", 100, 100 } };

    InstrumentDependency dep( instruments );
    const auto eur_rur_chain = dep.GetInstrumentChain( "EUR", "RUR" );
    const auto dm_rur_chain = dep.GetInstrumentChain( "DM", "RUR" );
    const auto rur_scr_chain = dep.GetInstrumentChain( "RUR", "SCR" );

    //////

    typedef std::pair <std::string, std::string> E;
    E edges[] = { E( "EUR", "USD" ), E( "USD", "RUR" ) };

    const char* vertices[] = { "EUR", "USD", "RUR" };
    std::map<std::string, mygraph::vertex_descriptor> indexes;

    const int nb_vertices = sizeof( vertices ) / sizeof( vertices[0] );

    mygraph g( nb_vertices );

    // fills the property 'vertex_name_t' of the vertices
    for (int i = 0; i < nb_vertices; i++)
    {
        boost::put( vertex_name_t(), g, i, vertices[i] ); // set the property of a vertex
        indexes[vertices[i]] = boost::vertex( i, g );     // retrives the associated vertex descriptor
    }

    // adds the edges
    // indexes[edges[0].first] maps "aaa" to the associated vertex index
    for (int i = 0; i < sizeof( edges ) / sizeof( edges[0] ); i++)
    {
        boost::add_edge( indexes[edges[i].first], indexes[edges[i].second], g );
    }

    /*add_edge( 0, 3, g );
    add_edge( 1, 2, g );
    add_edge( 2, 3, g );*/
    mygraph::vertex_iterator vertexIt, vertexEnd;
    mygraph::adjacency_iterator neighbourIt, neighbourEnd;
    tie( vertexIt, vertexEnd ) = boost::vertices( g );
    for (; vertexIt != vertexEnd; ++vertexIt)
    {
        std::cout << vertices[*vertexIt] << " is connected with ";
        tie( neighbourIt, neighbourEnd ) = adjacent_vertices( *vertexIt, g );
        for (; neighbourIt != neighbourEnd; ++neighbourIt)
            std::cout << vertices[*neighbourIt] << " ";
        std::cout << "\n";
    }

    typedef boost::adjacency_list
        <
        boost::vecS                                        //! edge list 
        , boost::vecS                                        //! vertex list
        , boost::undirectedS                                 //! undirected graph  
        , boost::property<boost::vertex_name_t, std::string> //! vertex properties : name                
        , boost::property<boost::edge_weight_t, int>         //! edge properties : weight 
        >   ScafGraph;

    //descriptors
    typedef boost::graph_traits<ScafGraph>::vertex_descriptor SV;
    typedef boost::graph_traits<ScafGraph>::edge_descriptor SE;

    //Graph Object
    ScafGraph SG;

    //property accessors
    boost::property_map<ScafGraph,
        boost::vertex_name_t>::type vertName = boost::get( boost::vertex_name, SG );
    boost::property_map<ScafGraph,
        boost::vertex_index_t>::type vertIndx = boost::get( boost::vertex_index, SG );
    boost::property_map<ScafGraph,
        boost::edge_weight_t>::type edgeWeight = boost::get( boost::edge_weight, SG );

    eur = boost::add_vertex( ScafGraph::vertex_property_type( "EUR" ), SG );
    usd = boost::add_vertex( ScafGraph::vertex_property_type( "USD" ), SG );
    rur = boost::add_vertex( ScafGraph::vertex_property_type( "RUR" ), SG );
    add_edge( eur, usd, SG );
    add_edge( usd, rur, SG );

    SV eur1 = boost::add_vertex( ScafGraph::vertex_property_type( "EUR" ), SG );
    SV usd1 = boost::add_vertex( ScafGraph::vertex_property_type( "USD" ), SG );
    add_edge( eur1, usd1, SG );

    ScafGraph::vertex_iterator SGvertexIt, SGvertexEnd;
    ScafGraph::adjacency_iterator SGneighbourIt, SGneighbourEnd;
    tie( SGvertexIt, SGvertexEnd ) = boost::vertices( SG );
    for (; SGvertexIt != SGvertexEnd; ++SGvertexIt)
    {
        std::cout << vertName[*SGvertexIt] << " is connected with ";
        tie( SGneighbourIt, SGneighbourEnd ) = adjacent_vertices( *SGvertexIt, SG );
        for (; SGneighbourIt != SGneighbourEnd; ++SGneighbourIt)
            std::cout << vertName[*SGneighbourIt] << " ";
        std::cout << "\n";
    }

    //Populate Graph
    /*std::vector<SV> svlist;
    for (int i = 0; i < 4; i++) {
        SV v = boost::add_vertex( ScafGraph::vertex_property_type( std::to_string( i ) ), SG );
        svlist.push_back( v );
        assert( vertName[v] == std::to_string( i ) );
        assert( vertIndx[v] == i );
    }*/

    return 0;
}


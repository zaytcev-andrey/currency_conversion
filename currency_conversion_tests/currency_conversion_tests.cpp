// currency_conversion_tests.cpp: определяет точку входа для консольного приложения.
//

#include "stdafx.h"

#include "currency_conversion/math_algorithms.h"
#include "currency_conversion/instrument_details.h"
#include "currency_conversion/instrument.h"
#include "currency_conversion/directed_instrument.h"
#include "currency_conversion/path_explorer.h"
#include "currency_conversion/graph_utils.h"
#include "currency_conversion/instrument_dependency.h"

#include <string>
#include <vector>
#include <set>
#include <iterator>
#include <algorithm>

#include "gtest/gtest.h"

using namespace currency_convertion;

int main( int argc, char **argv )
{
    ::testing::InitGoogleTest( &argc, argv );

    return RUN_ALL_TESTS();
}

namespace
{

class PathExplorerTest : public ::testing::Test
{
protected:
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

    PathExplorerTest()
    {
        eur = boost::add_vertex( InstrumentGraph::vertex_property_type( "EUR" ), currencyGraph );
        usd = boost::add_vertex( InstrumentGraph::vertex_property_type( "USD" ), currencyGraph );
        rur = boost::add_vertex( InstrumentGraph::vertex_property_type( "RUB" ), currencyGraph );
        cad = boost::add_vertex( InstrumentGraph::vertex_property_type( "CAD" ), currencyGraph );
        dkk = boost::add_vertex( InstrumentGraph::vertex_property_type( "DKK" ), currencyGraph );
        sgd = boost::add_vertex( InstrumentGraph::vertex_property_type( "SGD" ), currencyGraph );
        jpy = boost::add_vertex( InstrumentGraph::vertex_property_type( "JPY" ), currencyGraph );

        boost::add_edge( eur, usd, currencyGraph );
        boost::add_edge( eur, jpy, currencyGraph );
        boost::add_edge( usd, rur, currencyGraph );
        boost::add_edge( usd, jpy, currencyGraph );
        boost::add_edge( eur, cad, currencyGraph );
        boost::add_edge( eur, dkk, currencyGraph );
        boost::add_edge( usd, cad, currencyGraph );
    }

    InstrumentGraph currencyGraph;
    Vertex eur;
    Vertex usd;
    Vertex rur;
    Vertex cad;
    Vertex dkk;
    Vertex sgd;
    Vertex jpy;
};

}

TEST( MathTest, FloatingpPointCompare )
{
    const auto valueFirst = 1.0001;
    const auto valueSecond = 1.0002;
    EXPECT_FALSE( isEqual( valueFirst, valueSecond ) );

    const auto valueFirstInt = 1;
    const auto valueSecondDouble = 1.0;
    EXPECT_TRUE( isEqual( valueFirstInt, valueSecondDouble ) );

    const auto valueFirstDouble = 1.0;
    const auto valueSecondInt = 1;
    EXPECT_TRUE( isEqual( valueFirstDouble, valueSecondInt ) );
}

TEST( MathTest, FloatingpPointRoundedCompare )
{
    EXPECT_FALSE( isEqualRounded( 1.0001, 1.0002, 5 ) );
    EXPECT_FALSE( isEqualRounded( 1.00001, 1.00002, 5 ) );
    EXPECT_TRUE( isEqualRounded( 1.000001, 1.000002, 5 ) );

    EXPECT_TRUE( isEqualRounded( 1, 1.000002, 5 ) );
    EXPECT_FALSE( isEqualRounded( 1, 1.00002, 5 ) );

    EXPECT_TRUE( isEqualRounded( 1.000002, 1, 5 ) );
    EXPECT_FALSE( isEqualRounded( 1.00002, 1, 5 ) );
}

TEST( InstrumentTest, NameExtracting )
{
    const auto base = details::ExtractFirstCurrency( "EUR/USD" );
    EXPECT_STREQ( base.c_str(), "EUR" );

    const auto qouted = details::ExtractSecondCurrency( "EUR/USD" );
    EXPECT_STREQ( qouted.c_str(), "USD" );
}

TEST( InstrumentTest, InstrumentCreating )
{
    const Instrument instrument( "EUR/USD", 1.3, 1.3 );
    EXPECT_STREQ( instrument.GetName().c_str(), "EUR/USD" );
    EXPECT_STREQ( instrument.GetBaseCurrency().c_str(), "EUR" );
    EXPECT_STREQ( instrument.GetQoutedCurrency().c_str(), "USD" );
}

TEST( DirectedInstrumentTest, InstrumentCreating )
{
    const Instrument instrument( "EUR/USD", 1.3, 1.3 );
    EXPECT_STREQ( instrument.GetName().c_str(), "EUR/USD" );
    EXPECT_STREQ( instrument.GetBaseCurrency().c_str(), "EUR" );
    EXPECT_STREQ( instrument.GetQoutedCurrency().c_str(), "USD" );
}

TEST( InstrumentTest, SplitingIntoDirected )
{
    const auto directedInstruments = SplitInstrument( Instrument{ "EUR/USD", 1.3, 1.33 } );
    EXPECT_STREQ( directedInstruments.first.GetSourceCurrency().c_str(), "EUR" );
    EXPECT_STREQ( directedInstruments.first.GetDestCurrency().c_str(), "USD" );
    EXPECT_TRUE( isEqual( directedInstruments.first.GetRatio(), 1.3 ) );

    EXPECT_STREQ( directedInstruments.second.GetSourceCurrency().c_str(), "USD" );
    EXPECT_STREQ( directedInstruments.second.GetDestCurrency().c_str(), "EUR" );
    EXPECT_TRUE( isEqualRounded( directedInstruments.second.GetRatio(), 1 / 1.33, 5 ) );
}

TEST_F( PathExplorerTest, FindingPath )
{
    PathExplorer< InstrumentGraph > explorer( currencyGraph );

    const auto eurToRur = explorer.GetPath( eur, rur );
    std::vector< std::string > eurToRurRef{ "EUR", "USD", "RUB" };
    EXPECT_TRUE( std::equal( eurToRur.begin(), eurToRur.end(), eurToRurRef.begin(), [&]
        ( const PathExplorer< InstrumentGraph >::Vertex& v, const std::string& name )
    {
        return details::VertexToString( currencyGraph, v ) == name;
    } ) );

    const auto rurToDkk = explorer.GetPath( rur, dkk );
    std::vector< std::string > rurToDkkRef{ "RUB", "USD", "EUR", "DKK" };
    EXPECT_TRUE( std::equal( rurToDkk.begin(), rurToDkk.end(), rurToDkkRef.begin(), [&]
        ( const PathExplorer< InstrumentGraph >::Vertex& v, const std::string& name )
    {
        return details::VertexToString( currencyGraph, v ) == name;
    } ) );
}

TEST_F( PathExplorerTest, FindingInMultiPath )
{
    PathExplorer< InstrumentGraph > explorer( currencyGraph );

    const auto cadToJpy = explorer.GetPath( cad, jpy );

    std::vector< std::string > cadToJpyRef{ "CAD", "EUR", "JPY" };
    EXPECT_TRUE( std::equal( cadToJpy.begin(), cadToJpy.end(), cadToJpyRef.begin(), [&]
        ( const PathExplorer< InstrumentGraph >::Vertex& v, const std::string& name )
    {
        return details::VertexToString( currencyGraph, v ) == name;
    } ) );
}

TEST_F( PathExplorerTest, PathDoesNotExist )
{
    PathExplorer< InstrumentGraph > explorer( currencyGraph );

    const auto usdToSgd = explorer.GetPath( usd, sgd );
    EXPECT_TRUE( usdToSgd.empty() );
}

TEST_F( PathExplorerTest, FindingPathAndConvertionToString )
{
    PathExplorer< InstrumentGraph > explorer( currencyGraph );
    const auto eurToRur = explorer.GetPath( eur, rur );

    std::vector< std::string > eurToRurStr;
    details::VertexPathToStringPath( currencyGraph, eurToRur.begin(), eurToRur.end(), std::back_inserter( eurToRurStr ) );

    std::vector< std::string > eurToRurRef{ "EUR", "USD", "RUB" };
    EXPECT_TRUE( std::equal( eurToRurStr.begin(), eurToRurStr.end(), eurToRurRef.begin() ) );
}

TEST( InstrumentDependencyTest, GetInstrumentChain )
{
    std::set< Instrument > instruments{
        { "EUR/USD", 1.13300, 1.33302 }
        , { "EUR/DM", 1.3, 1.3 }
        , { "USD/RUB", 30, 30 }
        , { "RUB/TUG", 100, 100 }
        , { "JPY/SCR", 100, 100 } };

    InstrumentDependency dep( instruments );

    const auto eurRurChain = dep.GetInstrumentChain( "EUR", "RUB" );
    std::vector< std::string > eurRurChainRef{ "EUR/USD", "USD/RUB" };
    EXPECT_TRUE( std::equal( eurRurChain.begin(), eurRurChain.end(), eurRurChainRef.begin(), [&]
        ( const InstrumentDependency::InstrumentChain::value_type& item, const std::string& name )
    {
        return item.GetName() == name;
    } ) );

    const auto dmRurChain = dep.GetInstrumentChain( "DM", "RUB" );
    std::vector< std::string > dmRurChainRef{ "DM/EUR", "EUR/USD", "USD/RUB" };
    EXPECT_TRUE( std::equal( dmRurChain.begin(), dmRurChain.end(), dmRurChainRef.begin(), [&]
        ( const InstrumentDependency::InstrumentChain::value_type& item, const std::string& name )
    {
        return item.GetName() == name;
    } ) );

    const auto rurScrChain = dep.GetInstrumentChain( "RUB", "SCR" );
    EXPECT_TRUE( rurScrChain.empty() );
}

TEST( InstrumentDependencyTest, FindingInMultiPath )
{
    std::set< Instrument > instruments{
        { "EUR/USD", 1.13300, 1.33302 }
        ,{ "EUR/JPY", 1.13300, 1.33302 }
        ,{ "USD/JPY", 1.13300, 1.33302 }
        ,{ "EUR/CAD", 1.13300, 1.33302 }
        ,{ "USD/CAD", 1.13300, 1.33302 }
    };

    InstrumentDependency dep( instruments );

    const auto cadJpyChain = dep.GetInstrumentChain( "CAD", "JPY" );
    std::vector< std::string > cadJpyChainRef{ "CAD/EUR", "EUR/JPY" };
    EXPECT_TRUE( std::equal( cadJpyChain.begin(), cadJpyChain.end(), cadJpyChainRef.begin(), [&]
        ( const InstrumentDependency::InstrumentChain::value_type& item, const std::string& name )
    {
        return item.GetName() == name;
    } ) );
}

TEST( InstrumentDependencyTest, FakeInstrumentToItself )
{
    std::set< Instrument > instruments{
        { "EUR/USD", 1.13300, 1.33302 }
        ,{ "EUR/JPY", 1.13300, 1.33302 }
        ,{ "USD/JPY", 1.13300, 1.33302 }
        ,{ "EUR/CAD", 1.13300, 1.33302 }
        ,{ "USD/CAD", 1.13300, 1.33302 }
    };

    InstrumentDependency dep( instruments );
    const auto eurRurChain = dep.GetInstrumentChain( "EUR", "EUR" );

    std::vector< std::string > eurRurChainRef{ "EUR/EUR" };
    EXPECT_TRUE( std::equal( eurRurChain.begin(), eurRurChain.end(), eurRurChainRef.begin(), [&]
        ( const InstrumentDependency::InstrumentChain::value_type& item, const std::string& name )
    {
        return item.GetName() == name;
    } ) );
}
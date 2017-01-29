#include "stdafx.h"

#include "instrument.h"
#include "money.h"
#include "converter.h"

#include <string>
#include <iostream>
#include <utility>
#include <exception>

#include "boost/assert.hpp"

int main()
{
    using namespace currency_convertion;

    try
    {
        // Test: No convertion chain found
        {
            std::set< Instrument > instruments{
                { "EUR/USD", 1.13300, 1.33302 }
                , { "EUR/DM", 1.8, 1.83 }
                , { "USD/RUB", 59.872, 59.882 }
                , { "RUB/DKK", 100, 102 }
                , { "JPY/SCR", 100, 102 }
                , { "SGD/JPY", 80.26, 80.51 }
                , { "CAD/JPY", 87.50, 87.53 } };

            auto noPathConverter = std::make_shared< Converter >( instruments );

            try
            {
                Money rur( 1, "RUB" );
                auto scr = noPathConverter->MakeMoney( rur, "SCR" );
            }
            catch (const Converter::Exception& e)
            {
                std::cout << e.what() << std::endl;
            }

            try
            {
                auto jpy = noPathConverter->MakeMoney( 1000, "JPY" );
                auto scr = noPathConverter->MakeMoney( 2000, "SCR" );
                Money eur( 0, "EUR" );
                eur = jpy + scr;
            }
            catch (const Converter::Exception& e)
            {
                std::cout << e.what() << std::endl;
            }
        }

        std::set< Instrument > instrumentsForTestMoney{
            { "EUR/USD", 1.13300, 1.33302 }
            ,{ "USD/RUB", 59.872, 59.882 }
            ,{ "USD/CAD", 3.7699, 3.777 }
            ,{ "EUR/JPY", 123.12, 123.15 }
            ,{ "EUR/GBP", 0.852, 0.8521 }
            ,{ "GBP/ILS", 4.7316, 4.7354 }
            ,{ "CAD/JPY", 87.50, 87.53 }
            ,{ "USD/JPY", 115.09, 115.10 }
            ,{ "EUR/SGD", 1.5305, 1.5324 }
            ,{ "USD/SGD", 1.4305, 1.433 }
            ,{ "SGD/JPY", 80.26, 80.51 }
            ,{ "EUR/CHF", 1.0691, 1.0695 }
            ,{ "USD/CHF", 0.9993, 0.9996 }
            ,{ "USD/HKD", 7.7579, 7.7608 }
            ,{ "USD/ILS", 3.7699, 3.777 }
        };

        auto converter = std::make_shared< Converter >( instrumentsForTestMoney );

        // Test: money simple operation
        {
            Money moneyEur( 100, "EUR" );
            const Money eur100( 100, "EUR" );
            moneyEur += eur100;
            moneyEur += 100;
            BOOST_ASSERT( moneyEur == 300 );
        }

        // Test: money convertion with direct link in path
        {
            Money eur( 10, "EUR", converter );
            auto usd = converter->MakeMoney( eur, "USD" );
            BOOST_ASSERT( usd == 11.3300 );

            eur += usd;
            BOOST_ASSERT( eur == 18.49950 );

            eur = Money( -10, "EUR", converter );
            usd = converter->MakeMoney( eur, "USD" );
            BOOST_ASSERT( usd == -11.3300 );

            eur += usd;
            BOOST_ASSERT( eur == -18.49950 );
        }

        // Test: money convertion without direct link in path
        {
            auto rur = converter->MakeMoney( 10, "RUB" );
            rur += 200;
            BOOST_ASSERT( rur == 210 );

            auto jpy = converter->MakeMoney( 1000, "JPY" );
            rur += jpy;
            BOOST_ASSERT( rur == 730.17376 );

            rur += Money( 200, "USD" ) + converter->MakeMoney( 200, "SGD" );
            BOOST_ASSERT( rur == 21060.74962 );

            Money ils( 10, "ILS" );
            auto eur = converter->MakeMoney( ils, "EUR" );
            BOOST_ASSERT( eur == 2.47829 );

            rur = converter->MakeMoney( 10000.0, "RUB" );
            jpy = converter->MakeMoney( rur, "JPY" );
            BOOST_ASSERT( jpy == 19219.46495 );
            jpy -= 100;
            BOOST_ASSERT( jpy == 19119.46495 );
            rur = jpy;
            BOOST_ASSERT( rur == 9945.44401 );
            rur += Money( 200, "USD" ) + converter->MakeMoney( 200, "SGD" );
            BOOST_ASSERT( rur == 30276.01986 );
            rur += converter->MakeMoney( 100, "EUR" );
            BOOST_ASSERT( rur == 37059.51746 );
            rur -= converter->MakeMoney( 500, "CAD" );
            BOOST_ASSERT( rur == 29133.65037 );
        }

        // Test: new qoute
        {
            Money eur( 10, "EUR", converter );
            auto usd = converter->MakeMoney( eur, "USD" );
            BOOST_ASSERT( usd == 11.3300 );

            eur += usd;
            BOOST_ASSERT( eur == 18.49950 );

            converter->NewQuote( "EUR/USD", 1.13305, 1.33307 );

            eur = Money( 10, "EUR", converter );
            usd = converter->MakeMoney( eur, "USD" );
            BOOST_ASSERT( usd == 11.3305 );

            eur += usd;
            BOOST_ASSERT( eur == 18.49955 );

            converter->NewQuote( "GBP/ILS", 4.7500, 4.7510 );
            Money ils( 10, "ILS" );
            eur = converter->MakeMoney( ils, "EUR" );
            BOOST_ASSERT( eur == 2.47016 );
        }
    }
    catch ( const Converter::Exception& err )
    {
        std::cout << "Convertion error occurred: " << err.what() << std::endl;
    }
    catch ( const std::exception& err )
    {
        std::cout << "runtime error occurred: " << err.what() << std::endl;
    }

    return 0;
}


#pragma once

#include <cmath>
#include <math.h>
#include <type_traits>

namespace currency_convertion
{

template<class T>
typename std::enable_if<!std::is_integral< T >::value, bool>::type
isEqual( T lhs, T rhs, int ulp = 1 )
{
    const auto diff = std::abs( lhs - rhs );
    const auto diff_max = std::numeric_limits<T>::epsilon()
        * std::max( std::abs( lhs ), std::abs( rhs ) )
        * ulp;
    return diff <= diff_max;
}

template<class T, class U>
typename std::enable_if<std::is_integral< T >::value && std::is_floating_point<U>::value, bool>::type
isEqual( T lhs, U rhs, int ulp = 1 )
{
    return isEqual( static_cast< U >( lhs ), rhs, ulp );
}

template<class T, class U>
typename std::enable_if<std::is_floating_point<T>::value && std::is_integral<U>::value, bool>::type
isEqual( T lhs, U rhs, int ulp = 1 )
{
    return isEqual< U, T >( rhs, lhs, ulp );
}

template<class T, class U>
typename std::enable_if< std::is_integral<T>::value && std::is_integral<U>::value, bool>::type
isEqual( T lhs, U rhs )
{
    return lhs == rhs;
}

template<class T>
typename std::enable_if<!std::is_integral< T >::value, bool>::type
isEqualRounded( T lhs, T rhs, int precision )
{
    const auto precisionValue = pow( 10, precision );
    
    lhs = std::trunc( lhs * precisionValue + 0.5 ) / precisionValue;
    rhs = std::trunc( rhs * precisionValue + 0.5 ) / precisionValue;

    return isEqual( lhs, rhs );
}

template<class T, class U>
typename std::enable_if<std::is_integral< T >::value && std::is_floating_point<U>::value, bool>::type
isEqualRounded( T lhs, U rhs, int precision )
{
    return isEqualRounded( static_cast< U >( lhs ), rhs, precision );
}

template<class T, class U>
typename std::enable_if<std::is_floating_point<T>::value && std::is_integral<U>::value, bool>::type
isEqualRounded( T lhs, U rhs, int precision )
{
    return isEqualRounded< U, T >( rhs, lhs, precision );
}

template<class T, class U>
typename std::enable_if< std::is_integral<T>::value && std::is_integral<U>::value, bool>::type
isEqualRounded( T lhs, U rhs )
{
    return lhs == rhs;
}

}
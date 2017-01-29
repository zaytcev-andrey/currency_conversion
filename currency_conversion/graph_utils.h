#pragma once

#include <algorithm>

#include <boost/graph/adjacency_list.hpp>

namespace currency_convertion
{
namespace details
{

template < class Graph > inline
std::string VertexToString( const Graph& grap
    , typename boost::graph_traits< Graph >::vertex_descriptor v )
{
        const auto vertexName = boost::get( boost::vertex_name, grap );

        return vertexName[ v ];
}

template < class Graph, class InItr, class OutItr > inline
void VertexPathToStringPath( const Graph& grap, InItr first, InItr last, OutItr dest )
{
    const auto vertexName = boost::get( boost::vertex_name, grap );

    std::transform( first, last, dest, [&]( const typename InItr::value_type& v )
    { return vertexName[ v ]; } );
}

}
}
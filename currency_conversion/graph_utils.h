#pragma once

#include <algorithm>

#include <boost/graph/adjacency_list.hpp>

template < class Graph, class InItr, class OutItr > inline
void VertexPathToStringPath( const Graph& grap, InItr first, InItr last, OutItr dest )
{
    const auto vertexName = boost::get( boost::vertex_name, grap );

    std::transform( first, last, dest, [&]( const typename InItr::value_type& v )
    { return vertexName[ v ]; } );
}
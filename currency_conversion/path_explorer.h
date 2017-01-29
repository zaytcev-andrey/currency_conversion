#pragma once

#include <vector>
#include <algorithm>
#include <functional>
#include <exception>
#include <assert.h>

#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/breadth_first_search.hpp>
#include <boost/graph/named_function_params.hpp>
#include <boost/graph/visitors.hpp>
#include <boost/array.hpp>

namespace currency_convertion
{
namespace details
{ 

template <class PredecessorMap, class Tag>
class PathRecorder : public boost::predecessor_recorder< PredecessorMap, Tag >
{
public:

    typedef Tag event_filter;
    PathRecorder( PredecessorMap pa, std::vector< bool >& marked )
        : predecessor_recorder( pa )
        , marked_( marked )
    {
    }

    template <class Edge, class Graph>
    void operator()( Edge e, const Graph& g ) {
        boost::predecessor_recorder< PredecessorMap, Tag >::operator()< Edge, Graph >(e, g);
        marked_.get()[ target( e, g ) ] = true;
    }
private:
    boost::reference_wrapper< std::vector< bool > > marked_;
};

template <class PredecessorMap, class Tag>
PathRecorder<PredecessorMap, Tag>
MakePathRecorder( PredecessorMap pa, Tag, std::vector< bool >& marked ) {
    return PathRecorder<PredecessorMap, Tag>( pa, marked );
}

}

template < class Graph >
class PathExplorer
{
public:
    typedef typename boost::graph_traits< Graph >::vertex_descriptor Vertex;
    typedef typename Graph::vertex_iterator vertexIterator;

    explicit PathExplorer( const Graph& graph )
    {
        const auto vertex_count = boost::num_vertices( graph );

        predecessors_.assign( vertex_count, std::vector< Vertex >( vertex_count ) );
        marked_.assign( vertex_count, std::vector< bool >( vertex_count ) );

        CalculatePath( graph );
    }


    std::vector< Vertex > GetPath( Vertex vertexFrom, Vertex vertexTo ) const
    {
        if ( !hasPath( vertexFrom, vertexTo ) )
        {
            std::cout << "does not have path v" << vertexFrom << " <-> " << "v" << vertexTo << '\n';
            return std::vector< Vertex >();
        }

        std::vector< Vertex > path_to_vertex;
        Vertex p = vertexTo;
        while ( p != vertexFrom )
        {
            path_to_vertex.push_back( p );
            // std::cout << p << '\n';
            p = predecessors_[ vertexFrom ][p];
        }
        // std::cout << p << '\n';
        path_to_vertex.push_back( p );
        std::reverse( path_to_vertex.begin(), path_to_vertex.end() );

        return path_to_vertex;
    }
private:

    void CalculatePath( const Graph& graph_ )
    {
        const auto vertex_count = boost::num_vertices( graph_ );
        auto vs = boost::vertices( graph_ );

        std::for_each( vs.first, vs.second, [=]( const Vertex& v )
        {
            boost::breadth_first_search( graph_, v,
                boost::visitor(
                    boost::make_bfs_visitor(
                        details::MakePathRecorder( &predecessors_[ v ][ 0 ],
                            boost::on_tree_edge(), marked_[ v ] ) ) ) );
        } );
    }

    bool hasPath( Vertex vertexFrom, Vertex vertexTo ) const
    {
        return marked_[ vertexFrom ][ vertexTo ];
    }

private:
    std::vector< std::vector< typename boost::graph_traits< Graph >::vertex_descriptor > > predecessors_;
    std::vector< std::vector< bool > > marked_;
};

}
// TestConsole.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#define SQLITE_ENABLE_COLUMN_METADATA
#define SQLITE_ENABLE_RTREE

#include <windows.h>
#include <process.h>
#include <memory.h>
#include <typeinfo>

#include <assert.h>
#include <sstream>
#include <fstream>
#include <exception>
#include <vector>
#include <locale>
#include <deque>
#include <stack>
#include <map>
#include <typeinfo>
//#include <soci/soci.h>
//#include <soci/soci-sqlite3.h>
#include <boost/array.hpp>
#define BOOST_FILESYSTEM_DEPRECATED
#pragma push_macro( "BOOST_FILESYSTEM_VERSION" )
#define BOOST_FILESYSTEM_VERSION 3
#include <boost/filesystem.hpp>
#include <boost/system/windows_error.hpp>
#undef BOOST_FILESYSTEM_VERSION
#pragma pop_macro( "BOOST_FILESYSTEM_VERSION" )
#include <boost/spirit/home/support/utf8.hpp>
#include <boost/format.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/info_parser.hpp>
#include <boost/filesystem/detail/utf8_codecvt_facet.hpp>
#include <boost/scoped_ptr.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/property_tree/xml_parser.hpp>
#include <boost/asio/ip/address.hpp>
#include <boost/asio.hpp>
#include <boost/noncopyable.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/thread.hpp>
#include <boost/function.hpp>
#include <boost/optional.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/scoped_array.hpp>
#include <boost/assign/list_of.hpp>
#include <boost/lambda/lambda.hpp>
#include <boost/chrono.hpp>
#include <boost/algorithm/string.hpp>

#include <boost/range/as_array.hpp>

#include <boost/detail/workaround.hpp>

#include <boost/range/iterator_range.hpp>
#include <boost/range/functions.hpp>
#include <boost/range/as_literal.hpp>
#include <boost/test/test_tools.hpp>
#include <boost/test/unit_test.hpp>
#include <boost/xpressive/xpressive.hpp>
#include <boost/typeof/typeof.hpp>

#include <boost/interprocess/managed_shared_memory.hpp> 
#include <boost/interprocess/sync/named_mutex.hpp> 
#include <boost/interprocess/sync/named_condition.hpp> 
#include <boost/interprocess/sync/scoped_lock.hpp>
#include <boost/interprocess/sync/file_lock.hpp>
#include <boost/signals2.hpp>
#include <boost/variant.hpp>
#include <boost/any.hpp>
#include <boost/bind.hpp>
#include <boost/function.hpp>
#include <boost/assign/std/vector.hpp>
#include <boost/make_shared.hpp>
#include <boost/optional.hpp>
#include <boost/chrono/chrono.hpp>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/breadth_first_search.hpp>
#include <boost/graph/named_function_params.hpp>
#include <boost/graph/visitors.hpp>
#include <boost/array.hpp>

#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <set>
#include <assert.h>
#include <functional>
#include <exception>


#include "utility.h"
#include "common_header.h"
#include "request_parser.h"

#define DECLARE_EVENT( EventName, id ) struct EventName { static const int nativeId = id; };

enum EVENT_TYPE { foo, bar, foobar };

DECLARE_EVENT( FooEvent, 0 )
DECLARE_EVENT( BarEvent, 1 )
DECLARE_EVENT( FooBarEvent, 3 )

struct HelloWorld
{
     void OnHandler( const FooEvent& )
     {
          std::cout << "Hello, FooEvent!" << std::endl;
     }

     void OnHandler( const BarEvent& )
     {
          std::cout << "Hello, BarEvent!" << std::endl;
     }
     
     void operator()() const
     {
          std::cout << "Hello, World!" << std::endl;
     }
};

class Logic
{

};

template< class Event, class Host >
boost::shared_ptr<
boost::signals2::signal<void (const Event&)> > create_signal_impl( Host* host )
{
     boost::shared_ptr<
          boost::signals2::signal<void (const Event&)> > sig;
     typedef void (Host::*Function)( const Event& );
     sig->connect( boost::bind( static_cast<Function>( &Host::OnHandler ), host, _1 ) );

     return sig;
};

template< class Event, class Host >
void create_signal( Host* host )
{
     create_signal_impl< Event >( host );
};

template< class Event1, class Event2, class Host >
void create_signal( Host* host )
{
     create_signal_impl< Event1 >( host );
     create_signal_impl< Event2 >( host );
};


class Event
{
public:

     Event()
          : fn_()
          , nativeId_()
     {}
     
     template< class E >
     Event( boost::function< void ( const E& ) > fn )
          : fn_( fn )
          , nativeId_( E::nativeId )
     {
     }

     template< class E >
     void operator() ( const E& evt ) const
     {
          if ( fn_.empty() )
          {
               return;
          }

          try
          {               
               return boost::any_cast< boost::function< void ( const E& ) > >( fn_ )( evt );
          }
          catch( const boost::bad_any_cast& )
          {
          }
     }

     template< class E >
     bool is_class_of() const
     {
          return fn_.type() == typeid( boost::function< void ( const E& ) > );
     }

     int get_native_id() const
     {
          return nativeId_;
     }

private:
     boost::any fn_;
     int nativeId_;
};

template< class E, class Host > Event MakeEvent( Host* host )
{
     using boost::bind;

     typedef void ( Host::*EventFunction )( const E& );
     boost::function< void ( const E& ) > fn( boost::bind(
          static_cast< EventFunction >( &Host::OnHandler ), host, _1 ) );

     Event event( fn );

     return event;
}

template< class E, class Host > inline
boost::function< void( const E& ) > MakeEventHandler( Host* host )
{
     using boost::bind;

     typedef void ( Host::*EventFunction )( const E& );
     boost::function< void ( const E& ) > fn( boost::bind(
          static_cast< EventFunction >( &Host::OnHandler ), host, _1 ) );

     return fn;
}

class EventHandlersStorage
{
public:
     template< size_t N >
     EventHandlersStorage( const Event ( & events )[ N ] )
          : events_()
     {
          BOOST_FOREACH( const Event& evt, events )
          {
               events_.push_back( evt );
          }
     }

     template< class E >
     void Invoke( const E& event ) const
     {
          using namespace boost::lambda;

          std::vector< Event >::const_iterator it_handler =
               std::find_if( events_.begin(), events_.end()
               , boost::bind( &Event::is_class_of< E >, boost::lambda::_1 ) == true );

          if ( it_handler != events_.end() )
          {
               (*it_handler)( event );
          }
     }

private:
     std::vector< Event > events_;
};

template< class E, class Host > EventHandlersStorage MakeEventHadlers( Host* host )
{
     Event events[] = {
          MakeEvent< E >( host )
     };

     return EventHandlersStorage( events );
}

template< class E1, class E2, class Host > EventHandlersStorage MakeEventHadlers( Host* host )
{
     Event events[] = {
          MakeEvent< E1 >( host ),
          MakeEvent< E2 >( host )
     };

     return EventHandlersStorage( events ); 
}

template< class E1, class E2, class E3, class Host > EventHandlersStorage MakeEventHadlers( Host* host )
{
     Event events[] = {
          MakeEvent< E1 >( host ),
          MakeEvent< E2 >( host ),
          MakeEvent< E3 >( host )
     };

     return EventHandlersStorage( events );
}

class Subscription
{
public:
     template< size_t N >
     Subscription( Event ( & const events )[ N ] )
          : events_()
     {
          BOOST_FOREACH( Event& evt, events )
          {
               events_.insert( std::make_pair( evt.get_native_id(), evt ) );
          }
     }

     void CallEvent( EVENT_TYPE event )
     {
          switch( event )
          {
          case foo:
               {
                    FooEvent evt_foo;
                    events_[ FooEvent::nativeId ]( evt_foo );
               }
               break;
          case bar:
               {
                    BarEvent evt_bar;
                    events_[ FooEvent::nativeId ]( evt_bar );
               }
               break;
          }
     }

private:
     std::map< int, Event > events_;
};

template< class E, class Host > Subscription CreateSubscription( Host* host )
{
     Event events[] = {
          MakeEvent< E >( host )
     };

     Subscription sub( events );

     return sub;
}

template< class E1, class E2, class Host > Subscription CreateSubscription( Host* host )
{
     Event events[] = {
          MakeEvent< E1 >( host ),
          MakeEvent< E2 >( host )
     };

     Subscription sub( events );

     return sub;
}

class ICommonResponseUpdate
{
public:
     typedef boost::shared_ptr< ICommonResponseUpdate > Ptr; 

     virtual void UpdateSuccessInfo(unsigned long updateType) const = 0;

     virtual ~ICommonResponseUpdate() {};
};

class ConcreteResponseUpdate : public ICommonResponseUpdate
{
public:
     ConcreteResponseUpdate( const EventHandlersStorage& callbacks )
          : callbacks_( callbacks )
     {
     }

     virtual void UpdateSuccessInfo(unsigned long updateType) const
     {
          if ( updateType == foo )
          {
               callbacks_.Invoke( FooEvent() );
          }

          if ( updateType == foo )
          {
               callbacks_.Invoke( BarEvent() );
          }

          if ( updateType == foobar )
          {
               callbacks_.Invoke( FooBarEvent() );
          }
     }  

private:
     EventHandlersStorage callbacks_;
};

class IUpdateSystemConnection 
{
public:
     typedef boost::shared_ptr< IUpdateSystemConnection > Ptr;

     virtual ~IUpdateSystemConnection() {};
};

class ConcreteUpdateSystemConnection : public IUpdateSystemConnection
{
public:
     ConcreteUpdateSystemConnection( ICommonResponseUpdate::Ptr )
     {}
};


class UpdateSystemConnector
{
public:
     template< class E, class Host > 
     IUpdateSystemConnection::Ptr CreateSubscription( Host* host )
     {
          EventHandlersStorage callbacks = MakeEventHadlers< E >( host );
          return CreateConnection( callbacks );
     }

     template< class E1, class E2, class Host > 
     IUpdateSystemConnection::Ptr CreateSubscription( Host* host )
     {
          EventHandlersStorage callbacks = MakeEventHadlers< E1, E2 >( host );
          return CreateConnection( callbacks );
     }

     template< class E1, class E2, class E3, class Host > 
     IUpdateSystemConnection::Ptr CreateSubscription( Host* host )
     {
          EventHandlersStorage callbacks = MakeEventHadlers< E1, E2, E3 >( host );
          return CreateConnection( callbacks );
     }
protected:

     virtual IUpdateSystemConnection::Ptr CreateConnection( const EventHandlersStorage& callbacks ) const
     {
          ICommonResponseUpdate::Ptr response( new ConcreteResponseUpdate( callbacks ) );
          return IUpdateSystemConnection::Ptr( new ConcreteUpdateSystemConnection( response ) );
     }
};

class ManWin 
{
public:
     ManWin( UpdateSystemConnector* ptr )
     {
          ptr->CreateSubscription<FooEvent>( this );
     }

     void OnHandler( const FooEvent& )
     {
          std::cout << "Hello, FooEvent!" << std::endl;
     }
};

class ManProcessing 
{
public:
     ManProcessing( UpdateSystemConnector* ptr )
     {
          ptr->CreateSubscription< FooEvent, BarEvent >( this );
     }

     void OnHandler( const FooEvent& )
     {
          std::cout << "Hello, FooEvent!" << std::endl;
     }

     void OnHandler( const BarEvent& )
     {
          std::cout << "Hello, BarEvent!" << std::endl;
     }
};

class CallTimeMeasure
{
public:
     
     CallTimeMeasure()
     {          
          ::memset( &freq_, 0, sizeof( LARGE_INTEGER ) );

          QueryPerformanceFrequency( &freq_ );
     }

     void StartMeasure()
     {
          ::memset( &start_counter_, 0, sizeof( LARGE_INTEGER ) );
          ::memset( &stop_counter_, 0, sizeof( LARGE_INTEGER ) );
          QueryPerformanceCounter( &start_counter_ );
     }

     void StopMeasure()
     {
          QueryPerformanceCounter( &stop_counter_ );
     }

     double GetCallTime() const
     {
          const double diff = stop_counter_.QuadPart - start_counter_.QuadPart;
          double time = diff / freq_.QuadPart;

          return time;
     }

private:
     LARGE_INTEGER freq_;
     LARGE_INTEGER start_counter_;
     LARGE_INTEGER stop_counter_;
};

class RaiiService
{
public:
     RaiiService( SC_HANDLE service, const std::string& name )
          : service_( service ), name_( name )
          , activity_event_name_( "Global\\MFTPMainServiceEvent" )
     {          
     }

     void Start()
     {
          std::cout << "Service \"" << name_ << "\" start pending..." << std::endl;

          const BOOL status = ::StartService( service_, 0, NULL );
          
          if ( !status )               
          {
               std::ostringstream strm;
               strm << "can not start service \"" << name_ << "\": error " << GetLastError();
               throw std::exception( strm.str().c_str() );               
          }                   
     }

     void WaitForMftp2() const
     {
          HANDLE activity_event = 
               ::CreateEvent( NULL, TRUE, FALSE, activity_event_name_.c_str() );

          if ( activity_event == NULL )
          {
               std::ostringstream strm;
               strm << "can not create event \"" << activity_event_name_ << "\": error " << GetLastError();
               throw std::exception( strm.str().c_str() );
          }

          ::WaitForSingleObject( activity_event, INFINITE );

          std::cout << "Service mftp2 successfully started." << std::endl;
     }

     void Stop()
     {          
          DWORD dwStartTime = GetTickCount();
          DWORD dwTimeout = 30000; // 30-second time-out
          SERVICE_STATUS_PROCESS ssp = {};

          {
               ssp = GetServiceStatus();

               if ( ssp.dwCurrentState == SERVICE_STOPPED )
               {
                    std::cout << "Service \"" << name_ << "\" already stopped." << std::endl;
                    return;
               }
                        
               std::cout << "Service \"" << name_ << "\" stop pending..." << std::endl;

               while ( ssp.dwCurrentState == SERVICE_STOP_PENDING ) 
               {
                    Sleep( ssp.dwWaitHint );
                    
                    ssp = GetServiceStatus();
                    if ( ssp.dwCurrentState == SERVICE_STOPPED )
                    {
                         std::cout << "Service \"" << name_ << "\" already stopped." << std::endl;
                         return;
                    }

                    if ( GetTickCount() - dwStartTime > dwTimeout )
                    {
                         std::cout << "Service \"" << name_ << "\" stop timed out." << std::endl;
                         return;
                    }
               }
          }          

          const BOOL stop_status = ControlService( 
               service_, 
               SERVICE_CONTROL_STOP, 
               (LPSERVICE_STATUS)&ssp );

          if ( !stop_status )
          {
               std::ostringstream strm;
               strm << "error call ControlService for service \"" << name_ << "\": error " << GetLastError();
               throw std::exception( strm.str().c_str() );
          }
          
          while ( ssp.dwCurrentState != SERVICE_STOPPED ) 
          {
               Sleep( ssp.dwWaitHint );

               ssp = GetServiceStatus();
               if ( ssp.dwCurrentState == SERVICE_STOPPED )
               {                    
                    break;
               }

               if ( GetTickCount() - dwStartTime > dwTimeout )
               {
                    std::cout << "Service \"" << name_ << "\" stop timed out." << std::endl;
                    return;
               }          
          }
          
          std::cout << "Service \"" << name_ << "\" stopped successfully." << std::endl;
     }

     ~RaiiService()
     {
          ::CloseServiceHandle( service_ );
     }

private:

     SERVICE_STATUS_PROCESS GetServiceStatus() const
     {
          SERVICE_STATUS_PROCESS ssp;
          DWORD dwBytesNeeded = 0;

          const BOOL query_status = QueryServiceStatusEx( 
               service_, 
               SC_STATUS_PROCESS_INFO,
               reinterpret_cast< LPBYTE >( &ssp ), 
               sizeof( SERVICE_STATUS_PROCESS ),
               &dwBytesNeeded );

          if ( !query_status )
          {
               std::ostringstream strm;
               strm << "can not QueryServiceStatusEx for service \"" << name_ << "\": error " << GetLastError();
               throw std::exception( strm.str().c_str() );
          }

          return ssp;
     }

private:

     SC_HANDLE service_;
     std::string name_;

     const std::string activity_event_name_;
};

class RaiiSCManager
{
public:
     RaiiSCManager()
     {
          manager_ = ::OpenSCManager( NULL, NULL, SC_MANAGER_ALL_ACCESS );
     }     

     ~RaiiSCManager()
     {
          ::CloseServiceHandle( manager_ );
     }

     boost::shared_ptr< RaiiService >
     OpenService( const std::string& name )
     {
          SC_HANDLE srv_handle = ::OpenService( manager_, name.c_str(), SERVICE_ALL_ACCESS );
          
          if ( srv_handle == NULL )
          {               
               std::ostringstream strm;
               strm << "can not open service \"" << name << "\": error " << GetLastError();
               throw std::exception( strm.str().c_str() );
          }

          boost::shared_ptr< RaiiService > srvc( new RaiiService( srv_handle, name ) );

          return srvc;
     }

private:
     SC_HANDLE manager_;
};

struct A {};

struct B
{
     std::string serialize() const
     {
          return "I am a B!";
     }
};

struct C
{
     C( const std::string& str = std::string() )
          : serialize( str )
     {          
     }

     C( const C& other )
          : serialize( other.serialize )
     {
          std::cout << "C( const C& other )" << std::endl;
     }

     std::string serialize;
};

class Shape
{
public:
     void rotate( int angle )
     {
          std::cout << "rotate on angle: " << angle << std::endl;
     }
};

// zaa

#include <iostream>
#include <algorithm>
#include <functional>
#include <boost/filesystem.hpp>

#define MFTP_INFO_TEST_FILENAME "mftp_info_test.txt"

class mftp_info_test_params
{
public:
     mftp_info_test_params( const std::string& base_dir )
          : mftp_info_timeout_def_( 30 )
          , mftp_info_rec_count_def_( 10000 )
          , test_params_path_()
     {
          boost::filesystem::path base_dir_path( base_dir );
          base_dir_path /= MFTP_INFO_TEST_FILENAME;
          test_params_path_ = base_dir_path;

          if ( !boost::filesystem::exists( test_params_path_ ) )
          {
               std::ofstream out_file( test_params_path_.string().c_str() );
               out_file << "mftp_info_timeout:" << mftp_info_timeout_def_ << std::endl;
               out_file << "mftp_info_rec_count:" << mftp_info_rec_count_def_ << std::endl;
          }
     }

     int get_mftp_info_timeout() const
     {
          return read_param_name( "mftp_info_timeout" );
     }
     
     int get_mftp_info_rec_count() const
     {
          return read_param_name( "mftp_info_rec_count" );          
     }

private:
     
     int read_param_name( const std::string& param_name ) const
     {
          std::ifstream in_file( test_params_path_.string().c_str() );       
          while ( !in_file.eof() )
          {
               const size_t title_buff_len = 128;
               std::vector< char > title_buff( title_buff_len );
               in_file.getline( &title_buff[ 0 ], title_buff_len, ':' );
                   
               std::string title( title_buff.begin(), title_buff.end() );
               title.erase( std::remove_if( title.begin(), title.end()
                    , std::not1( std::ptr_fun( std::isprint ) ) ), title.end() );
               if ( title == param_name )
               {
                    int param_value = 0;
                    in_file >> param_value;

                    return param_value;
               }
               else
               {
                    std::string skeep;
                    in_file >> skeep;
               }
          } 

          throw std::domain_error( "there is no param " + param_name  );
     }

private:
     int mftp_info_timeout_def_;
     int mftp_info_rec_count_def_;
     boost::filesystem::path test_params_path_;
};

class shared_data
{
public:

     ~shared_data()
     {
          std::cout << "~shared_data" << std::endl;
     }
};

class NonCopyable
{
protected:
     NonCopyable() {};
     ~NonCopyable() {};
private:
     NonCopyable( const NonCopyable& );
     NonCopyable& operator=( const NonCopyable& );
};

class Spinlock : NonCopyable
{
public:

     Spinlock()
     {
          //pthread_spin_init( &spin_, PTHREAD_PROCESS_PRIVATE );
     }

     ~Spinlock()
     {
          //pthread_spin_destroy( &spin_ );
     }

     void Lock()
     {
          // pthread_spin_lock( &spin_ );
     }

     void Unlock()
     {
         // pthread_spin_unlock( &spin_ );
     }

private:
     //pthread_spinlock_t spin_;
};

template< class Lockable >
class ScopedLock : NonCopyable
{
public:

     explicit ScopedLock( Lockable& locakble )
          : locakble_( locakble )
     {
          locakble_.Lock();
     }

     ~ScopedLock()
     {
          locakble_.Unlock();
     }

private:
     Lockable& locakble_;
};

class ReferenceImpl : NonCopyable
{
public:
     ReferenceImpl()
          : refsCount_( 1 )
     {
     }

     long AddRef()
     {
          ScopedLock< Spinlock > lock( spin_ );

          return ++refsCount_;
     }

     long Release()
     {
          ScopedLock< Spinlock > lock( spin_ );

          return --refsCount_;
     }

     long RefCount() const
     {
          ScopedLock< Spinlock > lock( spin_ );

          return refsCount_;
     }

private:
     mutable Spinlock spin_;
     volatile long refsCount_;
};

template <class PredecessorMap, class Tag>
class path_recorder : public boost::predecessor_recorder< PredecessorMap, Tag >
{
public:

     typedef Tag event_filter;
     path_recorder(PredecessorMap pa, boost::shared_ptr< std::vector< bool > > marked)
          : predecessor_recorder(pa)
          , marked_( marked )
     {          
     }  

     template <class Edge, class Graph>
     void operator()(Edge e, const Graph& g) {
          boost::predecessor_recorder< PredecessorMap, Tag >::operator()< Edge, Graph >( e, g);
          (*marked_)[ target(e, g) ] = true;
     }
private:
     boost::shared_ptr< std::vector< bool > > marked_;
};

template <class PredecessorMap, class Tag>
class path_recorder_ref : public boost::predecessor_recorder< PredecessorMap, Tag >
{
public:

     typedef Tag event_filter;
     path_recorder_ref(PredecessorMap pa, std::vector< bool >& marked)
          : predecessor_recorder(pa)
          , marked_( marked )
     {          
     }  

     template <class Edge, class Graph>
     void operator()(Edge e, const Graph& g) {
          boost::predecessor_recorder< PredecessorMap, Tag >::operator()< Edge, Graph >( e, g);
          marked_.get()[ target(e, g) ] = true;
     }
private:
     boost::reference_wrapper< std::vector< bool > > marked_;
};

template <class PredecessorMap, class Tag>
path_recorder<PredecessorMap, Tag>
make_path_recorder(PredecessorMap pa, Tag, boost::shared_ptr< std::vector< bool > > marked) {
     return path_recorder<PredecessorMap, Tag> (pa, marked);
}

template <class PredecessorMap, class Tag>
path_recorder_ref<PredecessorMap, Tag>
make_path_recorder(PredecessorMap pa, Tag, std::vector< bool >& marked) {
     return path_recorder_ref<PredecessorMap, Tag> (pa, marked);
}

template < class Graph >
class Path
{
public:
     Path( const Graph& graph )
     {
          const size_t vertex_count = boost::num_vertices( graph );

          predecessors_.assign( vertex_count, std::vector< typename boost::graph_traits< Graph >::vertex_descriptor >( vertex_count ) );
          marked_.assign( vertex_count, std::vector< bool >( vertex_count ) );

          CalculatePath( graph );
     }


     std::vector< int > GetPath( int init_vertex, int vertex ) const
     {
          if ( !marked_[ init_vertex ][ vertex ] )
          {
               std::cout << "does not have path v" << init_vertex << " <-> " << "v" << vertex << '\n';
               return std::vector< int >();
          }

          std::stack< int > vertecies_stack;
          int p = vertex;
          while ( p != init_vertex )
          {
               vertecies_stack.push( p );
               std::cout << p << '\n';
               p = predecessors_[ init_vertex ][ p ];
          }
          std::cout << p << '\n';
          vertecies_stack.push( p );

          std::vector< int > path_to_vertex( vertecies_stack.size() );
          while ( !vertecies_stack.empty() )
          {
               path_to_vertex.push_back( vertecies_stack.top() );
               vertecies_stack.pop();
          }

          return path_to_vertex;
     }
private:

     void CalculatePath( const Graph& graph_ )
     {          
          const size_t vertex_count = boost::num_vertices( graph_ );
          std::pair< typename Graph::vertex_iterator,
               typename Graph::vertex_iterator > vs = boost::vertices( graph_ );

          for ( typename Graph::vertex_iterator it = vs.first; it != vs.second; ++it )
          {
               typename boost::graph_traits< Graph >::vertex_descriptor v = *it;

               boost::breadth_first_search( graph_, v,     
                    boost::visitor(
                    boost::make_bfs_visitor(
                    make_path_recorder( &predecessors_[ v ][0],
                    boost::on_tree_edge(), marked_[ v ] ))) );
          }
     }

private:
     std::vector< std::vector< typename boost::graph_traits< Graph >::vertex_descriptor > > predecessors_;
     std::vector< std::vector< bool > > marked_;
};

template < class Container >
void get_path_to( int init_vertex, int vertex, const Container& predecessors, const std::vector< bool >& marked )
{
     if ( !marked[ vertex ] )
     {
          std::cout << "does not have path v" << init_vertex << " <-> " << "v" << vertex << '\n';
          return;
     }
     
     int p = vertex;
     while ( p != init_vertex )
     {
          std::cout << p << '\n';
          p = predecessors[p];
     }
     std::cout << p << '\n';
}

int _tmain(int argc, _TCHAR* argv[])
{
     enum { topLeft, topRight, bottomRight, bottomLeft, sep1, sep2, sep3, sep4 };
     enum { edge_count = 7, vertex_count = 8 };

     boost::array<std::pair<int, int>, edge_count> edges = {
          std::make_pair(topLeft, topRight),
               std::make_pair(topRight, bottomRight),
               std::make_pair(bottomRight, bottomLeft),
               std::make_pair(bottomLeft, topLeft),
               std::make_pair(sep1, sep2),
               std::make_pair(sep1, sep3),
               std::make_pair(sep3, sep4)
     };

     typedef boost::adjacency_list<boost::setS, boost::vecS,
          boost::undirectedS> graph;
     graph g(edges.begin(), edges.end(), vertex_count);

     Path< graph > graph_path( g );

     boost::shared_ptr< std::vector< bool > > marked( new std::vector< bool >( vertex_count, false ) );
     boost::array<int, vertex_count> predecessors;     
     predecessors[bottomRight] = bottomRight;

     boost::breadth_first_search(g, bottomRight,
          boost::visitor(
          boost::make_bfs_visitor(
          make_path_recorder(predecessors.begin(),
          boost::on_tree_edge(), marked))));

     get_path_to( bottomRight, topLeft, predecessors, *marked );
     std::cout << "same path through Path< graph >:" << std::endl;
     graph_path.GetPath( bottomRight, topLeft );

     marked.reset( new std::vector< bool >( vertex_count, false ) );
     boost::array<int, vertex_count> predecessors_for_sep2;     
     predecessors_for_sep2[sep2] = sep2;

     boost::breadth_first_search(g, sep2,
          boost::visitor(
          boost::make_bfs_visitor(
          make_path_recorder(predecessors_for_sep2.begin(),
          boost::on_tree_edge(), marked))));

     get_path_to( sep2, topLeft, predecessors_for_sep2, *marked );
     std::cout << "same path through Path< graph >:" << std::endl;
     graph_path.GetPath( sep2, topLeft );

     get_path_to( sep2, sep4, predecessors_for_sep2, *marked );
     std::cout << "same path through Path< graph >:" << std::endl;
     std::vector< int > path_to_sep4 = graph_path.GetPath( sep2, sep4 );
     std::copy( path_to_sep4.begin(), path_to_sep4.end(), std::ostream_iterator< int >( std::cout, " " ) );

     std::vector< Shape > ls( 10 );
     std::for_each( ls.begin(), ls.end(), std::bind2nd( std::mem_fun_ref( &Shape::rotate ), 90 ) );

     std::mem_fun_ref( &std::vector< Shape >::clear )( ls );
     assert( ls.empty() );

     std::bind1st( std::less< int >(), 1 );
     
     {
          using namespace perf::protocol;

          std::string ch_data( "MSGN 123" );

          common_header header;
          const boost::tribool res = 
               header.deserialize( ch_data.data(), ch_data.size() );
          assert( bool( res == true ) );

          const common_header::body_length_t size = header.get_body_length();

          assert( size == 123 );
     }

     {
          using namespace perf::protocol;

          std::string ch_data( "MSGN 1" );

          common_header header;
          const boost::tribool res = 
               header.deserialize( ch_data.data(), ch_data.size() );

          assert( boost::indeterminate( res ) );
     }

     {
          using namespace perf::protocol;

          std::string ch_data( "MSGN9999" );

          common_header header;
          const boost::tribool res = 
               header.deserialize( ch_data.data(), ch_data.size() );
          assert( bool( res == false ) );
     }

     {
          using namespace perf::protocol;

          common_header header;
          header.set_body_length( 256 );

          char big_buff[ 1024 ] = {};
          size_t buff_len = sizeof( big_buff );          
          const bool res = header.serialize( big_buff, buff_len );

          assert( res );
          const size_t serialized_len = strlen( big_buff );
          assert( serialized_len == common_header::full_length );
     }

     {
          using namespace perf::protocol;

          common_header header;
          header.set_body_length( 256 );

          std::vector< char > buff = header.serialize();

          std::string ref( "MSGN 256" );
          std::string res( buff.begin(), buff.end() );
          assert( buff.size() == common_header::full_length );
          assert( res == ref );
     }

     {
          using namespace perf::protocol;
          
          std::string request_data( "MSGN   3GET" );
          request_parser parser;

          const boost::tribool res = parser.decode_header( 
               request_data.data()
               , request_data.size() );

          assert( bool( res == true ) );

          if ( res )
          {               
               request req;
               const boost::tribool req_res = parser.decode_request( 
                    request_data.data() + parser.get_header_length()
                    , parser.get_request_length()
                    , req );

               assert( bool( req_res == true ) );
          }
     }

     {
          using namespace perf::protocol;

          std::string request_data( "MSGN   " );
          request_parser parser;

          const boost::tribool res = parser.decode_header( 
               request_data.data()
               , request_data.size() );

          assert( boost::indeterminate( res ) );
     }

     {
          using namespace perf::protocol;

          try
          {
               request_parser parser;
               const size_t res = parser.get_request_length();
          }
          catch( const std::exception& e )
          {
               std::cout << e.what() << std::endl;
          }          
     }
            
     try
     {
          RaiiSCManager scm;

          boost::shared_ptr< RaiiService > mftp2( scm.OpenService( "mftp2" ) );
          
          mftp2->Stop();
          
          {
               CallTimeMeasure timer;

               timer.StartMeasure();
               
               mftp2->Start();
               mftp2->WaitForMftp2();
               
               timer.StopMeasure();

               std::cout << "Service mftp2 started during " << timer.GetCallTime() << " sec." << std::endl;
          }
          mftp2->Stop();
     }
     catch( const std::exception& e )
     {
          std::cout << e.what() << std::endl;
     }

     return 0;

     boost::signals2::signal<void ()> sig;
     
     HelloWorld hello;

     UpdateSystemConnector connector;     

     ManWin man( &connector );
     ManProcessing man_proc( &connector );
     /*sig.connect(hello);

     typedef void (HelloWorld::*FunctionFoo)( const FooEvent& );
     boost::signals2::signal<void (const FooEvent&)> sig_foo;
     sig_foo.connect( boost::bind( static_cast<FunctionFoo>( &HelloWorld::OnHandler ), &hello, _1 ) );

     create_signal< FooEvent >( &hello );

     FooEvent evt;
     sig_foo( evt );

     typedef void (HelloWorld::*FunctionBar)( const BarEvent& );
     boost::signals2::signal<void (const BarEvent&)> sig_bar;
     sig_bar.connect( boost::bind( static_cast<FunctionBar>( &HelloWorld::OnHandler ), &hello, _1 ) );

     BarEvent bar_evt;
     sig_bar( bar_evt );

     create_signal< FooEvent, BarEvent >( &hello );*/

     Subscription subscription = CreateSubscription< FooEvent, BarEvent >( &hello );
     subscription.CallEvent( foo );
     subscription.CallEvent( bar );

     return 0;
}
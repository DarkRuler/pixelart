// Copyright (c) 2015-2016 Dr. Colin Hirsch and Daniel Frey
// Please see LICENSE for license or visit https://github.com/taocpp/json/

#ifndef TAOCPP_JSON_INCLUDE_VALUE_HH
#define TAOCPP_JSON_INCLUDE_VALUE_HH

#include <cstdint>
#include <cmath>
#include <cassert>
#include <utility>
#include <stdexcept>

#include "external/operators.hpp"
#include "external/optional.hpp"

#include "internal/totally_ordered.hh"
#include "internal/value_union.hh"
#include "internal/get_by_enum.hh"
#include "internal/throw.hh"

#include "type.hh"
#include "traits.hh"
#include "pair.hh"
#include "single.hh"
#include "pointer.hh"

namespace tao
{
   namespace json
   {
      namespace internal
      {
         template< typename, typename, typename = void > struct has_extract : std::false_type {};
         template< typename T, typename V > struct has_extract< T, V, decltype( T::extract( std::declval< const V & >() ), void() ) > : std::true_type {};

         // required work-around for GCC 6
         inline void rethrow()
         {
            throw;
         }

      } // internal

      template< template< typename ... > class Traits >
      class basic_value
         : operators::totally_ordered< basic_value< Traits > >,
           internal::totally_ordered< basic_value< Traits >, null_t, type::NULL_ >,
           internal::totally_ordered< basic_value< Traits >, bool, type::BOOLEAN >,
           internal::totally_ordered< basic_value< Traits >, signed char, type::SIGNED >,
           internal::totally_ordered< basic_value< Traits >, unsigned char, type::UNSIGNED >,
           internal::totally_ordered< basic_value< Traits >, signed short, type::SIGNED >,
           internal::totally_ordered< basic_value< Traits >, unsigned short, type::UNSIGNED >,
           internal::totally_ordered< basic_value< Traits >, signed int, type::SIGNED >,
           internal::totally_ordered< basic_value< Traits >, unsigned int, type::UNSIGNED >,
           internal::totally_ordered< basic_value< Traits >, signed long, type::SIGNED >,
           internal::totally_ordered< basic_value< Traits >, unsigned long, type::UNSIGNED >,
           internal::totally_ordered< basic_value< Traits >, signed long long, type::SIGNED >,
           internal::totally_ordered< basic_value< Traits >, unsigned long long, type::UNSIGNED >,
           internal::totally_ordered< basic_value< Traits >, float, type::DOUBLE >,
           internal::totally_ordered< basic_value< Traits >, double, type::DOUBLE >,
           internal::totally_ordered< basic_value< Traits >, std::string, type::STRING >,
           internal::totally_ordered< basic_value< Traits >, const char *, type::STRING >,
           internal::totally_ordered< basic_value< Traits >, std::vector< basic_value< Traits > >, type::ARRAY >,
           internal::totally_ordered< basic_value< Traits >, empty_array_t, type::ARRAY >,
           internal::totally_ordered< basic_value< Traits >, std::map< std::string, basic_value< Traits > >, type::OBJECT >,
           internal::totally_ordered< basic_value< Traits >, empty_object_t, type::OBJECT >,
           internal::totally_ordered< basic_value< Traits >, const basic_value< Traits > *, type::RAW_PTR >,
           internal::totally_ordered< basic_value< Traits >, basic_value< Traits > *, type::RAW_PTR >,
           internal::totally_ordered< basic_value< Traits >, std::nullptr_t, type::RAW_PTR >
      {
      public:
         basic_value() noexcept
         { }

         basic_value( basic_value && r ) noexcept
              : m_type( r.m_type )
         {
            seize( std::move( r ) );
         }

         // required work-around for a bug in older GCCs (<4.9)
         basic_value( const basic_value && r )
              : m_type( r.m_type )
         {
            embed( r );
         }

         basic_value( const basic_value & r )
              : m_type( r.m_type )
         {
            embed( r );
         }

         basic_value( basic_value & r )
              : basic_value( static_cast< const basic_value & >( r ) )
         { }

         template< typename T >
         basic_value( T && v ) noexcept( noexcept( Traits< typename std::decay< T >::type >::assign( std::declval< basic_value & >(), std::forward< T >( v ) ) ) )
         {
            try {
               using D = typename std::decay< T >::type;
               Traits< D >::assign( *this, std::forward< T >( v ) );
            }
            catch( ... ) {
               unsafe_destroy();
               internal::rethrow();
            }
         }

         basic_value( std::initializer_list< pair< Traits > > && l )
         {
            try {
               unsafe_assign( std::move( l ) );
            }
            catch( ... ) {
               unsafe_destroy();
               throw;
            }
         }

         basic_value( const std::initializer_list< pair< Traits > > & l )
         {
            try {
               unsafe_assign( l );
            }
            catch( ... ) {
               unsafe_destroy();
               throw;
            }
         }

         basic_value( std::initializer_list< pair< Traits > > & l )
              : basic_value( static_cast< const std::initializer_list< pair< Traits > > & >( l ) )
         { }

         ~basic_value() noexcept
         {
            unsafe_destroy();
         }

         static basic_value array( std::initializer_list< single< Traits > > && l )
         {
            basic_value v;
            v.append( std::move( l ) );
            return v;
         }

         static basic_value array( const std::initializer_list< single< Traits > > & l )
         {
            basic_value v;
            v.append( l );
            return v;
         }

         basic_value & operator= ( basic_value v ) noexcept
         {
            unsafe_destroy();
            seize( std::move( v ) );
            m_type = v.m_type;
            return * this;
         }

         void swap( basic_value & r ) noexcept
         {
            basic_value t( std::move( r ) );
            r = std::move( * this );
            ( * this ) = ( std::move( t ) );
         }

         json::type type() const noexcept
         {
            return m_type;
         }

         bool is_null() const noexcept
         {
            return m_type == json::type::NULL_;
         }

         bool is_bool() const noexcept
         {
            return m_type == json::type::BOOLEAN;
         }

         bool is_signed() const noexcept
         {
            return m_type == json::type::SIGNED;
         }

         bool is_unsigned() const noexcept
         {
            return m_type == json::type::UNSIGNED;
         }

         bool is_integer() const noexcept
         {
            return is_signed() || is_unsigned();
         }

         bool is_double() const noexcept
         {
            return m_type == json::type::DOUBLE;
         }

         bool is_number() const noexcept
         {
            return is_integer() || is_double();
         }

         bool is_string() const noexcept
         {
            return m_type == json::type::STRING;
         }

         bool is_array() const noexcept
         {
            return m_type == json::type::ARRAY;
         }

         bool is_object() const noexcept
         {
            return m_type == json::type::OBJECT;
         }

         bool is_raw_ptr() const noexcept
         {
            return m_type == json::type::RAW_PTR;
         }

         null_t get_null() const
         {
            TAOCPP_JSON_CHECK_TYPE_ERROR( m_type, json::type::NULL_ );
            return unsafe_get_null();  // LCOV_EXCL_LINE (always optimized)
         }

         bool get_bool() const
         {
            TAOCPP_JSON_CHECK_TYPE_ERROR( m_type, json::type::BOOLEAN );
            return unsafe_get_bool();
         }

         std::int64_t get_signed() const
         {
            TAOCPP_JSON_CHECK_TYPE_ERROR( m_type, json::type::SIGNED );
            return unsafe_get_signed();
         }

         std::uint64_t get_unsigned() const
         {
            TAOCPP_JSON_CHECK_TYPE_ERROR( m_type, json::type::UNSIGNED );
            return unsafe_get_unsigned();
         }

         double get_double() const
         {
            TAOCPP_JSON_CHECK_TYPE_ERROR( m_type, json::type::DOUBLE );
            return unsafe_get_double();
         }

         std::string & get_string()
         {
            TAOCPP_JSON_CHECK_TYPE_ERROR( m_type, json::type::STRING );
            return unsafe_get_string();
         }

         const std::string & get_string() const
         {
            TAOCPP_JSON_CHECK_TYPE_ERROR( m_type, json::type::STRING );
            return unsafe_get_string();
         }

         std::vector< basic_value > & get_array()
         {
            TAOCPP_JSON_CHECK_TYPE_ERROR( m_type, json::type::ARRAY );
            return unsafe_get_array();
         }

         const std::vector< basic_value > & get_array() const
         {
            TAOCPP_JSON_CHECK_TYPE_ERROR( m_type, json::type::ARRAY );
            return unsafe_get_array();
         }

         std::map< std::string, basic_value > & get_object()
         {
            TAOCPP_JSON_CHECK_TYPE_ERROR( m_type, json::type::OBJECT );
            return unsafe_get_object();
         }

         const std::map< std::string, basic_value > & get_object() const
         {
            TAOCPP_JSON_CHECK_TYPE_ERROR( m_type, json::type::OBJECT );
            return unsafe_get_object();
         }

         const basic_value * get_raw_ptr() const
         {
            TAOCPP_JSON_CHECK_TYPE_ERROR( m_type, json::type::RAW_PTR );
            return unsafe_get_raw_ptr();
         }

         template< json::type E >
         decltype( internal::get_by_enum< E >::get( std::declval< internal::value_union< basic_value > & >() ) ) get()
         {
            TAOCPP_JSON_CHECK_TYPE_ERROR( m_type, E );
            return internal::get_by_enum< E >::get( m_union );
         }

         template< json::type E >
         decltype( internal::get_by_enum< E >::get( std::declval< const internal::value_union< basic_value > & >() ) ) get() const
         {
            TAOCPP_JSON_CHECK_TYPE_ERROR( m_type, E );
            return internal::get_by_enum< E >::get( m_union );
         }

         template< typename T >
         void as( T& v ) const
         {
            Traits< typename std::decay< T >::type >::extract( * this, v );
         }

         template< typename T >
         typename std::enable_if< internal::has_extract< Traits< T >, basic_value >::value, T >::type as() const
         {
            return Traits< T >::extract( * this );
         }

         template< typename T >
         typename std::enable_if< !internal::has_extract< Traits< T >, basic_value >::value, T >::type as() const
         {
            T nrv;
            as( nrv );
            return nrv;
         }

         template< typename T >
         tao::optional< T > optional() const
         {
           if ( this->is_null() ) {
              return tao::nullopt;
           }
           else {
              return this->as< T >();
           }
         }

         // The unsafe_get_*() accessor functions MUST NOT be
         // called when the type of the value is not the one
         // corresponding to the type of the accessor!

         // LCOV_EXCL_START (always optimized)
         null_t unsafe_get_null() const noexcept
         {
            return null;
         }
         // LCOV_EXCL_STOP

         bool unsafe_get_bool() const noexcept
         {
            return m_union.b;
         }

         std::int64_t unsafe_get_signed() const noexcept
         {
            return m_union.i;
         }

         std::uint64_t unsafe_get_unsigned() const noexcept
         {
            return m_union.u;
         }

         double unsafe_get_double() const noexcept
         {
            return m_union.d;
         }

         std::string & unsafe_get_string() noexcept
         {
            return m_union.s;
         }

         const std::string & unsafe_get_string() const noexcept
         {
            return m_union.s;
         }

         std::vector< basic_value > & unsafe_get_array() noexcept
         {
            return m_union.a;
         }

         const std::vector< basic_value > & unsafe_get_array() const noexcept
         {
            return m_union.a;
         }

         std::map< std::string, basic_value > & unsafe_get_object() noexcept
         {
            return m_union.o;
         }

         const std::map< std::string, basic_value > & unsafe_get_object() const noexcept
         {
            return m_union.o;
         }

         const basic_value * unsafe_get_raw_ptr() const noexcept
         {
            return m_union.p;
         }

         template< json::type E >
         decltype( internal::get_by_enum< E >::get( std::declval< internal::value_union< basic_value > & >() ) ) unsafe_get()
         {
            return internal::get_by_enum< E >::get( m_union );
         }

         template< json::type E >
         decltype( internal::get_by_enum< E >::get( std::declval< const internal::value_union< basic_value > & >() ) ) unsafe_get() const
         {
            return internal::get_by_enum< E >::get( m_union );
         }

         basic_value * unsafe_find( const std::string & key )
         {
            const auto it = m_union.o.find( key );
            return ( it != m_union.o.end() ) ? ( & it->second ) : nullptr;
         }

         const basic_value * unsafe_find( const std::string & key ) const
         {
            const auto it = m_union.o.find( key );
            return ( it != m_union.o.end() ) ? ( & it->second ) : nullptr;
         }

         basic_value * find( const std::string & key )
         {
            TAOCPP_JSON_CHECK_TYPE_ERROR( m_type, json::type::OBJECT );
            return unsafe_find( key );
         }

         const basic_value * find( const std::string & key ) const
         {
            TAOCPP_JSON_CHECK_TYPE_ERROR( m_type, json::type::OBJECT );
            return unsafe_find( key );
         }

         basic_value & at( const std::size_t index )
         {
            TAOCPP_JSON_CHECK_TYPE_ERROR( m_type, json::type::ARRAY );
            return m_union.a.at( index );
         }

         const basic_value & at( const std::size_t index ) const
         {
            TAOCPP_JSON_CHECK_TYPE_ERROR( m_type, json::type::ARRAY );
            return m_union.a.at( index );
         }

         basic_value & at( const std::string & key )
         {
            TAOCPP_JSON_CHECK_TYPE_ERROR( m_type, json::type::OBJECT );
            return m_union.o.at( key );
         }

         const basic_value & at( const std::string & key ) const
         {
            TAOCPP_JSON_CHECK_TYPE_ERROR( m_type, json::type::OBJECT );
            return m_union.o.at( key );
         }

         basic_value & at( const pointer & k )
         {
            return internal::pointer_at( this, k.begin(), k.end() );
         }

         const basic_value & at( const pointer & k ) const
         {
            return internal::pointer_at( this, k.begin(), k.end() );
         }

         basic_value & unsafe_at( const std::size_t index )
         {
            return m_union.a[ index ];
         }

         const basic_value & unsafe_at( const std::size_t index ) const
         {
            return m_union.a[ index ];
         }

         basic_value & unsafe_at( const std::string & key )
         {
            return m_union.o.find( key )->second;
         }

         const basic_value & unsafe_at( const std::string & key ) const
         {
            return m_union.o.find( key )->second;
         }

         void erase( const std::size_t index )
         {
            TAOCPP_JSON_CHECK_TYPE_ERROR( m_type, json::type::ARRAY );
            auto & a = m_union.a;
            if ( index >= a.size() ) {
               throw std::out_of_range( "JSON array index out of bounds" );
            }
            a.erase( a.begin() + index );
         }

         void erase( const std::string & key )
         {
            TAOCPP_JSON_CHECK_TYPE_ERROR( m_type, json::type::OBJECT );
            if ( m_union.o.erase( key ) == 0 ) {
               throw std::out_of_range( "JSON object key not found: " + key );
            }
         }

         void erase( const pointer & k )
         {
            if ( ! k ) {
               throw std::runtime_error( "invalid root JSON Pointer for erase" );
            }
            const auto b = k.begin();
            const auto e = std::prev( k.end() );
            basic_value & v = internal::pointer_at( this, b, e );
            switch ( v.m_type ) {
               case json::type::ARRAY:
                  v.erase( e->index() );
                  break;
               case json::type::OBJECT:
                  v.erase( e->key() );
                  break;
               default:
                  throw internal::invalid_type( b, std::next( e ) );
            }
         }

         basic_value & insert( const pointer & k, basic_value value )
         {
            if ( ! k ) {
               * this = std::move( value );
               return * this;
            }
            const auto b = k.begin();
            const auto e = std::prev( k.end() );
            basic_value & v = internal::pointer_at( this, b, e );
            switch ( v.m_type ) {
               case json::type::ARRAY:
                  {
                     if ( e->key() == "-" ) {
                        v.unsafe_emplace_back( std::move( value ) );
                        return v.m_union.a.back();
                     }
                     const auto i = e->index();
                     if ( i >= v.m_union.a.size() ) {
                        throw std::out_of_range( "invalid JSON Pointer '" + internal::tokens_to_string( b, std::next( e ) ) + "' -- array index out of bounds" );
                     }
                     v.m_union.a.insert( v.m_union.a.begin() + i, std::move( value ) );
                     return v.m_union.a.at( i );
                  }
                  break;
               case json::type::OBJECT:
                  {
                     const auto & k = e->key();
                     const auto it = v.m_union.o.find( k );
                     if ( it == v.m_union.o.end() ) {
                        const auto r = v.unsafe_emplace( k, std::move( value ) );
                        assert( r.second );
                        return r.first->second;
                     }
                     it->second = std::move( value );
                     return it->second;
                  }
                  break;
               default:
                  throw internal::invalid_type( b, std::next( e ) );
            }
         }

         basic_value & operator[] ( const std::size_t index ) noexcept
         {
            return m_union.a[ index ];
         }

         const basic_value & operator[] ( const std::size_t index ) const noexcept
         {
            return m_union.a[ index ];
         }

         basic_value & operator[] ( const std::string & key )
         {
            return m_union.o[ key ];
         }

         basic_value & operator[] ( std::string && key )
         {
            return m_union.o[ std::move( key ) ];
         }

         basic_value & operator[] ( const pointer & k )
         {
            if ( ! k ) {
               return * this;
            }
            const auto b = k.begin();
            const auto e = std::prev( k.end() );
            basic_value & v = internal::pointer_at( this, b, e );
            switch ( v.m_type ) {
               case json::type::ARRAY:
                  {
                     if ( e->key() == "-" ) {
                        v.unsafe_emplace_back( null );
                        return v.m_union.a.back();
                     }
                     return v.at( e->index() );
                  }
                  break;
               case json::type::OBJECT:
                  {
                     const auto & k = e->key();
                     const auto it = v.m_union.o.find( k );
                     if ( it == v.m_union.o.end() ) {
                        const auto r = v.unsafe_emplace( k, null );
                        assert( r.second );
                        return r.first->second;
                     }
                     return it->second;
                  }
                  break;
               default:
                  throw internal::invalid_type( b, std::next( e ) );
            }
         }

         template< typename T >
         tao::optional< T > optional( const std::string & key ) const
         {
            TAOCPP_JSON_CHECK_TYPE_ERROR( m_type, json::type::OBJECT );
            const auto it = m_union.o.find( key );
            if ( it == m_union.o.end() ) {
               return tao::nullopt;
            }
            else {
               return it->second.template as< T >();
            }
         }

         // The unsafe_assign()-functions MUST NOT be called on a
         // value v when json::needs_destroy( v.type() ) is true!

         template< typename T >
         void unsafe_assign( T && v ) noexcept( noexcept( Traits< typename std::decay< T >::type >::assign( std::declval< basic_value & >(), std::forward< T >( v ) ) ) )
         {
            using D = typename std::decay< T >::type;
            Traits< D >::assign( *this, std::forward< T >( v ) );
         }

         void unsafe_assign( std::initializer_list< pair< Traits > > && l )
         {
            unsafe_emplace_object();
            for ( auto & e : l ) {
               const auto r = unsafe_emplace( std::move( e.key ), std::move( e.value ) );
               if ( ! r.second ) {
                  throw std::runtime_error( "duplicate JSON object key: " + r.first->first );
               }
            }
         }

         void unsafe_assign( const std::initializer_list< pair< Traits > > & l )
         {
            unsafe_emplace_object();
            for ( const auto & e : l ) {
               const auto r = unsafe_emplace( e.key, e.value );
               if ( ! r.second ) {
                  throw std::runtime_error( "duplicate JSON object key: " + r.first->first );
               }
            }
         }

         void unsafe_assign( std::initializer_list< pair< Traits > > & l )
         {
            unsafe_assign( static_cast< const std::initializer_list< pair< Traits > > & >( l ) );
         }

         void unsafe_assign_null() noexcept
         {
            m_type = json::type::NULL_;
         }

         void unsafe_assign_bool( const bool b ) noexcept
         {
            m_union.b = b;
            m_type = json::type::BOOLEAN;
         }

         void unsafe_assign_signed( const std::int64_t i ) noexcept
         {
            m_union.i = i;
            m_type = json::type::SIGNED;
         }

         void unsafe_assign_unsigned( const std::uint64_t u ) noexcept
         {
            m_union.u = u;
            m_type = json::type::UNSIGNED;
         }

         void unsafe_assign_double_unchecked( const double d ) noexcept
         {
            // d must be a finite value!
            m_union.d = d;
            m_type = json::type::DOUBLE;
         }

         void unsafe_assign_double( const double d )
         {
            if ( ! std::isfinite( d ) ) {
               throw std::runtime_error( "non-finite double value invalid for JSON" );
            }
            unsafe_assign_double_unchecked( d );
         }

         template< typename ... Ts >
         void unsafe_emplace_string( Ts && ... ts )
         {
            new ( & m_union.s ) std::string( std::forward< Ts >( ts ) ... );
            m_type = json::type::STRING;
         }

         template< typename ... Ts >
         void emplace_string( Ts && ... ts )
         {
            destroy();
            unsafe_emplace_string( std::forward< Ts >( ts ) ... );
         }

         template< typename ... Ts >
         void unsafe_emplace_array( Ts && ... ts )
         {
            new ( & m_union.a ) std::vector< basic_value >( std::forward< Ts >( ts ) ... );
            m_type = json::type::ARRAY;
         }

         template< typename ... Ts >
         void emplace_array( Ts && ... ts )
         {
            destroy();
            unsafe_emplace_array( std::forward< Ts >( ts ) ... );
         }

         void prepare_array()
         {
            switch ( m_type ) {
               case json::type::NULL_:
                  unsafe_emplace_array();
               case json::type::ARRAY:
                  break;
               default:
                  TAOCPP_JSON_THROW_TYPE_ERROR( m_type );
            }
         }

         template< typename V >
         void unsafe_emplace_back( V && v )
         {
            m_union.a.emplace_back( std::forward< V >( v ) );
         }

         template< typename V >
         void emplace_back( V && v )
         {
            prepare_array();
            unsafe_emplace_back( std::forward< V >( v ) );
         }

         template< typename ... Ts >
         void unsafe_emplace_object( Ts && ... ts )
         {
            new ( & m_union.o ) std::map< std::string, basic_value >( std::forward< Ts >( ts ) ... );
            m_type = json::type::OBJECT;
         }

         template< typename ... Ts >
         void emplace_object( Ts && ... ts )
         {
            destroy();
            unsafe_emplace_object( std::forward< Ts >( ts ) ... );
         }

         void prepare_object()
         {
            switch ( m_type ) {
               case json::type::NULL_:
                  unsafe_emplace_object();
               case json::type::OBJECT:
                  break;
               default:
                  TAOCPP_JSON_THROW_TYPE_ERROR( m_type );
            }
         }

         template< typename K, typename V >
         std::pair< typename std::map< std::string, basic_value >::iterator, bool > unsafe_emplace( K && k, V && v )
         {
            return m_union.o.emplace( std::forward< K >( k ), std::forward< V >( v ) );
         }

         template< typename K, typename V >
         std::pair< typename std::map< std::string, basic_value >::iterator, bool > emplace( K && k, V && v )
         {
            prepare_object();
            return unsafe_emplace( std::forward< K >( k ), std::forward< V >( v ) );
         }

         void unsafe_assign_pointer( const basic_value * p ) noexcept
         {
            m_union.p = p;
            m_type = json::type::RAW_PTR;
         }

         void append( std::initializer_list< single< Traits > > && l )
         {
            prepare_array();
            auto & v = unsafe_get_array();
            v.reserve( v.size() + l.size() );
            for ( auto & e : l ) {
               unsafe_emplace_back( std::move( e.value ) );
            }
         }

         void append( const std::initializer_list< single< Traits > > & l )
         {
            prepare_array();
            auto & v = unsafe_get_array();
            v.reserve( v.size() + l.size() );
            for ( const auto & e : l ) {
               unsafe_emplace_back( e.value );
            }
         }

         void insert( std::initializer_list< pair< Traits > > && l )
         {
            prepare_object();
            for ( auto & e : l ) {
               const auto r = unsafe_emplace( std::move( e.key ), std::move( e.value ) );
               if ( ! r.second ) {
                  throw std::runtime_error( "duplicate JSON object key: " + r.first->first );
               }
            }
         }

         void insert( const std::initializer_list< pair< Traits > > & l )
         {
            prepare_object();
            for ( const auto & e : l ) {
               const auto r = unsafe_emplace( e.key, e.value );
               if ( ! r.second ) {
                  throw std::runtime_error( "duplicate JSON object key: " + r.first->first );
               }
            }
         }

         bool empty() const noexcept
         {
            switch ( m_type ) {
               case json::type::NULL_:
                  return true;
               case json::type::BOOLEAN:
               case json::type::SIGNED:
               case json::type::UNSIGNED:
               case json::type::DOUBLE:
                  return false;
               case json::type::STRING:
                  return m_union.s.empty();
               case json::type::ARRAY:
                  return m_union.a.empty();
               case json::type::OBJECT:
                  return m_union.o.empty();
               case json::type::RAW_PTR:
                  return !m_union.p;
            }
            assert( false );  // LCOV_EXCL_LINE
         }

         void unsafe_destroy() noexcept
         {
            switch ( m_type ) {
               case json::type::NULL_:
               case json::type::BOOLEAN:
               case json::type::SIGNED:
               case json::type::UNSIGNED:
               case json::type::DOUBLE:
               case json::type::RAW_PTR:
                  return;
               case json::type::STRING:
                  m_union.s.~basic_string();
                  return;
               case json::type::ARRAY:
                  m_union.a.~vector();
                  return;
               case json::type::OBJECT:
                  m_union.o.~map();
                  return;
            }
            assert( false );  // LCOV_EXCL_LINE
         }

         void destroy() noexcept
         {
            unsafe_destroy();
            m_type = json::type::NULL_;
         }

      private:
         void seize( basic_value && r ) noexcept
         {
            switch ( r.m_type ) {
               case json::type::NULL_:
                  return;
               case json::type::BOOLEAN:
                  m_union.b = r.m_union.b;
                  return;
               case json::type::SIGNED:
                  m_union.i = r.m_union.i;
                  return;
               case json::type::UNSIGNED:
                  m_union.u = r.m_union.u;
                  return;
               case json::type::DOUBLE:
                  m_union.d = r.m_union.d;
                  return;
               case json::type::STRING:
                  new ( & m_union.s ) std::string( std::move( r.m_union.s ) );
                  return;
               case json::type::ARRAY:
                  new ( & m_union.a ) std::vector< basic_value >( std::move( r.m_union.a ) );
                  return;
               case json::type::OBJECT:
                  new ( & m_union.o ) std::map< std::string, basic_value >( std::move( r.m_union.o ) );
                  return;
               case json::type::RAW_PTR:
                  m_union.p = r.m_union.p;
                  return;
            }
            assert( false );  // LCOV_EXCL_LINE
         }

         void embed( const basic_value & r )
         {
            switch ( r.m_type ) {
               case json::type::NULL_:
                  return;
               case json::type::BOOLEAN:
                  m_union.b = r.m_union.b;
                  return;
               case json::type::SIGNED:
                  m_union.i = r.m_union.i;
                  return;
               case json::type::UNSIGNED:
                  m_union.u = r.m_union.u;
                  return;
               case json::type::DOUBLE:
                  m_union.d = r.m_union.d;
                  return;
               case json::type::STRING:
                  new ( & m_union.s ) std::string( r.m_union.s );
                  return;
               case json::type::ARRAY:
                  new ( & m_union.a ) std::vector< basic_value >( r.m_union.a );
                  return;
               case json::type::OBJECT:
                  new ( & m_union.o ) std::map< std::string, basic_value >( r.m_union.o );
                  return;
               case json::type::RAW_PTR:
                  m_union.p = r.m_union.p;
                  return;
            }
            assert( false );  // LCOV_EXCL_LINE
         }

         internal::value_union< basic_value > m_union;
         json::type m_type = json::type::NULL_;
      };

      template< template< typename ... > class Traits >
      bool operator== ( const basic_value< Traits > & lhs, const basic_value< Traits > & rhs ) noexcept
      {
         if ( lhs.type() == type::RAW_PTR ) {
            if ( const auto * p = lhs.unsafe_get_raw_ptr() ) {
               return * p == rhs;
            }
            else {
               return null == rhs;
            }
         }
         if ( lhs.type() != rhs.type() ) {
            if ( rhs.type() == type::RAW_PTR ) {
               if ( const auto * p = rhs.unsafe_get_raw_ptr() ) {
                  return lhs == * p;
               }
               else {
                  return lhs == null;
               }
            }
            if ( lhs.type() == type::SIGNED ) {
               if ( rhs.type() == type::UNSIGNED ) {
                  const auto v = lhs.unsafe_get_signed();
                  return ( v >= 0 ) && ( static_cast< std::uint64_t >( v ) == rhs.unsafe_get_unsigned() );
               }
               if ( rhs.type() == type::DOUBLE ) {
                  return lhs.unsafe_get_signed() == rhs.unsafe_get_double();
               }
            }
            else if ( lhs.type() == type::UNSIGNED ) {
               if ( rhs.type() == type::SIGNED ) {
                  const auto v = rhs.unsafe_get_signed();
                  return ( v >= 0 ) && ( lhs.unsafe_get_unsigned() == static_cast< std::uint64_t >( v ) );
               }
               if ( rhs.type() == type::DOUBLE ) {
                  return lhs.unsafe_get_unsigned() == rhs.unsafe_get_double();
               }
            }
            else if ( lhs.type() == type::DOUBLE ) {
               if ( rhs.type() == type::SIGNED ) {
                  return lhs.unsafe_get_double() == rhs.unsafe_get_signed();
               }
               if ( rhs.type() == type::UNSIGNED ) {
                  return lhs.unsafe_get_double() == rhs.unsafe_get_unsigned();
               }
            }
            return false;
         }
         switch ( lhs.type() ) {
            case type::NULL_:
               return true;
            case type::BOOLEAN:
               return lhs.unsafe_get_bool() == rhs.unsafe_get_bool();
            case type::SIGNED:
               return lhs.unsafe_get_signed() == rhs.unsafe_get_signed();
            case type::UNSIGNED:
               return lhs.unsafe_get_unsigned() == rhs.unsafe_get_unsigned();
            case type::DOUBLE:
               return lhs.unsafe_get_double() == rhs.unsafe_get_double();
            case type::STRING:
               return lhs.unsafe_get_string() == rhs.unsafe_get_string();
            case type::ARRAY:
               return lhs.unsafe_get_array() == rhs.unsafe_get_array();
            case type::OBJECT:
               return lhs.unsafe_get_object() == rhs.unsafe_get_object();
            case type::RAW_PTR:
               break;  // LCOV_EXCL_LINE
         }
         assert( false );  // LCOV_EXCL_LINE
      }

      template< template< typename ... > class Traits >
      bool operator< ( const basic_value< Traits > & lhs, const basic_value< Traits > & rhs ) noexcept
      {
         if ( lhs.type() == type::RAW_PTR ) {
            if ( const auto * p = lhs.unsafe_get_raw_ptr() ) {
               return * p < rhs;
            }
            else {
               return null < rhs;
            }
         }
         if ( lhs.type() != rhs.type() ) {
            if ( rhs.type() == type::RAW_PTR ) {
               if ( const auto * p = rhs.unsafe_get_raw_ptr() ) {
                  return lhs < * p;
               }
               else {
                  return lhs < null;
               }
            }
            if ( lhs.type() == type::SIGNED ) {
               if ( rhs.type() == type::UNSIGNED ) {
                  const auto v = lhs.unsafe_get_signed();
                  return ( v < 0 ) || ( static_cast< std::uint64_t >( v ) < rhs.unsafe_get_unsigned() );
               }
               if ( rhs.type() == type::DOUBLE ) {
                  return lhs.unsafe_get_signed() < rhs.unsafe_get_double();
               }
            }
            else if ( lhs.type() == type::UNSIGNED ) {
               if ( rhs.type() == type::SIGNED ) {
                  const auto v = rhs.unsafe_get_signed();
                  return ( v >= 0 ) && ( lhs.unsafe_get_unsigned() < static_cast< std::uint64_t >( v ) );
               }
               if ( rhs.type() == type::DOUBLE ) {
                  return lhs.unsafe_get_unsigned() < rhs.unsafe_get_double();
               }
            }
            else if ( lhs.type() == type::DOUBLE ) {
               if ( rhs.type() == type::SIGNED ) {
                  return lhs.unsafe_get_double() < rhs.unsafe_get_signed();
               }
               if ( rhs.type() == type::UNSIGNED ) {
                  return lhs.unsafe_get_double() < rhs.unsafe_get_unsigned();
               }
            }
            return lhs.type() < rhs.type();
         }
         switch ( lhs.type() ) {
            case type::NULL_:
               return false;
            case type::BOOLEAN:
               return lhs.unsafe_get_bool() < rhs.unsafe_get_bool();
            case type::SIGNED:
               return lhs.unsafe_get_signed() < rhs.unsafe_get_signed();
            case type::UNSIGNED:
               return lhs.unsafe_get_unsigned() < rhs.unsafe_get_unsigned();
            case type::DOUBLE:
               return lhs.unsafe_get_double() < rhs.unsafe_get_double();
            case type::STRING:
               return lhs.unsafe_get_string() < rhs.unsafe_get_string();
            case type::ARRAY:
               return lhs.unsafe_get_array() < rhs.unsafe_get_array();
            case type::OBJECT:
               return lhs.unsafe_get_object() < rhs.unsafe_get_object();
            case type::RAW_PTR:
               break;  // LCOV_EXCL_LINE
         }
         assert( false );  // LCOV_EXCL_LINE
      }

      template< template< typename ... > class Traits >
      basic_value< Traits > & operator+= ( basic_value< Traits > & v, std::initializer_list< pair< Traits > > && l )
      {
         v.insert( std::move( l ) );
         return v;
      }

      template< template< typename ... > class Traits >
      basic_value< Traits > & operator+= ( basic_value< Traits > & v, const std::initializer_list< pair< Traits > > & l )
      {
         v.insert( l );
         return v;
      }

      template< template< typename ... > class Traits >
      basic_value< Traits > & operator-= ( basic_value< Traits > & v, std::initializer_list< std::string > l )
      {
         auto & o = v.get_object();
         for ( const auto & k : l ) {
            if ( o.erase( k ) == 0 ) {
               throw std::runtime_error( "JSON object key not found: " + k );
            }
         }
         return v;
      }

      using value = basic_value< traits >;

   } // json

} // tao

#endif

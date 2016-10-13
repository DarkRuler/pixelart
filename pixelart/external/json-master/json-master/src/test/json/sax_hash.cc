// Copyright (c) 2016 Dr. Colin Hirsch and Daniel Frey
// Please see LICENSE for license or visit https://github.com/taocpp/json/

#include "test.hh"

#include <tao/json/value.hh>
#include <tao/json/sax/hash.hh>
#include <tao/json/sax/traverse_value.hh>
#include <tao/json/sax/from_string.hh>

namespace tao
{
   namespace json
   {
      std::string hash_value( const value & v )
      {
         sax::hash h;
         sax::traverse_value( v, h );
         return h.value();
      }

      std::string hash_parse( const std::string & v )
      {
         sax::hash h;
         sax::from_string( v, h );
         return h.value();
      }

      bool test( const value & v, const std::string & s )
      {
         return hash_value( v ) == hash_parse( s );
      }

      void unit_test()
      {
         TEST_ASSERT( test( null, "null" ) );
         TEST_ASSERT( test( 0, "0" ) );
         TEST_ASSERT( ! test( 0, "null" ) );

         TEST_ASSERT( test( empty_array, "[]" ) );
         TEST_ASSERT( test( value::array( { 1, 2u, 3 } ), "[1,2,3.0]" ) );

         TEST_ASSERT( test( empty_object, "{}" ) );
         TEST_ASSERT( test( { { "a", 0 }, { "b", 1 } }, "{\"a\":0,\"b\":1}" ) );
         TEST_ASSERT( test( { { "a", 0 }, { "b", -1 } }, "{\"a\":-0,\"b\":-1}" ) );
         TEST_ASSERT( test( { { "a", 0.0 }, { "b", 1 } }, "{\"a\":0,\"b\":1.0}" ) );
         TEST_ASSERT( test( { { "a", 0 }, { "b", -1.0 } }, "{\"a\":-0.0,\"b\":-1}" ) );
         TEST_ASSERT( test( { { "a", 0 }, { "b", 1 } }, "{ \"a\": 0, \"b\": 1 }" ) );
         TEST_ASSERT( test( { { "a", 0 }, { "b", 1 } }, "{ \"b\": 1, \"a\": 0 }" ) );
         TEST_ASSERT( test( { { "a", true }, { "b", false } }, "{ \"a\": true, \"b\": false }" ) );
         TEST_ASSERT( test( { { "a", "Hello" }, { "b", "World" }, { "c", 1.25 } }, "{ \"a\": \"H\\u0065llo\", \"b\": \"World\", \"c\": 125e-2 }" ) );

         TEST_ASSERT( ! test( { { "a", 0 }, { "b", 1 } }, "{ \"a\": 1, \"b\": 1 }" ) );
         TEST_ASSERT( ! test( { { "a", 0 }, { "b", 1 } }, "{ \"a\": 0, \"b\": 0 }" ) );
         TEST_ASSERT( ! test( { { "a", 0 }, { "b", 1 } }, "{ \"a\": 0 }" ) );
         TEST_ASSERT( ! test( { { "a", 0 }, { "b", 1 } }, "{ \"b\": 1 }" ) );
         TEST_ASSERT( ! test( { { "a", 0 }, { "b", 1 } }, "{ \"a\": 0, \"b\": 1, \"c\": 2 }" ) );
         TEST_ASSERT( ! test( { { "a", 0 }, { "b", 1 } }, "{ \"a\": 0, \"c\": 1 }" ) );
         TEST_ASSERT( ! test( { { "a", 0 }, { "b", 1 } }, "{ \"c\": 0, \"b\": 1 }" ) );

         TEST_THROWS( test( { { "a", 0 }, { "b", 1 } }, "{ \"a\": 0, \"a\": 0, \"c\": 1 }" ) );
      }

   } // json

} // tao

#include "main.hh"

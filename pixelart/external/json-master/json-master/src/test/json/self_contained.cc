// Copyright (c) 2016 Dr. Colin Hirsch and Daniel Frey
// Please see LICENSE for license or visit https://github.com/taocpp/json/

#include "test.hh"

#include <tao/json/value.hh>
#include <tao/json/self_contained.hh>

namespace tao
{
   namespace json
   {
      void unit_test()
      {
         value v = { { "foo", 1 } };

         value v1 = { { "bar", v }, { "baz", value::array( { 2, v, 3, nullptr } ) } };
         value v2 = { { "bar", &v }, { "baz", value::array( { 2, &v, 3, nullptr } ) } };
         value v3 = { { "bar", v }, { "baz", value::array( { 2, v, 3, null } ) } };
         value v4 = { { "bar", { { "foo", 1 } } }, { "baz", value::array( { 2, { { "foo", 1 } }, 3, null } ) } };

         TEST_ASSERT( v1 == v2 );
         TEST_ASSERT( v1 == v3 );
         TEST_ASSERT( v1 == v4 );
         TEST_ASSERT( v2 == v3 );
         TEST_ASSERT( v2 == v4 );
         TEST_ASSERT( v3 == v4 );

         TEST_ASSERT( v1.at( "bar" ).type() == type::OBJECT );
         TEST_ASSERT( v1.at( "baz" ).at( 1 ).type() == type::OBJECT );
         TEST_ASSERT( v1.at( "baz" ).at( 3 ).type() == type::RAW_PTR );

         TEST_ASSERT( v2.at( "bar" ).type() == type::RAW_PTR );
         TEST_ASSERT( v2.at( "baz" ).at( 1 ).type() == type::RAW_PTR );
         TEST_ASSERT( v2.at( "baz" ).at( 3 ).type() == type::RAW_PTR );

         TEST_ASSERT( v3.at( "bar" ).type() == type::OBJECT );
         TEST_ASSERT( v3.at( "baz" ).at( 1 ).type() == type::OBJECT );
         TEST_ASSERT( v3.at( "baz" ).at( 3 ).type() == type::NULL_ );

         TEST_ASSERT( ! is_self_contained( v1 ) );
         TEST_ASSERT( ! is_self_contained( v2 ) );
         TEST_ASSERT( is_self_contained( v3 ) );
         TEST_ASSERT( is_self_contained( v4 ) );

         make_self_contained( v1 );
         make_self_contained( v2 );

         TEST_ASSERT( is_self_contained( v1 ) );
         TEST_ASSERT( is_self_contained( v2 ) );
         TEST_ASSERT( v1 == v2 );
         TEST_ASSERT( v1 == v3 );
         TEST_ASSERT( v1 == v4 );
         TEST_ASSERT( v2 == v3 );
         TEST_ASSERT( v2 == v4 );
         TEST_ASSERT( v3 == v4 );

         TEST_ASSERT( v1.at( "bar" ).type() == type::OBJECT );
         TEST_ASSERT( v1.at( "baz" ).at( 1 ).type() == type::OBJECT );
         TEST_ASSERT( v1.at( "baz" ).at( 3 ).type() == type::NULL_ );

         TEST_ASSERT( v2.at( "bar" ).type() == type::OBJECT );
         TEST_ASSERT( v2.at( "baz" ).at( 1 ).type() == type::OBJECT );
         TEST_ASSERT( v2.at( "baz" ).at( 3 ).type() == type::NULL_ );
      }

   } // json

} // tao

#include "main.hh"

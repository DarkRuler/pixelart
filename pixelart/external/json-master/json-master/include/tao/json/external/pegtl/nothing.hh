// Copyright (c) 2014-2015 Dr. Colin Hirsch and Daniel Frey
// Please see LICENSE for license or visit https://github.com/ColinH/PEGTL/

#ifndef TAOCPP_JSON_EMBEDDED_PEGTL_NOTHING_HH
#define TAOCPP_JSON_EMBEDDED_PEGTL_NOTHING_HH

#include <type_traits>

namespace tao_json_pegtl
{
   template< typename Rule > struct nothing {};

   template< template< typename ... > class Action, typename Rule >
   using is_nothing = std::is_base_of< nothing< Rule >, Action< Rule > >;

} // tao_json_pegtl

#endif

#ifndef BOOST_SMART_PTR_DETAIL_SPINLOCK_HPP_INCLUDED
#define BOOST_SMART_PTR_DETAIL_SPINLOCK_HPP_INCLUDED

// MS compatible compilers support #pragma once

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
# pragma once
#endif

//
//  boost/detail/spinlock.hpp
//
//  Copyright (c) 2008 Peter Dimov
//
//  Distributed under the Boost Software License, Version 1.0.
//  See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//
//  struct spinlock
//  {
//      void lock();
//      bool try_lock();
//      void unlock();
//
//      class scoped_lock;
//  };
//
//  #define BOOST_DETAIL_SPINLOCK_INIT <unspecified>
//

#include <pareto_front/boost_subset/config.hpp>
#include <pareto_front/boost_subset/smart_ptr/detail/sp_has_sync.hpp>

#if defined( BOOST_SP_USE_STD_ATOMIC )
# if !defined( __clang__ )
#   include <pareto_front/boost_subset/smart_ptr/detail/spinlock_std_atomic.hpp>
# else
//  Clang (at least up to 3.4) can't compile spinlock_pool when
//  using std::atomic, so substitute the __sync implementation instead.
#   include <pareto_front/boost_subset/smart_ptr/detail/spinlock_sync.hpp>
# endif

#elif defined( BOOST_SP_USE_PTHREADS )
#  include <pareto_front/boost_subset/smart_ptr/detail/spinlock_pt.hpp>

#elif !defined( BOOST_NO_CXX11_HDR_ATOMIC )
#  include <pareto_front/boost_subset/smart_ptr/detail/spinlock_std_atomic.hpp>

#elif defined(__GNUC__) && defined( __arm__ ) && !defined( __thumb__ )
#  include <pareto_front/boost_subset/smart_ptr/detail/spinlock_gcc_arm.hpp>

#elif defined( BOOST_SP_HAS_SYNC )
#  include <pareto_front/boost_subset/smart_ptr/detail/spinlock_sync.hpp>

#elif defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__CYGWIN__)
#  include <pareto_front/boost_subset/smart_ptr/detail/spinlock_w32.hpp>

#elif defined(BOOST_HAS_PTHREADS)
#  include <pareto_front/boost_subset/smart_ptr/detail/spinlock_pt.hpp>

#elif !defined(BOOST_HAS_THREADS)
#  include <pareto_front/boost_subset/smart_ptr/detail/spinlock_nt.hpp>

#else
#  error Unrecognized threading platform
#endif

#endif // #ifndef BOOST_SMART_PTR_DETAIL_SPINLOCK_HPP_INCLUDED

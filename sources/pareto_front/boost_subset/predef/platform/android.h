/*
Copyright Rene Rivera 2015-2019
Distributed under the Boost Software License, Version 1.0.
(See accompanying file LICENSE_1_0.txt or copy at
http://www.boost.org/LICENSE_1_0.txt)
*/

#ifndef BOOST_PREDEF_PLAT_ANDROID_H
#define BOOST_PREDEF_PLAT_ANDROID_H

#include <pareto_front/boost_subset/predef/version_number.h>
#include <pareto_front/boost_subset/predef/make.h>

/*`
[heading `BOOST_PLAT_ANDROID`]

[@http://en.wikipedia.org/wiki/Android_%28operating_system%29 Android] platform.

[table
    [[__predef_symbol__] [__predef_version__]]

    [[`__ANDROID__`] [__predef_detection__]]
    ]
 */

#define BOOST_PLAT_ANDROID BOOST_VERSION_NUMBER_NOT_AVAILABLE

#if defined(__ANDROID__)
#   undef BOOST_PLAT_ANDROID
#   define BOOST_PLAT_ANDROID BOOST_VERSION_NUMBER_AVAILABLE
#endif

#if BOOST_PLAT_ANDROID
#   define BOOST_PLAT_ANDROID_AVAILABLE
#   include <pareto_front/boost_subset/predef/detail/platform_detected.h>
#endif

#define BOOST_PLAT_ANDROID_NAME "Android"

#endif

#include <pareto_front/boost_subset/predef/detail/test.h>
BOOST_PREDEF_DECLARE_TEST(BOOST_PLAT_ANDROID,BOOST_PLAT_ANDROID_NAME)

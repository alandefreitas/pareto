
// (C) Copyright Tobias Schwinger
//
// Use modification and distribution are subject to the boost Software License,
// Version 1.0. (See http://www.boost.org/LICENSE_1_0.txt).

//------------------------------------------------------------------------------

// no include guards, this file is intended for multiple inclusions

#ifdef __WAVE__
// this file has been generated from the master.hpp file in the same directory
#   pragma wave option(preserve: 0)
#endif


#if !BOOST_PP_IS_ITERATING

#   ifndef BOOST_FT_DETAIL_CC_LOOP_MASTER_HPP_INCLUDED
#   define BOOST_FT_DETAIL_CC_LOOP_MASTER_HPP_INCLUDED
#     include <pareto_front/boost_subset/function_types/config/cc_names.hpp>

#     include <pareto_front/boost_subset/preprocessor/cat.hpp>
#     include <pareto_front/boost_subset/preprocessor/seq/size.hpp>
#     include <pareto_front/boost_subset/preprocessor/seq/elem.hpp>
#     include <pareto_front/boost_subset/preprocessor/tuple/elem.hpp>
#     include <pareto_front/boost_subset/preprocessor/iteration/iterate.hpp>
#     include <pareto_front/boost_subset/preprocessor/facilities/expand.hpp>
#     include <pareto_front/boost_subset/preprocessor/arithmetic/inc.hpp>
#   endif

#   include <pareto_front/boost_subset/function_types/detail/encoding/def.hpp>
#   include <pareto_front/boost_subset/function_types/detail/encoding/aliases_def.hpp>

#   define  BOOST_PP_FILENAME_1 \
        <pareto_front/boost_subset/function_types/detail/pp_cc_loop/master.hpp>
#   define  BOOST_PP_ITERATION_LIMITS \
        (0,BOOST_PP_SEQ_SIZE(BOOST_FT_CC_NAMES_SEQ)-1)
#   include BOOST_PP_ITERATE()
#   if !defined(BOOST_FT_config_valid) && BOOST_FT_CC_PREPROCESSING
#     define BOOST_FT_cc_id 1
#     define BOOST_FT_cc_name implicit_cc
#     define BOOST_FT_cc BOOST_PP_EMPTY
#     define BOOST_FT_cond callable_builtin
#     include BOOST_FT_cc_file
#     undef BOOST_FT_cond
#     undef BOOST_FT_cc_name
#     undef BOOST_FT_cc
#     undef BOOST_FT_cc_id
#   elif !defined(BOOST_FT_config_valid) // and generating preprocessed file
BOOST_PP_EXPAND(#) ifndef BOOST_FT_config_valid
BOOST_PP_EXPAND(#)   define BOOST_FT_cc_id 1
BOOST_PP_EXPAND(#)   define BOOST_FT_cc_name implicit_cc
BOOST_PP_EXPAND(#)   define BOOST_FT_cc BOOST_PP_EMPTY
BOOST_PP_EXPAND(#)   define BOOST_FT_cond callable_builtin
#define _()
BOOST_PP_EXPAND(#)   include BOOST_FT_cc_file
#undef _
BOOST_PP_EXPAND(#)   undef BOOST_FT_cond
BOOST_PP_EXPAND(#)   undef BOOST_FT_cc_name
BOOST_PP_EXPAND(#)   undef BOOST_FT_cc
BOOST_PP_EXPAND(#)   undef BOOST_FT_cc_id
BOOST_PP_EXPAND(#) else
BOOST_PP_EXPAND(#)   undef BOOST_FT_config_valid
BOOST_PP_EXPAND(#) endif

#   else
#     undef BOOST_FT_config_valid
#   endif

#   include <pareto_front/boost_subset/function_types/detail/encoding/aliases_undef.hpp>
#   include <pareto_front/boost_subset/function_types/detail/encoding/undef.hpp>

#elif BOOST_FT_CC_PREPROCESSING

#   define BOOST_FT_cc_id  BOOST_PP_INC(BOOST_PP_FRAME_ITERATION(1))
#   define BOOST_FT_cc_inf \
        BOOST_PP_SEQ_ELEM(BOOST_PP_FRAME_ITERATION(1),BOOST_FT_CC_NAMES_SEQ)

#   define BOOST_FT_cc_pp_name BOOST_PP_TUPLE_ELEM(3,0,BOOST_FT_cc_inf)
#   define BOOST_FT_cc_name    BOOST_PP_TUPLE_ELEM(3,1,BOOST_FT_cc_inf)
#   define BOOST_FT_cc         BOOST_PP_TUPLE_ELEM(3,2,BOOST_FT_cc_inf)

#   define BOOST_FT_cond BOOST_PP_CAT(BOOST_FT_CC_,BOOST_FT_cc_pp_name)

#   if BOOST_FT_cond
#     define BOOST_FT_config_valid 1
#     include BOOST_FT_cc_file
#   endif

#   undef BOOST_FT_cond

#   undef BOOST_FT_cc_pp_name
#   undef BOOST_FT_cc_name
#   undef BOOST_FT_cc

#   undef BOOST_FT_cc_id
#   undef BOOST_FT_cc_inf

#else // if generating preprocessed file
BOOST_PP_EXPAND(#) define BOOST_FT_cc_id BOOST_PP_INC(BOOST_PP_ITERATION())

#   define BOOST_FT_cc_inf \
        BOOST_PP_SEQ_ELEM(BOOST_PP_ITERATION(),BOOST_FT_CC_NAMES_SEQ)

#   define BOOST_FT_cc_pp_name BOOST_PP_TUPLE_ELEM(3,0,BOOST_FT_cc_inf)

#   define BOOST_FT_CC_DEF(name,index) \
        name BOOST_PP_TUPLE_ELEM(3,index,BOOST_FT_cc_inf)
BOOST_PP_EXPAND(#) define BOOST_FT_CC_DEF(BOOST_FT_cc_name,1)
BOOST_PP_EXPAND(#) define BOOST_FT_CC_DEF(BOOST_FT_cc,2)
#   undef  BOOST_FT_CC_DEF

#   define BOOST_FT_cc_cond_v BOOST_PP_CAT(BOOST_FT_CC_,BOOST_FT_cc_pp_name)
BOOST_PP_EXPAND(#) define BOOST_FT_cond BOOST_FT_cc_cond_v
#   undef  BOOST_FT_cc_cond_v

#   undef BOOST_FT_cc_pp_name
#   undef BOOST_FT_cc_inf

BOOST_PP_EXPAND(#) if BOOST_FT_cond
BOOST_PP_EXPAND(#)   define BOOST_FT_config_valid 1
#define _()
BOOST_PP_EXPAND(#)   include BOOST_FT_cc_file
#undef _
BOOST_PP_EXPAND(#) endif

BOOST_PP_EXPAND(#) undef BOOST_FT_cond

BOOST_PP_EXPAND(#) undef BOOST_FT_cc_name
BOOST_PP_EXPAND(#) undef BOOST_FT_cc

BOOST_PP_EXPAND(#) undef BOOST_FT_cc_id

#endif


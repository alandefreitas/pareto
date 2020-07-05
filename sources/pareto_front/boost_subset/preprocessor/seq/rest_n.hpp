# /* **************************************************************************
#  *                                                                          *
#  *     (C) Copyright Paul Mensonides 2002.
#  *     Distributed under the Boost Software License, Version 1.0. (See
#  *     accompanying file LICENSE_1_0.txt or copy at
#  *     http://www.boost.org/LICENSE_1_0.txt)
#  *                                                                          *
#  ************************************************************************** */
#
# /* See http://www.boost.org for most recent version. */
#
# ifndef BOOST_PREPROCESSOR_SEQ_REST_N_HPP
# define BOOST_PREPROCESSOR_SEQ_REST_N_HPP
#
# include <pareto_front/boost_subset/preprocessor/arithmetic/inc.hpp>
# include <pareto_front/boost_subset/preprocessor/comparison/not_equal.hpp>
# include <pareto_front/boost_subset/preprocessor/config/config.hpp>
# include <pareto_front/boost_subset/preprocessor/control/expr_iif.hpp>
# include <pareto_front/boost_subset/preprocessor/facilities/identity.hpp>
# include <pareto_front/boost_subset/preprocessor/logical/bitand.hpp>
# include <pareto_front/boost_subset/preprocessor/seq/detail/is_empty.hpp>
# include <pareto_front/boost_subset/preprocessor/seq/detail/split.hpp>
# include <pareto_front/boost_subset/preprocessor/tuple/elem.hpp>
#
# /* BOOST_PP_SEQ_REST_N */
#
# if ~BOOST_PP_CONFIG_FLAGS() & BOOST_PP_CONFIG_EDG()
#    define BOOST_PP_SEQ_REST_N(n, seq) BOOST_PP_SEQ_REST_N_DETAIL_EXEC(n, seq, BOOST_PP_SEQ_DETAIL_EMPTY_SIZE(seq))
# else
#    define BOOST_PP_SEQ_REST_N(n, seq) BOOST_PP_SEQ_REST_N_I(n, seq)
#    define BOOST_PP_SEQ_REST_N_I(n, seq) BOOST_PP_SEQ_REST_N_DETAIL_EXEC(n, seq, BOOST_PP_SEQ_DETAIL_EMPTY_SIZE(seq))
# endif
#
#    define BOOST_PP_SEQ_REST_N_DETAIL_EXEC(n, seq, size) \
		BOOST_PP_EXPR_IIF \
			( \
			BOOST_PP_BITAND \
				( \
				BOOST_PP_SEQ_DETAIL_IS_NOT_EMPTY_SIZE(size), \
				BOOST_PP_NOT_EQUAL(n,size) \
				), \
			BOOST_PP_TUPLE_ELEM(2, 1, BOOST_PP_SEQ_SPLIT(BOOST_PP_INC(n), BOOST_PP_IDENTITY( (nil) seq )))() \
			) \
/**/
#
# endif

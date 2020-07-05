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
# ifndef BOOST_PREPROCESSOR_SEQ_REPLACE_HPP
# define BOOST_PREPROCESSOR_SEQ_REPLACE_HPP
#
# include <pareto_front/boost_subset/preprocessor/arithmetic/dec.hpp>
# include <pareto_front/boost_subset/preprocessor/arithmetic/inc.hpp>
# include <pareto_front/boost_subset/preprocessor/config/config.hpp>
# include <pareto_front/boost_subset/preprocessor/comparison/equal.hpp>
# include <pareto_front/boost_subset/preprocessor/control/iif.hpp>
# include <pareto_front/boost_subset/preprocessor/seq/first_n.hpp>
# include <pareto_front/boost_subset/preprocessor/seq/rest_n.hpp>
# include <pareto_front/boost_subset/preprocessor/seq/size.hpp>
#
# /* BOOST_PP_SEQ_REPLACE */
#
# if ~BOOST_PP_CONFIG_FLAGS() & BOOST_PP_CONFIG_EDG()
#    define BOOST_PP_SEQ_REPLACE(seq, i, elem) BOOST_PP_SEQ_FIRST_N(i, seq) (elem) BOOST_PP_SEQ_REPLACE_DETAIL_REST(seq, i)
# else
#    define BOOST_PP_SEQ_REPLACE(seq, i, elem) BOOST_PP_SEQ_REPLACE_I(seq, i, elem)
#    define BOOST_PP_SEQ_REPLACE_I(seq, i, elem) BOOST_PP_SEQ_FIRST_N(i, seq) (elem) BOOST_PP_SEQ_REPLACE_DETAIL_REST(seq, i)
# endif
#
#    define BOOST_PP_SEQ_REPLACE_DETAIL_REST_EMPTY(seq, i)
#    define BOOST_PP_SEQ_REPLACE_DETAIL_REST_VALID(seq, i) BOOST_PP_SEQ_REST_N(BOOST_PP_INC(i), seq)
#    define BOOST_PP_SEQ_REPLACE_DETAIL_REST(seq, i) \
		BOOST_PP_IIF \
			( \
			BOOST_PP_EQUAL(i,BOOST_PP_DEC(BOOST_PP_SEQ_SIZE(seq))), \
			BOOST_PP_SEQ_REPLACE_DETAIL_REST_EMPTY, \
			BOOST_PP_SEQ_REPLACE_DETAIL_REST_VALID \
			) \
		(seq, i) \
/**/
#
# endif

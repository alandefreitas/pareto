
#ifndef BOOST_MPL_LOWER_BOUND_HPP_INCLUDED
#define BOOST_MPL_LOWER_BOUND_HPP_INCLUDED

// Copyright Aleksey Gurtovoy 2001-2004
//
// Distributed under the Boost Software License, Version 1.0. 
// (See accompanying file LICENSE_1_0.txt or copy at 
// http://www.boost.org/LICENSE_1_0.txt)
//
// See http://www.boost.org/libs/mpl for documentation.

// $Id$
// $Date$
// $Revision$

#include <pareto_front/boost_subset/mpl/less.hpp>
#include <pareto_front/boost_subset/mpl/lambda.hpp>
#include <pareto_front/boost_subset/mpl/aux_/na_spec.hpp>
#include <pareto_front/boost_subset/mpl/aux_/config/workaround.hpp>

#if BOOST_WORKAROUND(__BORLANDC__, BOOST_TESTED_AT(0x610))
#   define BOOST_MPL_CFG_STRIPPED_DOWN_LOWER_BOUND_IMPL
#endif

#if !defined(BOOST_MPL_CFG_STRIPPED_DOWN_LOWER_BOUND_IMPL)
#   include <pareto_front/boost_subset/mpl/minus.hpp>
#   include <pareto_front/boost_subset/mpl/divides.hpp>
#   include <pareto_front/boost_subset/mpl/size.hpp>
#   include <pareto_front/boost_subset/mpl/advance.hpp>
#   include <pareto_front/boost_subset/mpl/begin_end.hpp>
#   include <pareto_front/boost_subset/mpl/long.hpp>
#   include <pareto_front/boost_subset/mpl/eval_if.hpp>
#   include <pareto_front/boost_subset/mpl/prior.hpp>
#   include <pareto_front/boost_subset/mpl/deref.hpp>
#   include <pareto_front/boost_subset/mpl/apply.hpp>
#   include <pareto_front/boost_subset/mpl/aux_/value_wknd.hpp>
#else
#   include <pareto_front/boost_subset/mpl/not.hpp>
#   include <pareto_front/boost_subset/mpl/find.hpp>
#   include <pareto_front/boost_subset/mpl/bind.hpp>
#endif

#include <pareto_front/boost_subset/config.hpp>

namespace boost { namespace mpl {

#if defined(BOOST_MPL_CFG_STRIPPED_DOWN_LOWER_BOUND_IMPL)

// agurt 23/oct/02: has a wrong complexity etc., but at least it works
// feel free to contribute a better implementation!
template<
      typename BOOST_MPL_AUX_NA_PARAM(Sequence)
    , typename BOOST_MPL_AUX_NA_PARAM(T)
    , typename Predicate = less<>
    , typename pred_ = typename lambda<Predicate>::type
    >
struct lower_bound
    : find_if< Sequence, bind1< not_<>, bind2<pred_,_,T> > >
{
};

#else

namespace aux {

template<
      typename Distance
    , typename Predicate
    , typename T
    , typename DeferredIterator
    >
struct lower_bound_step_impl;

template< 
      typename Distance
    , typename Predicate
    , typename T
    , typename DeferredIterator
    >
struct lower_bound_step
{
    typedef typename eval_if<
          Distance
        , lower_bound_step_impl<Distance,Predicate,T,DeferredIterator>
        , DeferredIterator
        >::type type;
};
    
template<
      typename Distance
    , typename Predicate
    , typename T
    , typename DeferredIterator
    >
struct lower_bound_step_impl
{
    typedef typename divides< Distance, long_<2> >::type offset_;
    typedef typename DeferredIterator::type iter_;
    typedef typename advance< iter_,offset_ >::type middle_;
    typedef typename apply2<
              Predicate
            , typename deref<middle_>::type
            , T
            >::type cond_;

    typedef typename prior< minus< Distance, offset_> >::type step_;
    typedef lower_bound_step< offset_,Predicate,T,DeferredIterator > step_forward_;
    typedef lower_bound_step< step_,Predicate,T,next<middle_> > step_backward_;
    typedef typename eval_if<
          cond_
        , step_backward_
        , step_forward_
        >::type type;
};


} // namespace aux

template<
      typename BOOST_MPL_AUX_NA_PARAM(Sequence)
    , typename BOOST_MPL_AUX_NA_PARAM(T)
    , typename Predicate = less<>
    >
struct lower_bound
{
 private:
    typedef typename lambda<Predicate>::type pred_;
    typedef typename size<Sequence>::type size_;

 public:
    typedef typename aux::lower_bound_step<
        size_,pred_,T,begin<Sequence>
        >::type type;
};

#endif // BOOST_MPL_CFG_STRIPPED_DOWN_LOWER_BOUND_IMPL

BOOST_MPL_AUX_NA_SPEC(2, lower_bound)

}}

#endif // BOOST_MPL_LOWER_BOUND_HPP_INCLUDED

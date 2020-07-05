/*
 [auto_generated]
 boost/numeric/odeint.hpp

 [begin_description]
 Forward include for odeint. Includes nearly everything.
 [end_description]

 Copyright 2009-2013 Karsten Ahnert
 Copyright 2010-2013 Mario Mulansky

 Distributed under the Boost Software License, Version 1.0.
 (See accompanying file LICENSE_1_0.txt or
 copy at http://www.boost.org/LICENSE_1_0.txt)
 */


#ifndef BOOST_NUMERIC_ODEINT_HPP_INCLUDED
#define BOOST_NUMERIC_ODEINT_HPP_INCLUDED

#include <pareto_front/boost_subset/numeric/odeint/version.hpp>
#include <pareto_front/boost_subset/numeric/odeint/config.hpp>

// start with ublas wrapper because we need its specializations before including state_wrapper.hpp
#include <pareto_front/boost_subset/numeric/odeint/util/ublas_wrapper.hpp>

#include <pareto_front/boost_subset/numeric/odeint/stepper/euler.hpp>
#include <pareto_front/boost_subset/numeric/odeint/stepper/runge_kutta4_classic.hpp>
#include <pareto_front/boost_subset/numeric/odeint/stepper/runge_kutta4.hpp>
#include <pareto_front/boost_subset/numeric/odeint/stepper/runge_kutta_cash_karp54.hpp>
#include <pareto_front/boost_subset/numeric/odeint/stepper/runge_kutta_cash_karp54_classic.hpp>
#include <pareto_front/boost_subset/numeric/odeint/stepper/runge_kutta_dopri5.hpp>
#include <pareto_front/boost_subset/numeric/odeint/stepper/runge_kutta_fehlberg78.hpp>

#include <pareto_front/boost_subset/numeric/odeint/stepper/controlled_runge_kutta.hpp>

#include <pareto_front/boost_subset/numeric/odeint/stepper/dense_output_runge_kutta.hpp>

#include <pareto_front/boost_subset/numeric/odeint/stepper/bulirsch_stoer.hpp>

#ifndef __CUDACC__
/* Bulirsch Stoer with Dense Output does not compile with nvcc
 * because of the binomial library used there which relies on unsupported SSE functions
 */
#include <pareto_front/boost_subset/numeric/odeint/stepper/bulirsch_stoer_dense_out.hpp>
#endif

#include <pareto_front/boost_subset/numeric/odeint/stepper/symplectic_euler.hpp>
#include <pareto_front/boost_subset/numeric/odeint/stepper/symplectic_rkn_sb3a_mclachlan.hpp>
#include <pareto_front/boost_subset/numeric/odeint/stepper/velocity_verlet.hpp>

#include <pareto_front/boost_subset/numeric/odeint/stepper/adams_bashforth_moulton.hpp>

#include <pareto_front/boost_subset/numeric/odeint/stepper/adaptive_adams_bashforth_moulton.hpp>
#include <pareto_front/boost_subset/numeric/odeint/stepper/controlled_adams_bashforth_moulton.hpp>

#include <pareto_front/boost_subset/numeric/odeint/stepper/implicit_euler.hpp>
#include <pareto_front/boost_subset/numeric/odeint/stepper/rosenbrock4.hpp>
#include <pareto_front/boost_subset/numeric/odeint/stepper/rosenbrock4_controller.hpp>
#include <pareto_front/boost_subset/numeric/odeint/stepper/rosenbrock4_dense_output.hpp>

#include <pareto_front/boost_subset/numeric/odeint/algebra/algebra_dispatcher.hpp>
#include <pareto_front/boost_subset/numeric/odeint/algebra/multi_array_algebra.hpp>
#include <pareto_front/boost_subset/numeric/odeint/util/multi_array_adaption.hpp>
/*
 * Including this algebra slows down the compilation time
 */
// #include <pareto_front/boost_subset/numeric/odeint/algebra/fusion_algebra.hpp>

#include <pareto_front/boost_subset/numeric/odeint/integrate/integrate.hpp>
#include <pareto_front/boost_subset/numeric/odeint/integrate/integrate_adaptive.hpp>
#include <pareto_front/boost_subset/numeric/odeint/integrate/integrate_const.hpp>
#include <pareto_front/boost_subset/numeric/odeint/integrate/integrate_n_steps.hpp>
#include <pareto_front/boost_subset/numeric/odeint/integrate/integrate_times.hpp>

#include <pareto_front/boost_subset/numeric/odeint/integrate/observer_collection.hpp>
#include <pareto_front/boost_subset/numeric/odeint/integrate/max_step_checker.hpp>

#include <pareto_front/boost_subset/numeric/odeint/stepper/generation.hpp>

#include <pareto_front/boost_subset/numeric/odeint/iterator/adaptive_iterator.hpp>
#include <pareto_front/boost_subset/numeric/odeint/iterator/adaptive_time_iterator.hpp>
#include <pareto_front/boost_subset/numeric/odeint/iterator/const_step_iterator.hpp>
#include <pareto_front/boost_subset/numeric/odeint/iterator/const_step_time_iterator.hpp>


#endif // BOOST_NUMERIC_ODEINT_HPP_INCLUDED

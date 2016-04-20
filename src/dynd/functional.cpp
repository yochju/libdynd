//
// Copyright (C) 2011-16 DyND Developers
// BSD 2-Clause License, see LICENSE.txt
//

#include <dynd/callables/adapt_callable.hpp>
#include <dynd/callables/compose_callable.hpp>
#include <dynd/callables/compound_callable.hpp>
#include <dynd/callables/constant_callable.hpp>
#include <dynd/callables/elwise_entry_callable.hpp>
#include <dynd/callables/neighborhood_callable.hpp>
#include <dynd/callables/outer_callable.hpp>
#include <dynd/callables/reduction_callable.hpp>
#include <dynd/callables/state_callable.hpp>
#include <dynd/callables/where_callable.hpp>
#include <dynd/functional.hpp>
#include <dynd/types/ellipsis_dim_type.hpp>

using namespace std;
using namespace dynd;

const nd::callable &nd::get_elwise() {
  static callable elwise = make_callable<functional::elwise_entry_callable>(false);
  return elwise;
}

DYND_API nd::callable nd::elwise = nd::get_elwise();

nd::callable nd::functional::adapt(const ndt::type &value_tp, const callable &forward) {
  return make_callable<adapt_callable>(value_tp, forward);
}

nd::callable nd::functional::compose(const nd::callable &first, const nd::callable &second, const ndt::type &buf_tp) {
  if (first.get_type()->get_npos() != 1) {
    throw runtime_error("Multi-parameter callable chaining is not implemented");
  }

  if (second.get_type()->get_npos() != 1) {
    stringstream ss;
    ss << "Cannot chain functions " << first << " and " << second << ", because the second function is not unary";
    throw invalid_argument(ss.str());
  }

  if (buf_tp.get_id() == uninitialized_id) {
    throw runtime_error("Chaining functions without a provided intermediate "
                        "type is not implemented");
  }

  /* // TODO: Something like this should work
  map<nd::string, ndt::type> tp_vars;
  second.get_type()->get_pos_type(0).match(first.get_type()->get_return_type(),
                                           tp_vars);
  ndt::type return_tp =
      ndt::substitute(second.get_type()->get_return_type(), tp_vars, false);
  */

  return make_callable<compose_callable>(
      ndt::make_type<ndt::callable_type>(second.get_type()->get_return_type(), first.get_type()->get_pos_types()),
      first, second, buf_tp);
}

nd::callable nd::functional::constant(const array &val) { return make_callable<constant_callable>(val); }

nd::callable nd::functional::left_compound(const callable &child) {
  vector<ndt::type> pos_types = child.get_type()->get_pos_types();
  pos_types.resize(1);

  return make_callable<left_compound_callable>(
      ndt::make_type<ndt::callable_type>(child.get_type()->get_return_type(), pos_types), // head element or empty
      child);
}

nd::callable nd::functional::right_compound(const callable &child) {
  vector<ndt::type> pos_types = child.get_type()->get_pos_types();
  pos_types.erase(pos_types.begin());

  return make_callable<right_compound_callable>(ndt::make_type<ndt::callable_type>(child.get_type()->get_return_type(),
                                                                                   pos_types), // tail elements or empty
                                                child);
}

ndt::type nd::functional::elwise_make_type(const ndt::callable_type *child_tp, bool ret_variadic) {
  const std::vector<ndt::type> &param_types = child_tp->get_pos_types();
  std::vector<ndt::type> out_param_types;
  std::string dimsname("Dims");

  for (const ndt::type &t : param_types) {
    out_param_types.push_back(ndt::make_ellipsis_dim(dimsname, t));
  }

  ndt::type kwd_tp = child_tp->get_kwd_struct();

  const ndt::type &ret_tp = child_tp->get_return_type();
  if (ret_variadic) {
    return ndt::make_type<ndt::callable_type>(
        ndt::make_ellipsis_dim(dimsname, ret_tp),
        ndt::make_type<ndt::tuple_type>(out_param_types.size(), out_param_types.data()), kwd_tp);
  }

  return ndt::make_type<ndt::callable_type>(
      ret_tp, ndt::make_type<ndt::tuple_type>(out_param_types.size(), out_param_types.data()), kwd_tp);
}

nd::callable nd::functional::elwise(const callable &child, bool res_ignore) {
  ndt::type f_tp = elwise_make_type(child.get_type(), !res_ignore);

  size_t i;
  bool state = false;
  std::vector<ndt::type> arg_tp;
  for (size_t j = 0; j < f_tp.extended<ndt::callable_type>()->get_npos(); ++j) {
    const auto &tp = f_tp.extended<ndt::callable_type>()->get_pos_type(j);
    if (tp.get_dtype().get_id() == state_id) {
      i = j;
      state = true;
    } else {
      arg_tp.push_back(tp);
    }
  }

  nd::callable f = make_callable<elwise_entry_callable>(f_tp, child, res_ignore);

  if (state) {
    ndt::type tp = ndt::make_type<ndt::callable_type>(f.get_ret_type(), arg_tp);
    return make_callable<state_callable>(tp.extended<ndt::callable_type>()->get_npos(), tp, f, i);
  }

  return f;
}

nd::callable nd::functional::outer(const callable &child) {
  const ndt::type &self_tp = outer_make_type(child.get_type());
  switch (self_tp.extended<ndt::callable_type>()->get_npos()) {
  case 0:
    return make_callable<outer_callable<0>>(self_tp, child);
  case 1:
    return make_callable<outer_callable<1>>(self_tp, child);
  case 2:
    return make_callable<outer_callable<2>>(self_tp, child);
  case 3:
    return make_callable<outer_callable<3>>(self_tp, child);
  case 4:
    return make_callable<outer_callable<4>>(self_tp, child);
  case 5:
    return make_callable<outer_callable<5>>(self_tp, child);
  case 6:
    return make_callable<outer_callable<6>>(self_tp, child);
  case 7:
    return make_callable<outer_callable<7>>(self_tp, child);
  default:
    throw std::runtime_error("callable with nsrc > 7 not implemented yet");
  }
}

ndt::type nd::functional::outer_make_type(const ndt::callable_type *child_tp) {
  const std::vector<ndt::type> &param_types = child_tp->get_pos_types();
  std::vector<ndt::type> out_param_types;

  for (intptr_t i = 0, i_end = child_tp->get_npos(); i != i_end; ++i) {
    std::string dimsname("Dims" + std::to_string(i));
    out_param_types.push_back(ndt::make_ellipsis_dim(dimsname, param_types[i]));
  }

  ndt::type kwd_tp = child_tp->get_kwd_struct();

  ndt::type ret_tp = child_tp->get_return_type();
  ret_tp = ndt::make_ellipsis_dim("Dims", child_tp->get_return_type());

  return ndt::make_type<ndt::callable_type>(
      ret_tp, ndt::make_type<ndt::tuple_type>(out_param_types.size(), out_param_types.data()), kwd_tp);
}

nd::callable nd::functional::neighborhood(const callable &neighborhood_op, const callable &boundary_child) {
  const ndt::callable_type *funcproto_tp = neighborhood_op.get_array_type().extended<ndt::callable_type>();

  intptr_t nh_ndim = funcproto_tp->get_pos_type(0).get_ndim();
  std::vector<ndt::type> arg_tp(2);
  arg_tp[0] = ndt::type("?" + std::to_string(nh_ndim) + " * int");
  arg_tp[1] = ndt::type("?" + std::to_string(nh_ndim) + " * int");

  return make_callable<neighborhood_callable<1>>(
      ndt::make_type<ndt::callable_type>(
          funcproto_tp->get_pos_type(0).with_replaced_dtype(funcproto_tp->get_return_type()),
          funcproto_tp->get_pos_tuple(),
          ndt::make_type<ndt::struct_type>(std::vector<std::string>{"shape", "offset"}, arg_tp)),
      neighborhood_op, boundary_child);
}

nd::callable nd::functional::reduction(const callable &child) {
  if (child.is_null()) {
    throw invalid_argument("'child' cannot be null");
  }

  switch (child.get_narg()) {
  case 1:
    break;
  case 2:
    return reduction((child.get_flags() | right_associative) ? left_compound(child) : right_compound(child));
  default: {
    stringstream ss;
    ss << "'child' must be a unary callable, but its signature is " << child.get_array_type();
    throw invalid_argument(ss.str());
  }
  }

  return make_callable<reduction_dispatch_callable>(
      ndt::make_type<ndt::callable_type>(ndt::ellipsis_dim_type::make_if_not_variadic(child.get_ret_type()),
                                         {ndt::ellipsis_dim_type::make_if_not_variadic(child.get_arg_type(0))},
                                         {{ndt::make_type<ndt::option_type>(ndt::type("Fixed * int32")), "axes"},
                                          {ndt::make_type<ndt::option_type>(child.get_ret_type()), "identity"},
                                          {ndt::make_type<ndt::option_type>(ndt::make_type<bool1>()), "keepdims"}}),
      child);
}

nd::callable nd::functional::where(const callable &child) { return elwise(make_callable<where_callable>(child), true); }

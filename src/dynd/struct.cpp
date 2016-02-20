//
// Copyright (C) 2011-15 DyND Developers
// BSD 2-Clause License, see LICENSE.txt
//

#include <dynd/functional.hpp>
#include <dynd/array.hpp>
#include <dynd/struct.hpp>
#include <dynd/callable.hpp>
#include <dynd/kernels/field_access_kernel.hpp>
#include <dynd/types/callable_type.hpp>

using namespace std;
using namespace dynd;

DYND_API nd::callable nd::field_access::make() { return callable::make<field_access_kernel>(); }

DYND_API struct nd::field_access nd::field_access;

// Temporary solution until nd::field_access() handles views.
DYND_API nd::callable nd::make_field_access_kernel(const ndt::type &dt, const std::string &name)
{
  assert(dt.get_id() == struct_id);
  intptr_t i = dt.extended<ndt::struct_type>()->get_field_index(name);

  if (i < 0) {
    throw std::invalid_argument("no field named '" + name + "'");
  }

  ndt::type ct = ndt::callable_type::make(ndt::type("Any"), ndt::tuple_type::make(), ndt::struct_type::make({"self"}));
  nd::callable c = nd::callable::make<nd::get_array_field_kernel>(ct, i);

  return c;
}

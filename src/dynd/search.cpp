//
// Copyright (C) 2011-15 DyND Developers
// BSD 2-Clause License, see LICENSE.txt
//

#include <dynd/callables/binary_search_callable.hpp>
#include <dynd/search.hpp>
#include <dynd/types/fixed_dim_type.hpp>

using namespace std;
using namespace dynd;

DYND_API nd::callable nd::binary_search::make() { return make_callable<binary_search_callable>(); }

DYND_DEFAULT_DECLFUNC_GET(nd::binary_search)

DYND_API struct nd::binary_search nd::binary_search;

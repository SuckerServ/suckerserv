/*   
 *   The Fungu Scripting Engine
 *   
 *   Copyright (c) 2008-2009 Graham Daws.
 *
 *   Distributed under a BSD style license (see accompanying file LICENSE.txt)
 */
#ifndef FUNGU_SCRIPT_CAST_HPP
#define FUNGU_SCRIPT_CAST_HPP

#include "type_id.hpp"

namespace fungu{
namespace script{

void cast(void *, const type_id &, void *, const type_id &);

} //namespace script
} //namespace fungu

#endif

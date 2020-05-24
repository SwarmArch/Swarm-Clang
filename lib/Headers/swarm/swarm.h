/*===---- swarm/swarm.h - Manual Swarm task programming --------------------===
 *
 *                       The SCC Parallelizing Compiler
 *
 *          Copyright (c) 2020 Massachusetts Institute of Technology
 *
 * This file is distributed under the University of Illinois Open Source
 * License. See LICENSE.TXT for details.
 *
 *===-----------------------------------------------------------------------===
 *
 *  This header file defines nicer aliases of the Swarm keywords,
 *  following the practices of Cilk Plus.
 *
 *  Since Swarm features supported by this compiler are nonstandard extensions
 *  to both C and C++, the Swarm keywords all begin with "_Swarm_", which
 *  guarantees that they will not conflict with user-defined identifiers in
 *  properly written programs, since both the C and C++ standards reserve
 *  identifiers beginning with an underscore followed by an uppercase letter
 *  for internal use by compilers and standard library implementations.
 *
 *  However, this means that the keywords look like something grafted on to
 *  the base language. Therefore, you can include this header:
 *
 *      #include "swarm/swarm.h"
 *
 *  and then write the Swarm keywords with a "swarm_" prefix instead of
 *  "_Swarm_".
 *
 *===-----------------------------------------------------------------------===
 */

#ifndef swarm_spawn
#define swarm_spawn _Swarm_spawn
#define swarm_spawn_sub _Swarm_spawn_sub
#define swarm_spawn_super _Swarm_spawn_super
#define swarm_for _Swarm_for
#endif

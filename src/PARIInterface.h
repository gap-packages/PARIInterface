/*
 * PARIInterface: Interface to PARI
 *
 *
 *  Copyright (C) 2018-2019
 *    Bill Allombert <bill.allombert@math.u-bordeaux.fr>
 *    Vincent Delecroix <vincent.delecroix@math.cnrs.fr>
 *    Markus Pfeiffer <markus.pfeiffer@morphism.de>
 *
 * Licensed under the GPL 2 or later.
 */

#ifndef _PARIInterface_H
#define _PARIInterface_H

#include <pari/pari.h>
#include "src/compiled.h"          /* GAP headers */

#include <gmp.h>                   /* mp_limb_t :/ */

#define PARI_T_GEN 0          // Generic PARI object

#define PARI_DAT_WORDS (sizeof(Obj) * 4)
#define PARI_DAT_TYPE(obj)         ((Int)(CONST_ADDR_OBJ(obj)[1]))
#define SET_PARI_DAT_TYPE(obj, t)  (ADDR_OBJ(obj)[1] = (Obj)(t))
#define PARI_DAT_GEN(obj)          ((GEN)(CONST_ADDR_OBJ(obj)[2]))
#define SET_PARI_DAT_GEN(obj, g)   (ADDR_OBJ(obj)[2] = (Obj)(g))

// Type checking
int IS_PARI_OBJ(Obj o);

#define RequirePARIObj(funcname, op)                                             \
    RequireArgumentCondition(funcname, op, IS_PARI_OBJ(op), "must be a PARI object")

// Conversions from PARI GEN to corresponding GAP Obj
Obj NewPARIGEN(GEN data);      // create a GAP Obj wrapping a GEN
                               // (no copy)

Obj PariGENToObj(GEN v);       // generic dispatch (C level)
Obj PariVecToList(GEN v);
Obj PariVecSmallToList(GEN v);
Obj PariVecToList(GEN v);
Obj PariIntToIntObj(GEN v);
Obj PariFracToRatObj(GEN v);
Obj PariPolToList(GEN v);

// Conversions from GAP Object to PARI GEN
GEN ObjToPariGEN(Obj obj);    // generic dispatch (C level)
GEN ListToPariVec(Obj list);
GEN IntToPariGEN(Obj o);
GEN RatToPariGEN(Obj rat);
GEN ListToPariVecsmall(Obj list);
GEN ListToPariVecVecsmall(Obj list);
GEN ListToPariVecVecVecsmall(Obj list);
GEN CoeffListToPariGEN(Obj poly);

// Some PARI functions
Obj FuncPARI_output(Obj self, Obj x);
Obj FuncPARI_gcdii(Obj self, Obj x, Obj y);
Obj FuncPARI_polgalois(Obj self, Obj poly);
Obj FuncPARI_POL_FACTOR_MOD_P(Obj self, Obj poly, Obj p);

#endif

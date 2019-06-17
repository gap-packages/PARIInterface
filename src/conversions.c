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

#define _GNU_SOURCE     // for RTLD_DEFAULT on Linux

#include "PARIInterface.h"

/* TODO */
/* static Obj FuncPARI_INTMOD(Obj self, ?); */
/* static Obj FuncPARI_POLMOD(Obj self, ?); */

//
// Conversions from PARI GEN to GAP Obj
//

Obj PariVecToList(GEN v)
{
    Int len = lg(v);
    Obj res = NEW_PLIST(T_PLIST, len - 1);
    SET_LEN_PLIST(res, len - 1);

    for(Int i = 1; i < len; i++) {
        SET_ELM_PLIST(res, i, PariGENToObj(gel(v, i)));
        CHANGED_BAG(res);
    }
    return res;
}

Obj PariVecSmallToList(GEN v)
{
    Int len = lg(v);
    Obj res = NEW_PLIST(T_PLIST, len - 1);
    SET_LEN_PLIST(res, len - 1);

    for(Int i = 1; i < len; i++) {
        SET_ELM_PLIST(res, i, ObjInt_Int(v[i]));
        CHANGED_BAG(res);
    }
    return res;
}

#ifdef PARI_KERNEL_NONE
static void
xmpn_mirror(ulong *x, long n)
{
  long i;
  for(i = 0; i < (n>>1); i++)
  {
    ulong m=x[i];
    x[i]=x[n-1-i];
    x[n-1-i]=m;
  }
}
#define GMP_TO_PARI(a,b) xmpn_mirror(a, b);
#define PARI_TO_GMP(a,b) xmpn_mirror(a, b);
#else
#define GMP_TO_PARI(a,b)
#define PARI_TO_GMP(a,b)
#endif


Obj PariIntToIntObj(GEN v)
{
    long size;
    Obj res;

    if (typ (v) != t_INT)
        ErrorQuit("v has to be a PARI t_INT", 0L, 0L);

    size = signe(v) * (lgefint (v) - 2);

    PARI_TO_GMP(v+2, lgefint (v) - 2)

    res = MakeObjInt((const UInt *)(v+2), size);

    GMP_TO_PARI(v+2, lgefint (v) - 2)

    return res;
}

Obj PariFracToRatObj(GEN v)
{
    Obj num = PariGENToObj(gel(v, 1));
    Obj den = PariGENToObj(gel(v, 2));

    if (den == INTOBJ_INT(1))
        return num;

    Obj res = NewBag(T_RAT, 2 * sizeof(Obj));
    SET_NUM_RAT(res, PariGENToObj(gel(v, 1)));
    SET_DEN_RAT(res, PariGENToObj(gel(v, 2)));

    return res;
}

Obj PariPolToList(GEN v)
{
    Int len = lg(v);
    Obj res = NEW_PLIST(T_PLIST, len - 2);
    SET_LEN_PLIST(res, len - 2);
    for(Int i = 2; i<len; i++) {
        SET_ELM_PLIST(res, i-1, PariGENToObj(gel(v,i)));
        CHANGED_BAG(res);
    }
    return res;
}

// Main dispatch PARI GEN -> Gap object
// When there's no immediately obvious object to convert to
// We currently convert to a list of subobjects
// Some of this should probably be done on the GAP Level
Obj PariGENToObj(GEN v)
{
    Obj res;
    switch (typ(v)) {
    case t_INT:       // Integer
        return PariIntToIntObj(v);
    case t_COL:       // Column Vector
    case t_VEC:       // Row Vector
        return PariVecToList(v);
    case t_VECSMALL:  // Vector of small integers
        return PariVecSmallToList(v);
    case t_STR:       // String
        return MakeString(GSTR(v));
    case t_INTMOD:    // Int mod Modulus
        return PariVecToList(v);
    case t_FRAC:      // Fraction
        return PariFracToRatObj(v);
    case t_POLMOD:    // Polynomial mod modulus
        return PariVecToList(v);
    case t_POL:       // Polynomial
        return PariPolToList(v);
    case t_MAT:       // Matrix
        return PariVecToList(v);
    case t_FFELT:     // Finite field element
    case t_SER:       // Power series
    case t_RFRAC:     // Rational function
    case t_PADIC:     // p-adic numbers
    case t_QUAD:      // quadratic numbers
    default:
        // TODO: Find names for the types
        ErrorQuit("PariGENToObj: not a supported type %i", typ(v), 0L);
        break;
    }
    return res;
}

//
// Conversions from GAP Obj to PARI GEN
//

GEN RatToPariGEN(Obj rat)
{
    GEN x = cgetg(3, t_FRAC);

    if ( TNUM_OBJ(rat) == T_RAT ) {
        gel(x, 1) = IntToPariGEN(NUM_RAT(rat));
        gel(x, 2) = IntToPariGEN(DEN_RAT(rat));
    }
    else {
        gel(x, 1) = IntToPariGEN(rat);
        gel(x, 2) = stoi(1);
    }

    return x;
}

GEN Perm2ToPariGEN(Obj perm)
{
    UInt deg = DEG_PERM2(perm);
    const UInt2 * pt = CONST_ADDR_PERM2(perm);
    GEN  v = cgetg(deg + 1, t_VECSMALL);

    for (UInt i = 1; i <= deg; i++)
        v[i] = pt[i-1] + 1;
    return v;
}

GEN Perm4ToPariGEN(Obj perm)
{
    UInt deg = DEG_PERM4(perm);
    const UInt4 * pt = CONST_ADDR_PERM4(perm);
    GEN  v = cgetg(deg + 1, t_VECSMALL);

    for (UInt i = 1; i <= deg; i++)
        v[i] = pt[i-1] + 1;
    return v;
}

GEN ListToPariVecsmall(Obj list)
{
    UInt len = LEN_LIST(list);
    GEN v = cgetg(len + 1, t_VECSMALL);
    for(UInt i = 1; i <= len; i++ ) {
        Obj elt = ELM_LIST(list, i);
        v[i] = UInt_ObjInt(elt);
    }
    return v;
}

GEN ListToPariVecVecsmall(Obj list)
{
    UInt len = LEN_LIST(list);
    GEN v = cgetg(len + 1, t_VEC);
    for(UInt i = 1; i <= len; i++ ) {
        Obj elt = ELM_LIST(list, i);
        gel(v, i) = ListToPariVecsmall(elt);
    }
    return v;
}

GEN ListToPariVecVecVecsmall(Obj list)
{
    UInt len = LEN_LIST(list);
    GEN v = cgetg(len + 1, t_VEC);
    for(UInt i = 1; i <= len; i++ ) {
        Obj elt = ELM_LIST(list, i);
        gel(v, i) = ListToPariVecVecsmall(elt);
    }
    return v;
}

GEN ListToPariVec(Obj list)
{
    UInt len = LEN_LIST(list);
    GEN v = cgetg(len + 1, t_VEC);
    for(UInt i = 1; i <= len; i++ ) {
        Obj elt = ELM_LIST(list, i);
        gel(v, i) = ObjToPariGEN(elt);
    }
    return v;
}

GEN IntToPariGEN(Obj o)
{
    Int i, size, sign;
    GEN r;

    if (IS_INTOBJ(o)) {
        // Immediate integers can be converted using
        // stoi
        r = stoi(Int_ObjInt(o));
    } else { // Large integer
        size = SIZE_INT(o);
        sign = IS_POS_INT(o) ? 1 : -1;

        r = cgeti(size + 2);
        r[1] = evalsigne(sign) | evallgefint(size + 2);
        memcpy(r+2, ADDR_INT(o), size * sizeof(mp_limb_t));
        GMP_TO_PARI(r+2, lgefint(r)-2)
    }
    return r;
}

/* TODO: this only converts *integer* lists to polynomials. We should   */
/* instead have a converter between GAP and PARI univariate polynomials */
GEN CoeffListToPariGEN(Obj poly)
{
    UInt len;
    UInt deg;
    GEN v;

    len = LEN_LIST(poly);
    // Zero polynomial
    if (len == 0) {
        v = cgetg(2, t_POL);
    } else {
        deg = len - 1;
        v = cgetg(3 + deg, t_POL);
        for(UInt i = 2; i < 2 + len; i++) {
            Obj elt = ELM_LIST(poly, i - 1);
            gel(v, i) = IntToPariGEN(elt);
        }
    }
    v[1] = evalsigne(0);
    v = normalizepol(v);

    return v;
}

// Main dispatch GAP Obj -> PARI GEN
GEN ObjToPariGEN(Obj obj)
{
    if (IS_INT(obj))
        return IntToPariGEN(obj);
    else if (TNUM_OBJ(obj) == T_RAT)
        return RatToPariGEN(obj);
    else if (IS_PERM2(obj))
        return Perm2ToPariGEN(obj);
    else if (IS_PERM4(obj))
        return Perm4ToPariGEN(obj);
    else if (IS_LIST(obj))
        return ListToPariVec(obj);
    else
        ErrorQuit("ObjToPariGEN: not a supported type: %s", (Int)TNAM_OBJ(obj), 0L);
}

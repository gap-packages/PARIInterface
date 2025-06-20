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

#include <pari/pari.h>
#include "gap_all.h"          /* GAP headers */

#include <dlfcn.h>

#define PARI_T_GEN 0          // Generic PARI object

#define PARI_DAT_WORDS (sizeof(Obj) * 4)
#define PARI_DAT_TYPE(obj)         ((Int)(CONST_ADDR_OBJ(obj)[1]))
#define SET_PARI_DAT_TYPE(obj, t)  (ADDR_OBJ(obj)[1] = (Obj)(t))
#define PARI_DAT_GEN(obj)          ((GEN)(CONST_ADDR_OBJ(obj)[2]))
#define SET_PARI_DAT_GEN(obj, g)   (ADDR_OBJ(obj)[2] = (Obj)(g))

static Obj IsPARIObj;
static Obj PARI_GEN_Type;    // Imported from GAP
static Obj PARI_GEN_REFLIST; // list of references to PARI Objects
                             // that are still used by GAP
// Conversions from PARI GEN to corresponding GAP Obj
static Obj PariGENToObj(GEN v);
static Obj PariVecToList(GEN v);
static Obj PariVecSmallToList(GEN v);
static Obj PariVecToList(GEN v);

// Conversions from GAP Object to PARI GEN
static GEN ObjToPariGEN(Obj obj);
static GEN ListToPariVec(Obj list);
static GEN IntToPariGEN(Obj o);

#define RequirePARIObj(funcname, op)                                             \
    RequireArgumentCondition(funcname, op, IS_PARI_OBJ(op), "must be a PARI object")

int IS_PARI_OBJ(Obj o)
{
    return CALL_1ARGS(IsPARIObj, o) == True;
}

// This is a bag that wraps a PARI value, in case we want to hold on to
// it GAP-side.
// we should create reference counting for pari objects
// because we have full control over garbage collection
// We should never get unreachable loops in the dependency graph because
// never stick GAP objects into PARI objects
Obj NewPARIGEN(GEN data)
{
    Obj o;

    o = NewBag(T_DATOBJ, PARI_DAT_WORDS);
    SetTypeDatObj(o, PARI_GEN_Type);
    SET_PARI_DAT_TYPE(o, PARI_T_GEN);
    SET_PARI_DAT_GEN(o, data);
    return o;
}

static Obj FuncPARI_GEN_GET_TYPE(Obj self, Obj obj)
{
    if((TNUM_OBJ(obj) != T_DATOBJ) || (PARI_DAT_TYPE(obj) != PARI_T_GEN))
        ErrorQuit("obj has to be a DATOBJ of type PARI_T_GEN", 0L, 0L);
    return INTOBJ_INT(typ(PARI_DAT_GEN(obj)));
}

static Obj FuncPARI_GEN_GET_DATA(Obj self, Obj obj)
{
    if((TNUM_OBJ(obj) != T_DATOBJ) || (PARI_DAT_TYPE(obj) != PARI_T_GEN))
        ErrorQuit("obj has to be a DATOBJ of type PARI_T_GEN", 0L, 0L);
    return PariGENToObj(PARI_DAT_GEN(obj));
}

static Obj FuncINT_TO_PARI_GEN(Obj self, Obj obj)
{
    return NewPARIGEN(IntToPariGEN(obj));
}

//
// Conversions from PARI GEN to corresponding GAP Obj
//
static Obj PariVecToList(GEN v)
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

static Obj PariVecSmallToList(GEN v)
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


static Obj PariIntToIntObj(GEN v)
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

static Obj PariFracToRatObj(GEN v)
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

static Obj PariPolToList(GEN v)
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

// Main dispatch
// When there's no immediately obvious object to convert to
// We currently convert to a list of subobjects
// Some of this should probably be done on the GAP Level
static Obj PariGENToObj(GEN v)
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
// Converts a GAP Obj to a PARI GEN (if possible)
//

static GEN Perm2ToPariGEN(Obj perm)
{
    UInt deg = DEG_PERM2(perm);
    const UInt2 * pt = CONST_ADDR_PERM2(perm);
    GEN  v = cgetg(deg + 1, t_VECSMALL);

    for (UInt i = 1; i <= deg; i++)
        v[i] = pt[i-1] + 1;
    return v;
}

static GEN Perm4ToPariGEN(Obj perm)
{
    UInt deg = DEG_PERM4(perm);
    const UInt4 * pt = CONST_ADDR_PERM4(perm);
    GEN  v = cgetg(deg + 1, t_VECSMALL);

    for (UInt i = 1; i <= deg; i++)
        v[i] = pt[i-1] + 1;
    return v;
}

static GEN ListToPariVecsmall(Obj list)
{
    UInt len = LEN_LIST(list);
    GEN v = cgetg(len + 1, t_VECSMALL);
    for(UInt i = 1; i <= len; i++ ) {
        Obj elt = ELM_LIST(list, i);
        v[i] = UInt_ObjInt(elt);
    }
    return v;
}

static GEN ListToPariVecVecsmall(Obj list)
{
    UInt len = LEN_LIST(list);
    GEN v = cgetg(len + 1, t_VEC);
    for(UInt i = 1; i <= len; i++ ) {
        Obj elt = ELM_LIST(list, i);
        gel(v, i) = ListToPariVecsmall(elt);
    }
    return v;
}

static GEN ListToPariVecVecVecsmall(Obj list)
{
    UInt len = LEN_LIST(list);
    GEN v = cgetg(len + 1, t_VEC);
    for(UInt i = 1; i <= len; i++ ) {
        Obj elt = ELM_LIST(list, i);
        gel(v, i) = ListToPariVecVecsmall(elt);
    }
    return v;
}

static GEN ListToPariVec(Obj list)
{
    UInt len = LEN_LIST(list);
    GEN v = cgetg(len + 1, t_VEC);
    for(UInt i = 1; i <= len; i++ ) {
        Obj elt = ELM_LIST(list, i);
        gel(v, i) = ObjToPariGEN(elt);
    }
    return v;
}

static GEN IntToPariGEN(Obj o)
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
        memcpy(r+2, ADDR_INT(o), size * sizeof(GEN));
        GMP_TO_PARI(r+2, lgefint(r)-2)
    }
    return r;
}

static GEN CoeffListToPariGEN(Obj poly)
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

static GEN ObjToPariGEN(Obj obj)
{
    if (IS_INT(obj))
        return IntToPariGEN(obj);
    else if (IS_PERM2(obj))
        return Perm2ToPariGEN(obj);
    else if (IS_PERM4(obj))
        return Perm4ToPariGEN(obj);
    else if (IS_LIST(obj))
        return ListToPariVec(obj);
    else
        ErrorQuit("ObjToPariGEN: not a supported type: %s", (Int)TNAM_OBJ(obj), 0L);
}


//
// GAP Facing functions
//

Obj FuncPARI_GEN_TO_STR(Obj self, Obj o)
{
  char * str = GENtostr(PARI_DAT_GEN(o));
  Obj s = MakeString(str);
  pari_free(str);
  return s;
}

Obj FuncPARI_VECINT(Obj self, Obj list)
{
    GEN v = ListToPariVec(list);
    return NewPARIGEN(v);
}

Obj FuncPARI_VECVECSMALL(Obj self, Obj list)
{
    GEN v = ListToPariVecVecsmall(list);
    return NewPARIGEN(v);
}

Obj FuncPARI_VECVECVECSMALL(Obj self, Obj list)
{
    GEN v = ListToPariVecVecVecsmall(list);
    return NewPARIGEN(v);
}

Obj FuncPARI_UNIPOLY(Obj self, Obj poly)
{
    GEN v = CoeffListToPariGEN(poly);
    return NewPARIGEN(v);
}

Obj FuncPARI_POL_GALOIS_GROUP(Obj self, Obj poly)
{
    pari_sp av = avma;
    GEN v, w;
    Obj r;

    v = CoeffListToPariGEN(poly);
    w = polgalois(v, DEFAULTPREC);

    r = PariGENToObj(w);
    avma = av;

    return r;
}

Obj FuncPARI_POL_FACTOR_MOD_P(Obj self, Obj poly, Obj p)
{
    pari_sp av = avma;
    GEN v, w, x;
    Obj r;

    v = CoeffListToPariGEN(poly);
    x = stoi(Int_ObjInt(p));
    w = FpX_factor(v, x);

    r = PariGENToObj(w);
    avma = av;

    return r;
}

Obj FuncPARI_GEN_ROUNDTRIP(Obj self, Obj x)
{
    pari_sp av = avma;
    Obj r = PariGENToObj(ObjToPariGEN(x));
    avma = av;
    return r;
}

Obj FuncPARI_MULT(Obj self, Obj x, Obj y)
{
    pari_sp av = avma;
    GEN a,b;
    Obj r;

    a = ObjToPariGEN(x);
    b = ObjToPariGEN(y);

    r = PariGENToObj(gmul(a,b));
    avma = av;

    return r;
}

Obj FuncPARI_FACTOR_INT(Obj self, Obj x)
{
    pari_sp av = avma;
    GEN y, f;
    Obj r;

    y = ObjToPariGEN(x);
    f = factorint(y, 0);
    r = PariGENToObj(f);

    avma = av;
    return r;
}

Obj FuncPARI_GET_VERSION(Obj self)
{
    pari_sp av = avma;
    Obj r = PariGENToObj(pari_version());
    avma = av;
    return r;
}

Obj FuncPARI_INIT(Obj self, Obj stack, Obj stackmax)
{
    RequireInt("PARI_INIT", stack);
    RequireInt("PARI_INIT", stackmax);

    size_t stack_size = UInt_ObjInt(stack);
    size_t stack_maxsize = UInt_ObjInt(stackmax);

    pari_init_opts(stack_size, stack_maxsize, INIT_DFTm|INIT_JMPm);
#if PARI_VERSION_CODE >= PARI_VERSION(2,9,0)
    paristack_setsize(stack_size, stack_maxsize);
#endif
    return FuncPARI_GET_VERSION(self);
}

Obj FuncPARI_CLOSE(Obj self)
{
    pari_close();
    return 0;
}

/* These are good examples of the pattern
   of calls into PARI from GAP. These functions
   could be auto-generated */

static Obj FuncPARI_output(Obj self, Obj x)
{
    output(PARI_DAT_GEN(x));
    return 0;
}

static Obj FuncPARI_gcdii(Obj self, Obj x, Obj y)
{
    GEN x_ = PARI_DAT_GEN(x);
    GEN y_ = PARI_DAT_GEN(y);

    return NewPARIGEN(gcdii(x_, y_));
}

typedef GEN (* GENFunc) (/*arguments*/);

static GENFunc GetPARIFunc(Obj name)
{
    GENFunc func = dlsym(RTLD_DEFAULT, CONST_CSTR_STRING(name));
    if (!func)
        ErrorQuit("function not found: %s", (Int)dlerror(), 0);
    return func;
}

static Obj FuncPARI_VEC_TO_LIST(Obj self, Obj x)
{
    GEN v = PARI_DAT_GEN(x);
    Int len = lg(v);
    Obj res = NEW_PLIST(T_PLIST, len - 1);
    SET_LEN_PLIST(res, len - 1);

    for(Int i = 1; i < len; i++) {
        SET_ELM_PLIST(res, i, NewPARIGEN(gel(v, i)));
        CHANGED_BAG(res);
    }
    return res;
}

static Obj FuncPARI_CALL0(Obj self, Obj name)
{
    GENFunc func = GetPARIFunc(name);
    return NewPARIGEN((*func)());
}

static Obj FuncPARI_CALL1(Obj self, Obj name, Obj a1)
{
    GENFunc func = GetPARIFunc(name);
    GEN g1 = PARI_DAT_GEN(a1);
    return NewPARIGEN((*func)(g1));
}

static Obj FuncPARI_CALL2(Obj self, Obj name, Obj a1, Obj a2)
{
    GENFunc func = GetPARIFunc(name);
    GEN g1 = PARI_DAT_GEN(a1);
    GEN g2 = PARI_DAT_GEN(a2);
    return NewPARIGEN((*func)(g1,g2));
}

static Obj FuncPARI_CALL3(Obj self, Obj name, Obj a1, Obj a2, Obj a3)
{
    GENFunc func = GetPARIFunc(name);
    GEN g1 = PARI_DAT_GEN(a1);
    GEN g2 = PARI_DAT_GEN(a2);
    GEN g3 = PARI_DAT_GEN(a3);
    return NewPARIGEN((*func)(g1,g2,g3));
}

static Obj FuncPARI_CALL4(Obj self, Obj name, Obj a1, Obj a2, Obj a3, Obj a4)
{
    GENFunc func = GetPARIFunc(name);
    GEN g1 = PARI_DAT_GEN(a1);
    GEN g2 = PARI_DAT_GEN(a2);
    GEN g3 = PARI_DAT_GEN(a3);
    GEN g4 = PARI_DAT_GEN(a4);
    return NewPARIGEN((*func)(g1,g2,g3,g4));
}

static Obj FuncPARI_CALL5(Obj self, Obj name, Obj a1, Obj a2, Obj a3, Obj a4, Obj a5)
{
    GENFunc func = GetPARIFunc(name);
    GEN g1 = PARI_DAT_GEN(a1);
    GEN g2 = PARI_DAT_GEN(a2);
    GEN g3 = PARI_DAT_GEN(a3);
    GEN g4 = PARI_DAT_GEN(a4);
    GEN g5 = PARI_DAT_GEN(a5);
    return NewPARIGEN((*func)(g1,g2,g3,g4,g5));
}

// To wrap PARI function we use modified T_FUNCTION bags with one extra
// value in them, which points to the PARI function being wrapped.
typedef struct {
    FuncBag f;
    void * pariFunc;
} PARIFuncBag;

static inline void * PARI_FUNC(Obj func)
{
    return ((const PARIFuncBag *)CONST_ADDR_OBJ(func))->pariFunc;
}

static Obj PARI_FUNC_HANDLER0(Obj self)
{
    GEN (*func)() = PARI_FUNC(self);
    return NewPARIGEN((*func)());
}

static Obj PARI_FUNC_HANDLER1(Obj self, Obj a1)
{
    GEN (*func)(GEN) = PARI_FUNC(self);
    GEN g1 = PARI_DAT_GEN(a1);
    return NewPARIGEN((*func)(g1));
}

static Obj PARI_FUNC_HANDLER2(Obj self, Obj a1, Obj a2)
{
    GEN (*func)(GEN,GEN) = PARI_FUNC(self);
    GEN g1 = PARI_DAT_GEN(a1);
    GEN g2 = PARI_DAT_GEN(a2);
    return NewPARIGEN((*func)(g1,g2));
}

static Obj PARI_FUNC_HANDLER3(Obj self, Obj a1, Obj a2, Obj a3)
{
    GEN (*func)(GEN,GEN,GEN) = PARI_FUNC(self);
    GEN g1 = PARI_DAT_GEN(a1);
    GEN g2 = PARI_DAT_GEN(a2);
    GEN g3 = PARI_DAT_GEN(a3);
    return NewPARIGEN((*func)(g1,g2,g3));
}

static Obj PARI_FUNC_HANDLER4(Obj self, Obj a1, Obj a2, Obj a3, Obj a4)
{
    GEN (*func)(GEN,GEN,GEN,GEN) = PARI_FUNC(self);
    GEN g1 = PARI_DAT_GEN(a1);
    GEN g2 = PARI_DAT_GEN(a2);
    GEN g3 = PARI_DAT_GEN(a3);
    GEN g4 = PARI_DAT_GEN(a4);
    return NewPARIGEN((*func)(g1,g2,g3,g4));
}

static Obj PARI_FUNC_HANDLER5(Obj self, Obj a1, Obj a2, Obj a3, Obj a4, Obj a5)
{
    GEN (*func)(GEN,GEN,GEN,GEN,GEN) = PARI_FUNC(self);
    GEN g1 = PARI_DAT_GEN(a1);
    GEN g2 = PARI_DAT_GEN(a2);
    GEN g3 = PARI_DAT_GEN(a3);
    GEN g4 = PARI_DAT_GEN(a4);
    GEN g5 = PARI_DAT_GEN(a5);
    return NewPARIGEN((*func)(g1,g2,g3,g4,g5));
}

static Obj FuncPARI_FUNC_WRAP(Obj self, Obj name, Obj args)
{
    Obj func;
    Int narg;
    ObjFunc handler;
    GENFunc pariFunc = GetPARIFunc(name);

    narg = LEN_LIST(args);

    switch (narg) {
    case 0:
        handler = PARI_FUNC_HANDLER0;
        break;
    case 1:
        handler = PARI_FUNC_HANDLER1;
        break;
    case 2:
        handler = PARI_FUNC_HANDLER2;
        break;
    case 3:
        handler = PARI_FUNC_HANDLER3;
        break;
    case 4:
        handler = PARI_FUNC_HANDLER4;
        break;
    case 5:
        handler = PARI_FUNC_HANDLER5;
        break;
    default:
        ErrorQuit("cannot handle functions with %i arguments", narg, 0L);
        break;
    }
    func = NewFunctionT(T_FUNCTION, sizeof(PARIFuncBag), name, narg,
                        args, handler);

    ((PARIFuncBag *)ADDR_OBJ(func))->pariFunc = pariFunc;
    return func;
}

static Obj FuncPARI_AVMA(Obj self)
{
  return ObjInt_UInt(avma);
}
static void FuncPARI_SET_AVMA(Obj self, Obj av)
{
  RequireNonnegativeSmallInt("PARI_SET_AVMA", av); 
  avma = UInt_ObjInt(av);
}

static Obj FuncPARI_GEN_TO_OBJ(Obj self, Obj x)
{
  RequirePARIObj("GEN_TO_OBJ", x);
  return PariGENToObj(PARI_DAT_GEN(x));
}

// Table of functions to export
static StructGVarFunc GVarFuncs [] = {
    GVAR_FUNC(PARI_INIT, 2, "stack, stackmax"),
    GVAR_FUNC(PARI_CLOSE, 0, ""),
    GVAR_FUNC(PARI_GET_VERSION, 0, ""),
    GVAR_FUNC(PARI_GEN_ROUNDTRIP, 1, "x"),
    GVAR_FUNC(PARI_MULT, 2, "a, b"),
    GVAR_FUNC(PARI_VECINT, 1, "list"),
    GVAR_FUNC(PARI_VECVECSMALL, 1, "list"),
    GVAR_FUNC(PARI_VECVECVECSMALL, 1, "list"),
    GVAR_FUNC(PARI_UNIPOLY, 1, "poly"),
    GVAR_FUNC(PARI_POL_GALOIS_GROUP, 1, "poly"),
    GVAR_FUNC(PARI_POL_FACTOR_MOD_P, 2, "poly, p"),
    GVAR_FUNC(PARI_FACTOR_INT, 1, "x"),
    GVAR_FUNC(PARI_GEN_GET_TYPE, 1, "o"),
    GVAR_FUNC(PARI_GEN_GET_DATA, 1, "o"),
    GVAR_FUNC(PARI_GEN_TO_OBJ, 1, "o"),
    GVAR_FUNC(INT_TO_PARI_GEN, 1, "i"),
    GVAR_FUNC(PARI_GEN_TO_STR, 1, "o"),
    GVAR_FUNC(PARI_VEC_TO_LIST, 1, "o"),
    GVAR_FUNC(PARI_CALL0, 1, "name"),
    GVAR_FUNC(PARI_CALL1, 2, "name, a1"),
    GVAR_FUNC(PARI_CALL2, 3, "name, a1, a2"),
    GVAR_FUNC(PARI_CALL3, 4, "name, a1, a2, a3"),
    GVAR_FUNC(PARI_CALL4, 5, "name, a1, a2, a3, a4"),
    GVAR_FUNC(PARI_CALL5, 6, "name, a1, a2, a3, a4, a5"),
    GVAR_FUNC(PARI_FUNC_WRAP, 2, "name, nargs"),
    GVAR_FUNC(PARI_AVMA, 0, ""),
    GVAR_FUNC(PARI_SET_AVMA, 1, "av"),
    { 0 } /* Finish with an empty entry */
};

/******************************************************************************
*F  InitKernel( <module> )  . . . . . . . . initialise kernel data structures
*/
static Int InitKernel( StructInitInfo *module )
{
    /* init filters and functions                                          */
    InitHdlrFuncsFromTable( GVarFuncs );

    ImportGVarFromLibrary("IsPARIObj", &IsPARIObj);
    ImportGVarFromLibrary("PARI_GEN_Type", &PARI_GEN_Type);
    ImportGVarFromLibrary("PARI_GEN_REFLIST", &PARI_GEN_REFLIST);

    /* return success                                                      */
    return 0;
}

/******************************************************************************
*F  InitLibrary( <module> ) . . . . . . .  initialise library data structures
*/
static Int InitLibrary( StructInitInfo *module )
{
    /* init filters and functions */
    InitGVarFuncsFromTable( GVarFuncs );

    /* return success                                                      */
    return 0;
}

/******************************************************************************
*F  InitInfopl()  . . . . . . . . . . . . . . . . . table of init functions
*/
static StructInitInfo module = {
    .type = MODULE_DYNAMIC,
    .name = "PARIInterface",
    .initKernel = InitKernel,
    .initLibrary = InitLibrary,
};

StructInitInfo *Init__Dynamic( void )
{
    return &module;
}

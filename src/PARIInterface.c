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

#include <dlfcn.h>

static Obj IsPARIObj;        // Imported from GAP
static Obj PARI_GEN_Type;    // Imported from GAP
static Obj PARI_GEN_REFLIST; // list of references to PARI Objects

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

/* copied from rationals.c (not public) */
#define RequireRational(funcname, op)                                        \
    RequireArgumentCondition(funcname, op,                                   \
                             TNUM_OBJ(op) == T_RAT || IS_INT(op),            \
                             "must be a rational")

//
// GAP Facing functions
//

static Obj FuncPARI(Obj self, Obj o)
{
    return NewPARIGEN(ObjToPariGEN(o));
}

static Obj FuncPARI_GEN_TO_STR(Obj self, Obj o)
{
  char * str = GENtostr(PARI_DAT_GEN(o));
  Obj s = MakeString(str);
  pari_free(str);
  return s;
}

static Obj FuncPARI_VECINT(Obj self, Obj list)
{
    GEN v = ListToPariVec(list);
    return NewPARIGEN(v);
}

static Obj FuncPARI_VECVECSMALL(Obj self, Obj list)
{
    GEN v = ListToPariVecVecsmall(list);
    return NewPARIGEN(v);
}

static Obj FuncPARI_VECVECVECSMALL(Obj self, Obj list)
{
    GEN v = ListToPariVecVecVecsmall(list);
    return NewPARIGEN(v);
}

static Obj FuncPARI_UNIPOLY(Obj self, Obj poly)
{
    GEN v = CoeffListToPariGEN(poly);
    return NewPARIGEN(v);
}

static Obj FuncPARI_GET_VERSION(Obj self)
{
    pari_sp av = avma;
    Obj r = PariGENToObj(pari_version());
    avma = av;
    return r;
}

static Obj FuncPARI_INIT(Obj self, Obj stack, Obj stackmax)
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

static Obj FuncPARI_INT(Obj self, Obj obj)
{
    RequireInt("PARI_INT", obj);
    return NewPARIGEN(IntToPariGEN(obj));
}

static Obj FuncPARI_FRAC(Obj self, Obj obj)
{
    RequireRational("PARI_FRAC", obj);
    NewPARIGEN(RatToPariGEN(obj));
}

static Obj FuncPARI_CLOSE(Obj self)
{
    pari_close();
    return 0;
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
    /* conversions GAP Obj -> wrapped PARI GEN */
    GVAR_FUNC(PARI, 1, "o"),
    GVAR_FUNC(PARI_INT, 1, "i"),
    GVAR_FUNC(PARI_FRAC, 1, "rat"),
    GVAR_FUNC(PARI_VECINT, 1, "list"),
    GVAR_FUNC(PARI_VECVECSMALL, 1, "list"),
    GVAR_FUNC(PARI_VECVECVECSMALL, 1, "list"),
    GVAR_FUNC(PARI_UNIPOLY, 1, "poly"),
    /* conversions wrapped PARI GEN -> Gap Obj */
    GVAR_FUNC(PARI_GEN_GET_TYPE, 1, "o"),
    GVAR_FUNC(PARI_GEN_GET_DATA, 1, "o"),
    GVAR_FUNC(PARI_GEN_TO_OBJ, 1, "o"),
    GVAR_FUNC(PARI_GEN_TO_STR, 1, "o"),
    GVAR_FUNC(PARI_VEC_TO_LIST, 1, "o"),
    /* function wrapper */
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

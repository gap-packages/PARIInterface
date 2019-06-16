#include <PARIInterface.h>

/* Two functions returning PARI objects */

Obj FuncPARI_output(Obj self, Obj x)
{
    output(PARI_DAT_GEN(x));
    return 0;
}

Obj FuncPARI_gcdii(Obj self, Obj x, Obj y)
{
    GEN x_ = PARI_DAT_GEN(x);
    GEN y_ = PARI_DAT_GEN(y);

    return NewPARIGEN(gcdii(x_, y_));
}

/* Two functions where output are converted back to GAP objects */

Obj FuncPARI_polgalois(Obj self, Obj poly)
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

Obj FuncPARI_FpX_factor(Obj self, Obj poly, Obj p)
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


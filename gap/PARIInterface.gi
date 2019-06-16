#############################################################################
##
## PARIInterface: Interface to PARI
##
##  Copyright 2018-2019
##    Bill Allombert <bill.allombert@math.u-bordeaux.fr>
##    Vincent Delecroix <vincent.delecroix@math.cnrs.fr>
##    Markus Pfeiffer <markus.pfeiffer@morphism.de>
##
## Licensed under the GPL 2 or later.
##
#############################################################################

# PARIInterface: Interface to PARI library

InstallGlobalFunction( PARIInitialise,
function(args...)
    local stack, stackmax;

    if _PARIINTERFACE_INITIALISED then
        PARI_CLOSE();
    fi;

    stack := _PARIINTERFACE_STACK_DEFAULT;
    stackmax := _PARIINTERFACE_STACKMAX_DEFAULT;

    if IsBound(args[1]) and IsPosInt(args[1]) then
        stack := args[1];
    fi;
    if IsBound(args[2]) and IsPosInt(args[2]) then
        stackmax := args[2];
    fi;
    PARI_INIT(stack, stackmax);
    _PARIINTERFACE_INITIALISED := true;
    return PARI_GET_VERSION();
end);

InstallGlobalFunction( PARIClose,
function()
    if _PARIINTERFACE_INITIALISED then
        PARI_CLOSE();
        _PARIINTERFACE_INITIALISED := false;
    fi;
end);

InstallGlobalFunction( PARIPolynomial,
function(p)
    local coeffs;
    coeffs := CoefficientsOfUnivariatePolynomial(p);
    return PARI_UNIPOLY(coeffs);
end );

InstallGlobalFunction( PARIGaloisGroup,
function(p)
    local coeffs;
    coeffs := CoefficientsOfUnivariatePolynomial(p);
    return PARI_POL_GALOIS_GROUP(coeffs);
end );

InstallMethod( ViewObj, "for a PARI DatObj",
        [ IsPARIObj ],
function(o)
    Print("PARI(",PARI_GEN_TO_STR(o),")");
end);

InstallGlobalFunction( PARIGetAvma,
function()
   return PARI_AVMA();
end );

InstallGlobalFunction( PARISetAvma,
function(av)
   PARI_SET_AVMA(av);
end );

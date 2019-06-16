########################################################################
##
##  PARIInterface package
##
##  Copyright 2018-2019
##    Bill Allombert <bill.allombert@math.u-bordeaux.fr>
##    Vincent Delecroix <vincent.delecroix@math.cnrs.fr>
##    Markus Pfeiffer <markus.pfeiffer@morphism.de>
##
## Licensed under the GPL 2 or later.
##
########################################################################

_PARIINTERFACE_INITIALISED := false;
_PARIINTERFACE_STACK_DEFAULT := 8 * 1024 * 1024;
_PARIINTERFACE_STACKMAX_DEFAULT := 1024 * 1024 *1024;

#! @Chapter PARIInterface
#! @Section Basics
#!
#! A C interface to the PARI number theory library. The PARI documentation is
#! available at http://pari.math.u-bordeaux.fr/

#! @Arguments o
#! @Description
#!   Test whether an object is a PARI object
#! @BeginExampleSession
#! gap> x := Indeterminate(Rationals, "x");;
#! gap> IsPARIObj(x);
#! false
#! gap> IsPARIObj( PARIPolynomial(x) );
#! true
#! @EndExampleSession
DeclareCategory( "IsPARIObj",  IsObject );

BindGlobal( "PARI_GEN_Family", NewFamily("PARIGENFamily"));
BindGlobal( "PARI_GEN_Type", NewType(PARI_GEN_Family, IsPARIObj) );

DeclareGlobalFunction( "PARIInitialise" );
DeclareGlobalFunction( "PARIClose" );
DeclareGlobalFunction( "PARIGetAvma" );
DeclareGlobalFunction( "PARISetAvma" );

#! @Arguments p
#! @Description
#!   Turn a GAP univariate polynomial into a PARI polynomial
#! @BeginExampleSession
#! gap> PARIPolynomial(x^2 - 1);
#! PARI(x^2-1)
#! @EndExampleSession
DeclareGlobalFunction( "PARIPolynomial" );

#! @Arguments p
#! @Description
#!   Return the (abstract) Galois group of a polynomial
#! @BeginExampleSession
#! gap> PARIGaloisGroup(x^3 - x - 1);
#! [ 6, -1, 1, "S3" ]
#! @EndExampleSession
DeclareGlobalFunction( "PARIGaloisGroup" );

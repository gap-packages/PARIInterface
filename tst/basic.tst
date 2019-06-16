gap> PARIInitialise();;
gap> PARI_GEN_TO_OBJ(1);
Error, GEN_TO_OBJ: <x> must be a PARI object (not the integer 1)

gap> PARIClose();;
gap> PARIInitialise(100000,2^24);;

gap> i := PARI_INT(4913);
PARI(4913)
gap> PARI_GEN_TO_OBJ(i);
4913
gap> i := PARI_INT(3^100);
PARI(515377520732011331036461129765621272702107522001)
gap> PARI_GEN_TO_OBJ(i);
515377520732011331036461129765621272702107522001
gap> PARI_INT(3/2);
Error, PARI_INT: <obj> must be an integer (not a rational)

gap> f := PARI_FRAC(2/3);
PARI(2/3)
gap> PARI_GEN_TO_OBJ(f);
2/3
gap> PARI_FRAC(23);
PARI(23/1)
gap> PARI_FRAC(1.23);
Error, PARI_FRAC: <obj> must be a rational (not a macfloat)

gap> v := PARI_VECINT( [1,2,2^100] );
PARI([1, 2, 1267650600228229401496703205376])
gap> PARI_GEN_TO_OBJ(v);
[ 1, 2, 1267650600228229401496703205376 ]
gap> PARI_VECINT([ 1, "a" ]);
Error, ObjToPariGEN: not a supported type: character

gap> v := PARI_VECVECSMALL( [ [1,2], [3,4,5] ]);
PARI([Vecsmall([1, 2]), Vecsmall([3, 4, 5])])
gap> PARI_GEN_TO_OBJ(v);
[ [ 1, 2 ], [ 3, 4, 5 ] ]
gap> PARI_VECVECSMALL( [1,2,3] );
Error, Length: <list> must be a list (not the integer 1)

gap> v := PARI_VECVECVECSMALL( [ [ [1], [2,3] ], [[4,5]] ]);
PARI([[Vecsmall([1]), Vecsmall([2, 3])], [Vecsmall([4, 5])]])
gap> PARI_GEN_TO_OBJ(v);
[ [ [ 1 ], [ 2, 3 ] ], [ [ 4, 5 ] ] ]
gap> PARI_VECVECVECSMALL( [1] );
Error, Length: <list> must be a list (not the integer 1)

gap> p := PARI_UNIPOLY([2, 5, 0, 1, 0, 2^100, -3]);
PARI(-3*x^6 + 1267650600228229401496703205376*x^5 + x^3 + 5*x + 2)


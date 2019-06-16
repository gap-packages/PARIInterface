gap> PARIInitialise();;
gap> PARI_GEN_TO_OBJ(1);
Error, GEN_TO_OBJ: <x> must be a PARI object (not the integer 1)

gap> r := PolynomialRing(Integers, 1);;
gap> p := r.1 ^ 5 + 2;;
gap> PARIGaloisGroup(p);
[ 20, -1, 1, "F(5) = 5:4" ]

gap> PARIClose();;
gap> PARIInitialise(100000,2^24);;

gap> INT_TO_PARI_GEN(4913);
PARI(4913)
gap> INT_TO_PARI_GEN(3^100);
PARI(515377520732011331036461129765621272702107522001)

gap> PARI_GEN_ROUNDTRIP((1,3)(2,4,5));
[ 3, 4, 1, 5, 2 ]
gap> PARI_GEN_ROUNDTRIP([289, (1,3),(2,4,5)]);
[ 289, [ 3, 2, 1 ], [ 1, 4, 3, 5, 2 ] ]

gap> PARI_MULT(0,2);
0
gap> PARI_MULT(2,0);
0
gap> PARI_MULT(2,3);
6
gap> PARI_MULT(2^24,3^12);
8916100448256
gap> PARI_MULT(2^100,3^200);
336705732427516898587460627772004697542605295316077247452351004740460372771448563031143507354757009822674618697405379563749376

gap> PARI_FACTOR_INT(24012425);
[ [ 5, 960497 ], [ 2, 1 ] ]
gap> PARI_FACTOR_INT(100);
[ [ 2, 5 ], [ 2, 2 ] ]
gap> PARI_FACTOR_INT(1204102740127840128401821209348);
[ [ 2, 3, 163, 7823, 12097, 27719453, 234670785731 ], [ 2, 1, 1, 1, 1, 1, 1 ] 
 ]

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

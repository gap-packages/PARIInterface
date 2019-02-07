#
gap> # TODO: test PARI_CALL0

#
gap> PARI_CALL1("negi", INT_TO_PARI_GEN(3));
PARI(-3)
gap> negi := PARI_FUNC_WRAP("negi", ["a"]);
function( a ) ... end
gap> negi(INT_TO_PARI_GEN(3));
PARI(-3)

#
gap> PARI_CALL2("gcdii", INT_TO_PARI_GEN(15), INT_TO_PARI_GEN(12));
PARI(3)
gap> gcdii := PARI_FUNC_WRAP("gcdii", ["x", "y"]);
function( x, y ) ... end
gap> gcdii(INT_TO_PARI_GEN(15), INT_TO_PARI_GEN(12));
PARI(3)

#
gap> PARI_CALL3("addmulii", INT_TO_PARI_GEN(1), INT_TO_PARI_GEN(2), INT_TO_PARI_GEN(3));
PARI(7)
gap> addmulii := PARI_FUNC_WRAP("addmulii", ["x", "y", "z"]);
function( x, y, z ) ... end
gap> addmulii(INT_TO_PARI_GEN(1), INT_TO_PARI_GEN(2), INT_TO_PARI_GEN(3));
PARI(7)

#
gap> PARI_CALL4("lincombii", INT_TO_PARI_GEN(1), INT_TO_PARI_GEN(2), INT_TO_PARI_GEN(3), INT_TO_PARI_GEN(4));
PARI(11)
gap> lincombii := PARI_FUNC_WRAP("lincombii", ["u", "v", "x", "y"]);
function( u, v, x, y ) ... end
gap> lincombii(INT_TO_PARI_GEN(1), INT_TO_PARI_GEN(2), INT_TO_PARI_GEN(3), INT_TO_PARI_GEN(4));
PARI(11)

#
gap> # TODO: test PARI_CALL5

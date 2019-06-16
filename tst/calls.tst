#
gap> # TODO: test PARI_CALL0

# PARI_CALL1
gap> PARI_CALL1("negi", PARI_INT(3));
PARI(-3)
gap> negi := PARI_FUNC_WRAP("negi", ["a"]);
function( a ) ... end
gap> negi(PARI_INT(3));
PARI(-3)

gap> PARI_CALL1("polgalois", PARI_UNIPOLY([1, 0, 0, 0, 0, 2]));
PARI([ 20, -1, 1, "F(5) = 5:4" ])

# PARI_CALL2
gap> PARI_CALL2("gcdii", PARI_INT(15), PARI_INT(12));
PARI(3)
gap> gcdii := PARI_FUNC_WRAP("gcdii", ["x", "y"]);
function( x, y ) ... end
gap> gcdii(PARI_INT(15), PARI_INT(12));
PARI(3)

gap> PARI_CALL2("FpX_factor", PARI_UNIPOLY([1, 0, 0, -3, 1]), PARI_INT(3));
PARI([[x^2 + x + 2, x^2 + 2*x + 2]~, Vecsmall([1, 1])])

gap> PARI_CALL2("gmul", PARI_INT(0), PARI_INT(2));
PARI(0)
gap> PARI_CALL2("gmul", PARI_INT(2), PARI_INT(0));
PARI(0)
gap> PARI_CALL2("gmul", PARI_INT(2), PARI_INT(3));
PARI(6)
gap> PARI_CALL2("gmul", PARI_INT(2^24), PARI_INT(3^12));
PARI(8916100448256)
gap> PARI_CALL2("gmul", PARI_INT(2^100), PARI_INT(3^200));
PARI(336705732427516898587460627772004697542605295316077247452351004740460372771448563031143507354757009822674618697405379563749376)

gap> PARI_CALL2("factorint", PARI_INT(24012425), PARI_INT(0));
PARI(
[     5 2]
[960497 1]
)
gap> PARI_CALL2("factorint", PARI_INT(100), PARI_INT(0));
PARI(
[2 2]
[5 2]
)
gap> PARI_CALL2("factorint", PARI_INT(1204102740127840128401821209348), PARI_INT(0));
PARI(
[           2 2]

[           3 1]

[         163 1]

[        7823 1]

[       12097 1]

[    27719453 1]

[234670785731 1]
)

# PARI_CALL3
gap> PARI_CALL3("addmulii", PARI_INT(1), PARI_INT(2), PARI_INT(3));
PARI(7)
gap> addmulii := PARI_FUNC_WRAP("addmulii", ["x", "y", "z"]);
function( x, y, z ) ... end
gap> addmulii(PARI_INT(1), PARI_INT(2), PARI_INT(3));
PARI(7)

# PARI_CALL4
gap> PARI_CALL4("lincombii", PARI_INT(1), PARI_INT(2), PARI_INT(3), PARI_INT(4));
PARI(11)
gap> lincombii := PARI_FUNC_WRAP("lincombii", ["u", "v", "x", "y"]);
function( u, v, x, y ) ... end
gap> lincombii(PARI_INT(1), PARI_INT(2), PARI_INT(3), PARI_INT(4));
PARI(11)

#
gap> # TODO: test PARI_CALL5

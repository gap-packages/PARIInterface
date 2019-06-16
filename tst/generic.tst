gap> for a in [23, -5/7, [1,2,3]] do
>   b := PARI(a);
>   c := PARI_GEN_TO_OBJ(b);
>   if c <> a then
>       Print("wrong", a, c);
>   fi;
> od;

This file describes changes in the PARIInterface package.

## Unreleased

- Require GAP >= 4.12
- Build against a system PARI, optionally chosen via `PARI_PREFIX`, or build
  PARI as part of the package with `make BUILD_PARI=yes` (now also on macOS);
  support the portable PARI kernel
- Allow closing and reopening the PARI library with `PARIClose`;
  `PARIInitialise` accepts the PARI stack size and maximal stack size; add
  `PARIGetAvma` and `PARISetAvma`
- Extend conversion between GAP and PARI objects to lists, permutations,
  large integers and nested vectors, and add `ViewObj` for PARI objects
- Fix `PARI_POL_GALOIS_GROUP` and `PARI_VECINT` (#16)
- Add more documentation

## 0.1 (2018-02-01)

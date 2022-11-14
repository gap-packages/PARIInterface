[![Build Status](https://github.com/gap-packages/PARIInterface/workflows/CI/badge.svg?branch=master)](https://github.com/gap-packages/PARIInterface/actions?query=workflow%3ACI+branch%3Amaster)
[![Code Coverage](https://codecov.io/github/gap-packages/PARIInterface/coverage.svg?branch=master&token=)](https://codecov.io/gh/gap-packages/PARIInterface)

# The PARIInterface GAP package

The PARIInterface package aims at providing bindings to the PARI number theory system.

## Documentation

Full information and documentation can be found in the manual, available
as PDF `doc/manual.pdf` or as HTML `doc/chap0.html`, or on the package
homepage at

  <https://gap-packages.github.io/PARIInterface/>


## Bug reports and feature requests

Please submit bug reports and feature requests via our GitHub issue tracker:

  <https://github.com/gap-packages/PARIInterface/issues>


## Installing PARIInterface

The PARIInterface package needs a relatively recent version of pari
header files and libaries. To compile the package just run `make` in
the PARIInterface repository.

If you happen to have PARI headers and libraries in a non-standard
repository you can set the variable `PARI_PREFIX` as

    $ make PARI_PREFIX=/path/to/pari/prefix

## License

PARIInterface is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by the
Free Software Foundation; either version 2 of the License, or (at your
option) any later version.

For details see the files COPYRIGHT.md and LICENSE.

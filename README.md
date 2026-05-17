# TERMINATOR12

This ought to be a silly recreation of Indra PEGASUS_21 (Polish Enhanced Generation ATC System for Unified Solutions of 21st Century). The operational software used by Polish Airspace Navigation Services Agency, to display live traffic in controller & uncontrolled airspace, used as a main tool used by Air Traffic Controller. Because unfortunately I don't have any direct connection to PANSA's internal LAN, to get data feed from PSR/SSR network it will display data from OGN/FANET/FLARM receivers and ADS-B/ModeS 1090MHz receivers :(. 

_Because using a mouse as a pointing device is only for kids and sissy boys, ParaATC will use VIM-like keyboard command extensively._

### Dependencies
- SDL2 (+ SDL2_gfx, SDL2_image, SDL2_ttf)
- libxml2
- pkgconf
- protobuf-c (only for the optional OGN/Glidernet live-traffic module)

### Building

The project ships a single top-level `Makefile`. On a fresh Arch Linux box:

```sh
make deps        # one-time: install packages via yay, init submodules,
                 # fetch the libsvg2 sources from upstream
make             # build the default binary (OGN module disabled)
make run         # launch ./TERMINATOR12
```

To build with the OGN/Glidernet live APRS module (issue #1):

```sh
make deps-ogn    # extra one-time install of protobuf-c
make WITH_OGN=1  # build with the OGN reader enabled
```

Other useful targets:

```sh
make clean       # remove build artifacts (build/, binary)
make distclean   # also drop the fetched libsvg2 sources
make help        # print all targets
```

### Controls

VIM-style keyboard navigation, applied once per rendered frame (~60 FPS) so
that holding a key produces a smooth, predictable rate:

| Key | Action          |
|-----|-----------------|
| `H` | pan west        |
| `J` | pan east        |
| `K` | pan north       |
| `L` | pan south       |
| `Z` | zoom out        |
| `X` | zoom in         |

Close the window or send `SIGINT` to quit.
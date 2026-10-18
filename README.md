# BattleCatTimer

A multi-threaded static timing analyzer that reports the worst-slack paths of a gate-level design, with common path pessimism removal (CPPR). Written in C++11 as a submission to the [TAU 2018 Timing Analysis Contest](https://sites.google.com/view/taucontest2018/home/resources).

## Overview

Given a Verilog netlist, early/late Liberty libraries, timing assertions and a list of `report_timing` queries, our timer performs the following:

1. parses the inputs (in parallel) and builds a timing graph;
2. propagates arrival times, slews and required arrival times for both early and late modes;
3. for each query, marks the requested `through` / `disable` pins and enumerates the N worst-slack paths with a CPPR-aware k-shortest-path search, parallelized with OpenMP.


## Contest Resources
Detailed description of the project can be found in the link belows.

- [Contest website](https://sites.google.com/view/taucontest2018/home/resources)
- [Contest overview](https://drive.google.com/file/d/0B7cpBRIFsGtrQkhKSXJqS0ZFdmc/view?resourcekey=0-Fy5Vpf74cZE5u8DEtD8uwA)
- [Contest rules](https://drive.google.com/file/d/0B7cpBRIFsGtrLXQtU2tIZGF2STA/view?resourcekey=0-e2mWdRygTT1wwZ3Q6VUIlw)
- [Contest education (timing analysis and path reporting tutorial)](https://drive.google.com/file/d/0B7cpBRIFsGtrQVhRSWpJQWxXM2c/view?resourcekey=0-LhWrLGzBtj_SQbV404ADUg)

This readme explains how to build and run BattleCatTimer and how to read its input and output files.

## Build

Requirements: GNU Make and `g++` with C++11 and OpenMP support.

```bash
git clone https://github.com/S920105123/BattleCatTimer.git
cd BattleCatTimer
make all            # produces ./BattleCatTimer
```

`make rm_clean` removes object files and logs (`make clean` uses the Windows `del` command).

## Usage

```bash
./BattleCatTimer <design>.tau2018 <design>.timing <design>.ops <output>
```

| Argument | Description |
|---|---|
| `<design>.tau2018` | Manifest naming the libraries, parasitics file and netlist |
| `<design>.timing` | Timing assertions: clock, input arrival/slew, output required time/load |
| `<design>.ops` | Queries (`report_timing ...`) |
| `<output>` | File the path reports are written to |

Example with the bundled `vga_lcd` sample (its manifest uses the older `.tau2015` extension; the format is the same):

```bash
./BattleCatTimer test_cache/vga_lcd/vga_lcd.tau2015 \
                 test_cache/vga_lcd/vga_lcd.timing \
                 test_cache/vga_lcd/similar_5.ops \
                 similar_5.out
```

Progress and a timing breakdown are printed to stderr, and a `message.log` file is created in the working directory.

## Input Files

The exact file formats are specified in *Contest file formats* on the [contest resources page](https://sites.google.com/view/taucontest2018/home/resources). The parts BattleCatTimer reads are summarized below.

### `.tau2018` manifest

One line with four file names, in this order, resolved relative to the manifest's directory:

```
<early>.lib <late>.lib <design>.spef <design>.v
```

### `.timing` assertions

One statement per line. Value order is early-rise, early-fall, late-rise, late-fall. Units follow the Liberty files (ps and fF in the benchmarks).

| Statement | Meaning |
|---|---|
| `clock <pin> <period> <low>` | Clock source pin, period and low-phase width |
| `at <pin> <eR> <eF> <lR> <lF>` | Arrival time at a primary input |
| `slew <pin> <eR> <eF> <lR> <lF>` | Slew at a primary input |
| `rat <pin> <eR> <eF> <lR> <lF>` | Required arrival time at a primary output |
| `load <pin> <cap>` | Load capacitance at a primary output |

### `.ops` queries

Only `report_timing` is supported:

```
report_timing [-from <pin>] [-through <pin>]... [-to <pin>] [-disable <pin>]... [-nworst <N>]
```

| Option | Effect |
|---|---|
| `-from`, `-through`, `-to` | Path must pass through the pin. All three behave identically and are matched by topological level, so their order on the line does not matter. |
| `-rise_*`, `-fall_*` (e.g. `-rise_through`) | Restrict the pin to a rising or falling transition. Unqualified options accept either. |
| `-disable`, `-rise_disable`, `-fall_disable` | Path must not pass through the pin. |
| `-nworst <N>` | Number of paths to report (default 1). |

Pins are written `<instance>:<pin>` or `<instance>/<pin>`; primary ports use their bare name. Example:

```
report_timing -rise_through inst_74410:CK -fall_through inst_72492:D -fall_through inst_21485:ZN
```

## Output

`<output>` holds one report block per path, in query order; within a query, paths are sorted by slack, worst first. A query with no feasible path writes `i= <query index>no path` (index starts at 0). Excerpt of one block:

```
Endpoint:   inst_72492/D
Beginpoint: x10359
= Required Time               70.815
- Arrival Time               140.661
= Slack Time                 -69.846
      ----------------------------------------
      Delay     Arrival    Edge  Pin
                Time
      ----------------------------------------
      -         0.000      v     x10359
      -0.000    0.000      v     inst_50714/A1
      9.775     9.775      ^     inst_50714/ZN
      ...
      -0.000    140.661    ^     inst_72492/D
      ----------------------------------------
```

- `Slack Time` = `Required Time` - `Arrival Time`; negative slack is a timing violation.
- `Delay` is the delay of each step, `Arrival Time` the running total.
- `Edge` is `^` for a rising and `v` for a falling transition.

## Project Layout

```
main.cpp             command-line entry point
src/timer/           input parsing (.tau2018, .timing, .ops) and query driver
src/verilog/         Verilog netlist parser
src/liberty/         Liberty parser and delay/slew/constraint lookup
src/spef/            SPEF parser
src/graph/           timing graph, RC trees, CPPR, k-shortest-path search
src/file_reader/     tokenizer shared by the parsers
src/debug/           logging and assertions
test_cache/vga_lcd/  sample inputs: netlist, libraries, .timing and query sets (no SPEF)
unit_test/           small inputs for module-level tests
compare.cpp          developer tool that diffs two reports (make compare)
```

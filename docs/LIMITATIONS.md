# Things Sealighter doesn't do well or at all:

## Handle Non-Ascii strings
Things may work with non-ascii filtering, config, or events, but they may not.

## Work on 32Bit
Sealighter works only on 64Bit, and no effort will be made to make it work on 32Bit.

## Be production-ready
Sealighter is a research tool first, so I would **not** deploy it to 1000s of computer as an EDR replacement.

## Be consistant between c++ and c code conventions
I mainly work in C, and resort to it more than I probably should, so some of the C++ was thrown together



# TODO before v1.0
 * Add new stuff to doco:
   * trace_name
     * Also now in header
     * Need to also update Event Log Manifest
   * max_before_buffer

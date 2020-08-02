# Things Sealighter doesn't do well or at all:

## Handle Non-Ascii strings
Things may work with non-ascii filtering, config, or events, but they may not.

## Work on 32Bit
Sealighter works only on 64Bit, and no effort will be made to make it work on 32Bit.

## Be production-ready
Sealighter is a research tool first, so I would **not** deploy it to 1000s of computer, or as an EDR replacement.

## Be consistent between c++ and c code conventions
I mainly work in C, and resort to it more than I probably should, so the C++ parts were thrown together a bit hastily.

## Auto-parse WPP traces
The format of WPP events have to be 'manually' defined or extracted from a pdb file, we can't extract them from
the session at runtime.

Instead set `dump_raw_event` to `true` on the provider to  get the hex-encoded bytes, and parse its aftet the fact from there.

I could in the future enable a user to specify the event format in a tmf or json struct, and then auto-parse the events,
but this isn't currently on the cards.

# Potential future work:
* Add [Related Activity IDs](https://github.com/microsoft/krabsetw/issues/64) parsing
* Option to pretty-print to file
* Code cleanup (ha)
* Take in a format specification to parse WPP events

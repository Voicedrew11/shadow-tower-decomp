"""DMPSX 3.0: turn INLINE_C.H's GTE placeholder words into COP2 commands.

The Runtime Library's INLINE_C.H ("Macro definitions of DMPSX version 3")
spells each GTE command as an inline `.word` whose opcode is SPECIAL and whose
function field is the undefined 0x3f; bits 6-15 select the command and, for
`gte_mvmva`, bits 18-26 carry its sf/mx/v/cv/lm fields. Psy-Q users ran Sony's
DMPSX on each object after `ccpsx -c`; it rewrites those words inside `.text`.

The 4.5 kit's DOS `BIN/DOS/DMPSX.EXE` (version 3.0, Borland C++, real mode)
copies an object record by record but reads an XDEF record (tag 12) as ten
bytes plus its name where the LNK v2 record has eight; the kit's Win32 DMPSX
3.05 (1998-09-07) reads eight. On ASPSX 2.79 objects the DOS build therefore
loses its place after the first XDEF and, depending on where it lands,
truncates the object or rewrites placeholder-shaped bytes inside other
records. It is given instead a view holding only what it rewrites: the
`.text` section declarations and code records, in their original order and
sizes, records it reads correctly. Its output words are copied back into the assembled
object, and `verify` then requires that only `.text` placeholders changed,
each into a COP2 command, so an object without placeholders stays
byte-identical. DMPSX reports errors only on the DOS console.
"""

from __future__ import annotations

from dataclasses import dataclass, field
import hashlib
from pathlib import Path

DMPSX_VERSION = '3.0'
DMPSX_SHA256 = 'd2e4febf99665a4f867d0dc44db4e0e37e1dadb6e0a2256f41d4e7f3cae41ee0'


def dmpsx_command(stem: str) -> str:
    """Rewrite STEM.GTE (see `view`) into STEM.GTO."""
    return f'DMPSX.EXE {stem}.GTE -o {stem}.GTO'


def is_placeholder(word: int) -> bool:
    return word & 0xFC00003F == 0x3F


def is_cop2_command(word: int) -> bool:
    return word >> 25 == 0x25


def stage(source: Path, root: Path) -> None:
    data = source.read_bytes()
    if hashlib.sha256(data).hexdigest() != DMPSX_SHA256:
        raise ValueError(f'{source}: expected the pinned DMPSX {DMPSX_VERSION}')
    (root / 'DMPSX.EXE').write_bytes(data)


# Fixed-size LNK v2 records (after the tag) that carry no section bytes.
_FIXED = {46: 1, 50: 2, 52: 3, 54: 4, 56: 6, 58: 8, 60: 2}


@dataclass
class _Text:
    """The `.text` part of an object: raw section declarations and code records."""

    declarations: dict[int, bytes] = field(default_factory=dict)
    # (section, file offset of each byte) per code record, in file order.
    records: list[tuple[int, list[int]]] = field(default_factory=list)


def _text(data: bytes) -> _Text:
    """Walk the record layout of an LNK v2 object; anything unknown fails closed."""
    if data[:4] != b'LNK\x02':
        raise ValueError('expected a native Psy-Q LNK v2 object')
    position, active, text = 4, None, _Text()

    def take(size: int) -> int:
        nonlocal position
        if position + size > len(data):
            raise ValueError(f'truncated LNK record at {position:#x}')
        value = int.from_bytes(data[position:position + size], 'little')
        position += size
        return value

    def skip_name() -> None:
        take(take(1))

    def skip_expression(depth: int = 0) -> None:
        tag = take(1)
        if depth > 32:
            raise ValueError('LNK expression too deep')
        if tag == 0:
            take(4)
        elif tag in (2, 4):
            take(2)
        elif tag in (44, 46):
            skip_expression(depth + 1)
            skip_expression(depth + 1)
        else:
            raise ValueError(f'unsupported LNK expression {tag}')

    while True:
        start = position
        tag = take(1)
        if tag == 0:
            if position != len(data):
                raise ValueError('bytes after the LNK end record')
            return text
        if tag == 16:
            number = take(2)
            take(3)
            name_start = position + 1
            skip_name()
            if data[name_start:position] == b'.text':
                text.declarations[number] = data[start:position]
        elif tag == 6:
            active = take(2)
        elif tag == 2:
            size = take(2)
            if active in text.declarations:
                text.records.append((active, list(range(position, position + size))))
            take(size)
        elif tag == 8:
            take(4)
            if active in text.declarations:
                raise ValueError('uninitialized reservation inside .text')
        elif tag == 10:
            take(3)
            skip_expression()
        elif tag in (12, 48):
            take(8)
            skip_name()
        elif tag in (14, 28):
            take(2)
            skip_name()
        elif tag in (18, 40):
            take(6)
            skip_name()
        elif tag in _FIXED:
            take(_FIXED[tag])
        elif tag == 74:
            take(28)
            skip_name()
        elif tag in (76, 78, 80):
            take(10)
        elif tag in (82, 84):
            take(14)
            if tag == 84:
                take(4 * take(2))
                skip_name()
            skip_name()
        else:
            raise ValueError(f'unsupported LNK record {tag} at {start:#x}')


def text_words(data: bytes) -> list[tuple[tuple[int, ...], int]]:
    """Every aligned `.text` word with the file offsets of its four bytes."""
    sections: dict[int, list[int]] = {}
    for section, offsets in _text(data).records:
        sections.setdefault(section, []).extend(offsets)
    words = []
    for offsets in sections.values():
        for start in range(0, len(offsets) - 3, 4):
            where = tuple(offsets[start:start + 4])
            words.append((where, int.from_bytes(bytes(data[i] for i in where), 'little')))
    return words


def view(data: bytes) -> bytes:
    """The object as DMPSX 3.0 can walk it: `.text` declarations and code only."""
    text = _text(data)
    output = bytearray(b'LNK\x02')
    for declaration in text.declarations.values():
        output += declaration
    active = None
    for section, offsets in text.records:
        if section != active:
            output += bytes([6]) + section.to_bytes(2, 'little')
            active = section
        output += bytes([2]) + len(offsets).to_bytes(2, 'little') + bytes(data[i] for i in offsets)
    return bytes(output + b'\0')


def merge(data: bytes, rewritten: bytes, name: str) -> bytes:
    """Copy DMPSX's code bytes from its rewritten `view` back into the object."""
    original = _text(data).records
    try:
        result = _text(rewritten).records
    except ValueError as error:
        raise RuntimeError(f'{name}: DMPSX wrote an unreadable object ({error}); '
                           'see its console diagnostics') from error
    if [(section, len(offsets)) for section, offsets in result] != \
            [(section, len(offsets)) for section, offsets in original]:
        raise RuntimeError(f'{name}: DMPSX changed the .text code records')
    output = bytearray(data)
    for (_, targets), (_, sources) in zip(original, result, strict=True):
        for target, source in zip(targets, sources, strict=True):
            output[target] = rewritten[source]
    return bytes(output)


def verify(before: bytes, after: bytes, name: str) -> int:
    """Check one DMPSX result; return how many placeholders it rewrote.

    The output must equal the assembler's object except that each `.text`
    placeholder word became a COP2 command. An object without placeholders
    must come back byte-identical.
    """
    if len(after) != len(before):
        raise RuntimeError(f'{name}: DMPSX changed the object size '
                           f'({len(before)} -> {len(after)}); see its console diagnostics')
    expected = bytearray(before)
    rewritten = 0
    for where, word in text_words(before):
        if not is_placeholder(word):
            continue
        result = int.from_bytes(bytes(after[i] for i in where), 'little')
        if not is_cop2_command(result):
            raise RuntimeError(f'{name}: DMPSX left GTE placeholder {word:#010x} as {result:#010x} '
                               '(undefined macro or too few delay slots before it)')
        for i in where:
            expected[i] = after[i]
        rewritten += 1
    if bytes(expected) != after:
        raise RuntimeError(f'{name}: DMPSX changed bytes other than GTE placeholder words')
    return rewritten

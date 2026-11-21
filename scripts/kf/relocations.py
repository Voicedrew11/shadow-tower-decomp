"""Pure validation and encoding rules for inferred PS-X MIPS relocations.

The linked executables do not contain a relocation directory.  Both the
delinker and semantic navigator therefore use this module to distinguish a
byte-validated relocation candidate from weaker investigation evidence.
"""

from __future__ import annotations

import re
import struct
from collections.abc import Sequence
from dataclasses import dataclass

from scripts.kf.retail import IMAGE_GP, parse_int, referent_image


SIGNED_LOW_OPCODES = {
    0x08,  # addi
    0x09,  # addiu
    0x20,  # lb
    0x21,  # lh
    0x23,  # lw
    0x24,  # lbu
    0x25,  # lhu
    0x28,  # sb
    0x29,  # sh
    0x2B,  # sw
}


@dataclass(frozen=True)
class DataReferent:
    """A named allocation base, distinct from a decoded S+A address."""

    name: str
    va: int


def named_data_referent(catalog, image: str, name: str) -> DataReferent | None:
    """Resolve an explicit image-local owner without assuming S+A is inside it.

    Address-derived owner spellings select a datum's start, not whichever
    allocation happens to contain the decoded address. Unknown names remain
    unresolved; ambiguous names must not pick the first inventory row.
    """
    if not name:
        return None
    address_name = re.fullmatch(r"DAT_([0-9a-fA-F]{8})", name)
    address = int(address_name.group(1), 16) if address_name else None
    matches = [item for item in catalog.data.get(image, ())
               if (item.va == address if address is not None else item.symbol == name)]
    if len(matches) > 1:
        raise ValueError(f"ambiguous-data-referent:{image}:{name}")
    return DataReferent(matches[0].symbol, matches[0].va) if matches else None


@dataclass(frozen=True)
class RelocationValidation:
    """Decoded fields shared by relocation consumers after validation."""

    site: int
    target: int
    offset: int
    paired_site: int | None = None
    paired_offset: int | None = None
    word: int = 0
    paired_word: int | None = None


def decode_mips26_target(site_va: int, word: int) -> int:
    """Decode a J/JAL target at its linked virtual address."""
    return ((site_va + 4) & 0xF0000000) | ((word & 0x03FFFFFF) << 2)


def encode_mips26_addend(word: int, addend: int) -> int:
    if addend & 3:
        raise ValueError(f"unaligned MIPS26 addend {addend:#x}")
    return (word & 0xFC000000) | ((addend >> 2) & 0x03FFFFFF)


STORE_OPCODES = frozenset({0x28, 0x29, 0x2B})  # sb, sh, sw: write no register


def lui_copy_register(
    hi_word: int, next_word: int, after_word: int | None = None
) -> int | None:
    """The register a `move` right after a LUI copies its value into.

    GCC can take the high half into a temporary and copy it to the register
    the signed lows use (`lui v0,%hi(x)` then `move s3,v0`); the LUI and each
    low still form one HI16/LO16 family. The scheduler may put one store,
    which writes no register, between the two (`lui v0,%hi(x)`;
    `sw s1,20(sp)`; `move s1,v0`); `after_word` is the instruction after
    `next_word`.
    """
    if hi_word >> 26 != 0x0F:
        return None
    hi_register = (hi_word >> 16) & 0x1F

    def move_destination(word: int) -> int | None:
        # addu rd, rs, $zero (move) whose source is the LUI register
        if word & 0xFC1F07FF != 0x21 or (word >> 21) & 0x1F != hi_register:
            return None
        return ((word >> 11) & 0x1F) or None

    copy = move_destination(next_word)
    if copy is None and after_word is not None and next_word >> 26 in STORE_OPCODES:
        copy = move_destination(after_word)
    return copy


def written_register(word: int) -> int | None:
    """The general register an instruction writes (None for none or $zero)."""
    opcode = word >> 26
    rt = (word >> 16) & 0x1F
    if opcode == 0:
        if word & 0x3F in (0x08, 0x0C, 0x0D, 0x11, 0x13, 0x18, 0x19, 0x1A, 0x1B):
            return None  # jr, syscall, break, mthi, mtlo, mult/div: no GPR
        register = (word >> 11) & 0x1F
    elif opcode == 1:
        register = 31 if rt in (0x10, 0x11) else 0  # bltzal/bgezal link
    elif opcode == 3:
        register = 31
    elif 0x08 <= opcode <= 0x0F or 0x20 <= opcode <= 0x26:
        register = rt
    elif 0x10 <= opcode <= 0x13 and (word >> 21) & 0x1F in (0, 2):
        register = rt  # mfcz, cfcz
    else:
        register = 0
    return register or None


def lui_copied_through(hi_word: int, between: Sequence[int]) -> set[int]:
    """Registers holding the LUI value after `between`, the straight-line
    instructions from the LUI to a low that follows it.

    GCC can hoist a loop's high half and copy it into the register the
    loop's low uses a few instructions later (VS.EXE 0x80013978: `lui a0`,
    the loop's first test, then `move v1,a0` before the loop reading
    `lhu 0x61e2(v1)`). A `move` from a holding register adds its
    destination; any other write to a holding register drops it.
    """
    holding = {(hi_word >> 16) & 0x1F}
    for word in between:
        destination = written_register(word)
        if destination is None:
            continue
        if word & 0xFC1F07FF == 0x21 and (word >> 21) & 0x1F in holding:
            holding.add(destination)
        else:
            holding.discard(destination)
    return holding


def read_registers(word: int) -> set[int]:
    """The general registers an instruction reads ($zero excluded)."""
    opcode = word >> 26
    rs = (word >> 21) & 0x1F
    rt = (word >> 16) & 0x1F
    if opcode == 0:
        function = word & 0x3F
        if function in (0x00, 0x02, 0x03):  # sll, srl, sra
            registers = {rt}
        elif function in (0x08, 0x09, 0x11, 0x13):  # jr, jalr, mthi, mtlo
            registers = {rs}
        elif function in (0x0C, 0x0D, 0x10, 0x12):  # syscall, break, mfhi, mflo
            registers = set()
        else:
            registers = {rs, rt}
    elif opcode in (2, 3, 0x0F):  # j, jal, lui
        registers = set()
    elif opcode in (4, 5) or 0x28 <= opcode <= 0x2E or opcode in (0x22, 0x26):
        registers = {rs, rt}  # beq/bne, stores, lwl/lwr (merge into rt)
    elif 0x10 <= opcode <= 0x13:
        registers = {rt} if rs in (4, 6) else set()  # mtcz, ctcz
    else:
        registers = {rs}  # regimm, blez/bgtz, immediates, loads, lwcz/swcz
    registers.discard(0)
    return registers


ARGUMENT_REGISTERS = frozenset({4, 5, 6, 7})
# at, v0-v1, t0-t7, t8-t9: a callee may clobber them and no caller reads
# them back, so a call ends their life.
CALL_CLOBBERED_TEMPORARIES = frozenset({1, 2, 3, *range(8, 16), 24, 25})


def lui_register_is_dead(words: Sequence[int], index: int) -> bool:
    """Whether the register loaded by the LUI at `words[index]` is overwritten
    on every path before anything reads it.

    `words` are one function's instructions. The walk follows both edges of a
    conditional branch (its delay slot first) and stays conservative: a path
    that leaves the function, reaches a computed jump, returns with the value
    in a result or saved register, or passes it to a call as an argument
    counts as a read.
    """
    if words[index] >> 26 != 0x0F:
        return False
    register = (words[index] >> 16) & 0x1F
    if register == 0:
        return False
    seen: set[int] = set()

    def touches(at: int) -> bool | None:
        """True: written first. False: read first. None: neither."""
        if at >= len(words) or register in read_registers(words[at]):
            return False
        return True if written_register(words[at]) == register else None

    def dead_from(at: int) -> bool:
        while True:
            if at >= len(words):
                return False
            if at in seen:
                return True
            seen.add(at)
            word = words[at]
            opcode = word >> 26
            function = word & 0x3F if opcode == 0 else None
            branch = opcode in (1, 4, 5, 6, 7)
            jump = opcode in (2, 3) or function in (0x08, 0x09)
            if not branch and not jump:
                state = touches(at)
                if state is not None:
                    return state
                at += 1
                continue
            if register in read_registers(word):
                return False
            slot = touches(at + 1)
            if slot is not None:
                return slot
            if branch:
                if opcode == 1 and (word >> 16) & 0x1F in (0x10, 0x11):
                    return False  # bltzal/bgezal link: not followed
                offset = word & 0xFFFF
                target = at + 1 + (offset - 0x10000 if offset & 0x8000 else offset)
                if not 0 <= target < len(words) or not dead_from(target):
                    return False
                if opcode == 4 and (word >> 16) & 0x3FF == 0:  # beq $0,$0
                    return True
                at += 2
                continue
            if opcode == 3 or function == 0x09:  # jal, jalr
                if register in ARGUMENT_REGISTERS:
                    return False
                if register in CALL_CLOBBERED_TEMPORARIES:
                    return True
                at += 2
                continue
            # j and jr leave this walk: only a temporary is known dead at a
            # plain return, and nothing is known past a jump.
            return (function == 0x08 and (word >> 21) & 0x1F == 31
                    and register in CALL_CLOBBERED_TEMPORARIES - {2, 3})

    return dead_from(index + 1)


def decode_orphan_hi_target(hi_word: int, target: int) -> None:
    """Check a LUI that no low instruction completes against `target`.

    Reload can leave the high half of an address behind when the pseudo that
    held it is rematerialized elsewhere (FDAT_262.BIN 0x801ed78c: `lui v1`
    in the prologue, v1 rewritten on both paths). The instruction still
    carries the symbol's HI16; with no low to decode, only the carry-adjusted
    high half of the named target can be checked.
    """
    if hi_word >> 26 != 0x0F:
        raise ValueError("HI site is not LUI")
    if ((target + 0x8000) >> 16) & 0xFFFF != hi_word & 0xFFFF:
        raise ValueError("orphan-high-half-mismatch")


def decode_hi_lo_target(
    hi_word: int,
    lo_word: int,
    copy_word: int | None = None,
    after_word: int | None = None,
    between: Sequence[int] | None = None,
) -> int:
    """Decode a HI/LO address; `copy_word` and `after_word` are the two
    instructions after the LUI, and `between` (when the low follows the LUI)
    every instruction from the LUI to the low."""
    if hi_word >> 26 != 0x0F:
        raise ValueError("HI site is not LUI")
    hi_register = (hi_word >> 16) & 0x1F
    low_opcode = lo_word >> 26
    base = (lo_word >> 21) & 0x1F
    if base != hi_register and (
            copy_word is None
            or lui_copy_register(hi_word, copy_word, after_word) != base) and (
            between is None or base not in lui_copied_through(hi_word, between)):
        raise ValueError("LO instruction does not consume the LUI register")
    low = lo_word & 0xFFFF
    if low_opcode == 0x0D:  # ori
        return ((hi_word & 0xFFFF) << 16) | low
    if low_opcode not in SIGNED_LOW_OPCODES:
        raise ValueError(f"unsupported LO opcode {low_opcode:#x}")
    signed_low = low if low < 0x8000 else low - 0x10000
    return (((hi_word & 0xFFFF) << 16) + signed_low) & 0xFFFFFFFF


def encode_hi_lo_addend(hi_word: int, lo_word: int, addend: int) -> tuple[int, int]:
    """Encode the implicit addend used by an ELF HI16/LO16 REL pair."""
    low_opcode = lo_word >> 26
    if low_opcode not in SIGNED_LOW_OPCODES:
        raise ValueError("ELF HI16/LO16 requires a sign-extending low instruction")
    high = ((addend + 0x8000) >> 16) & 0xFFFF
    low = addend & 0xFFFF
    return (hi_word & 0xFFFF0000) | high, (lo_word & 0xFFFF0000) | low


def encode_orphan_hi_addend(hi_word: int, addend: int) -> int:
    """Encode the implicit addend of an R_MIPS_HI16 that has no LO16 of its own."""
    return (hi_word & 0xFFFF0000) | (((addend + 0x8000) >> 16) & 0xFFFF)


# addiu, lb, lh, lwl, lw, lbu, lhu, lwr, sb, sh, swl, sw, swr, lwc2, swc2
GP_RELATIVE_OPCODES = {0x09, 0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x28, 0x29, 0x2A, 0x2B,
                       0x2E, 0x32, 0x3A}


def decode_gprel16_target(image: str, word: int) -> int:
    """Decode a `$gp`-relative access against the image's startup `$gp`."""
    gp = IMAGE_GP.get(image)
    if gp is None:
        raise ValueError("image-gp-unknown")
    if (word >> 21) & 0x1F != 28 or word >> 26 not in GP_RELATIVE_OPCODES:
        raise ValueError("instruction-opcode-mismatch")
    low = word & 0xFFFF
    return (gp + (low - 0x10000 if low & 0x8000 else low)) & 0xFFFFFFFF


def ambiguous_sites(rows) -> set[int]:
    """Sites claimed by more than one row that do not form one HI/LO family.

    GCC shares address halves: one `lui` can feed several signed-low
    instructions, and two `lui`s on converging paths can feed one. Rows that
    share a site and decode the same target through `mips_hi16_lo16` describe
    one relocation family, as ELF REL represents it (HI16 entries precede the
    LO16 entries they pair with). Any other shared site, including a repeated
    row for the same pair, stays ambiguous.
    """
    claims: dict[int, list[dict[str, str]]] = {}
    for row in rows:
        sites = [parse_int(row["site_va"])]
        if row["paired_site_va"]:
            sites.append(parse_int(row["paired_site_va"]))
        for site in sites:
            claims.setdefault(site, []).append(row)
    ambiguous = set()
    for site, members in claims.items():
        if len(members) < 2:
            continue
        family = {(row["kind"], parse_int(row["target_va"]), row.get("target_name", ""))
                  for row in members}
        pairs = {(row["site_va"], row["paired_site_va"]) for row in members}
        if (len(family) != 1 or next(iter(family))[0] != "mips_hi16_lo16"
                or len(pairs) != len(members)):
            ambiguous.add(site)
    return ambiguous


def policy_reason(row: dict[str, str], policy: str) -> str | None:
    if row["status"] == "rejected":
        return "candidate-rejected"
    if policy == "reviewed" and row["status"] != "reviewed":
        return "not-manually-reviewed"
    if (
        policy in {"safe", "reviewed"}
        and row["channel"] != "reachable-code"
        and row["status"] != "reviewed"
    ):
        return "non-reachable-code-channel"
    return None


def is_orphan_high_row(row: dict[str, str]) -> bool:
    """A `mips_hi16_lo16` row that names a LUI with no low: no paired site,
    opcode `lui`, and individually reviewed."""
    return (row["kind"] == "mips_hi16_lo16" and not row["paired_site_va"]
            and row["opcode"] == "lui" and row["status"] == "reviewed")


def orphan_high_reason(words: Sequence[int], index: int, target: int) -> str | None:
    """Why the LUI at `words[index]` is not an admissible orphan HI16 for
    `target`, or None. Shared by the delinker and the semantic navigator."""
    try:
        decode_orphan_hi_target(words[index], target)
    except ValueError as error:
        return f"instruction-pair-mismatch:{error}"
    if not lui_register_is_dead(words, index):
        return "orphan-high-register-is-live"
    return None


def _word(blob: bytes | bytearray, offset: int) -> int:
    if offset < 0 or offset + 4 > len(blob):
        raise ValueError("site-outside-function-extent")
    return struct.unpack_from("<I", blob, offset)[0]


def validate_relocation(
    blob: bytes | bytearray,
    function,
    row: dict[str, str],
    catalog,
    policy: str,
) -> RelocationValidation:
    """Validate one row against linked bytes without rewriting those bytes.

    ``function`` and ``catalog`` intentionally use structural typing so this
    evidence primitive remains below the delinker's object model.
    """
    reason = policy_reason(row, policy)
    if reason:
        raise ValueError(reason)

    site = parse_int(row["site_va"])
    target = parse_int(row["target_va"])
    offset = site - function.va
    word = _word(blob, offset)

    if row["kind"] == "mips26":
        expected_opcode = {"j": 2, "jal": 3}.get(row["opcode"])
        if expected_opcode is None or word >> 26 != expected_opcode:
            raise ValueError("instruction-opcode-mismatch")
        if decode_mips26_target(site, word) != target:
            raise ValueError("decoded-target-mismatch")
        target_function = catalog.function_starts[
            referent_image(function.image, target)].get(target)
        if not function.contains(target) and target_function is None and policy != "all":
            raise ValueError("target-is-not-a-function-start")
        return RelocationValidation(site, target, offset, word=word)

    if row["kind"] == "mips_hi16_lo16":
        if (
            row["target_region"] != "load"
            and row["status"] != "reviewed"
            and policy != "all"
        ):
            raise ValueError("target-outside-load-image")
        if not row["paired_site_va"]:
            if not is_orphan_high_row(row):
                raise ValueError("missing-paired-site")
            # A reviewed LUI whose register dies unread: HI16 only.
            if offset & 3 or len(blob) & 3:
                raise ValueError("site-outside-function-extent")
            words = struct.unpack_from(f"<{len(blob) // 4}I", blob)
            reason = orphan_high_reason(words, offset // 4, target)
            if reason:
                raise ValueError(reason)
            return RelocationValidation(site, target, offset, word=word)
        paired = parse_int(row["paired_site_va"])
        if not function.contains(paired, 4):
            raise ValueError("pair-crosses-function-extent")
        paired_offset = paired - function.va
        paired_word = _word(blob, paired_offset)
        copy_word = _word(blob, offset + 4) if offset + 8 <= len(blob) else None
        after_word = _word(blob, offset + 8) if offset + 12 <= len(blob) else None
        between = ([_word(blob, at) for at in range(offset + 4, paired_offset, 4)]
                   if paired_offset > offset else None)
        try:
            decoded = decode_hi_lo_target(word, paired_word, copy_word, after_word, between)
        except ValueError as error:
            raise ValueError(f"instruction-pair-mismatch:{error}") from error
        if decoded != target:
            raise ValueError("decoded-target-mismatch")
        try:
            encode_hi_lo_addend(word, paired_word, 0)
        except ValueError as error:
            raise ValueError("unsupported-unsigned-low-pair") from error
        return RelocationValidation(
            site,
            target,
            offset,
            paired,
            paired_offset,
            word,
            paired_word,
        )

    if row["kind"] == "mips_gprel16":
        # $gp-relative access to small data: a load, store or addiu off $gp.
        if decode_gprel16_target(function.image, word) != target:
            raise ValueError("decoded-target-mismatch")
        return RelocationValidation(site, target, offset, word=word)

    if row["kind"] == "mips32_candidate" and (
        policy == "all" or row["status"] == "reviewed"
    ):
        if word != target:
            raise ValueError("decoded-target-mismatch")
        return RelocationValidation(site, target, offset, word=word)

    raise ValueError("unsupported-relocation-kind")

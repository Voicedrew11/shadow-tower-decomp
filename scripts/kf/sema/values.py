"""Reaching definitions and constant value sets inside one function.

The analysis runs over a complete CFG (`scripts.kf.sema.cfg.build_graph`, no
reachable unresolved indirect jump and no CFG issue) and answers two
questions about a register just before an instruction: which instructions
may have written it last (`definitions`), and, when every one of them builds
a link-time constant, which values it can hold (`values`).

Locations are the general registers and, when the frame is simple, the
function's own word stack slots. Model assumptions, each checked or stated:

* The O32 call convention: a call (`jal`, `jalr`, `bltzal`/`bgezal`) may
  change every caller-saved register (`$at`, `$v0`-`$v1`, `$a0`-`$a3`,
  `$t0`-`$t9`, `$ra`) after its delay slot, and preserves `$s0`-`$s7`,
  `$gp`, `$sp` and `$fp`.
* A stack slot is tracked only if every write to `$sp` is an
  `addiu sp,sp,imm` in the entry block or in the delay slot of `jr $ra`, and
  no other instruction reads `$sp` except as a load/store base or as
  `addiu r,sp,k` (an address taken). A slot whose word an address-taking
  `addiu` names is not tracked; another escaped stack object is assumed not
  to reach it (the C object model). A non-word store into a slot makes it
  unknown. A call also makes the outgoing argument area unknown: offsets
  below 16 (the argument home words) and every slot the function stores in
  the same block before a call (a stack-passed argument).
* Incoming registers and slots hold unknown values.

Value evaluation follows `lui`, `addiu`, `ori`, register moves (`addu`/`or`
with `$zero`), `$zero` itself, and word stores/loads through tracked
slots. Anything else, a cycle, or an incoming/clobbered value is unknown.
"""

from __future__ import annotations

from dataclasses import dataclass

from scripts.kf.sema.mips import sign_extend_16

SP = 29
RA = 31
CALLER_SAVED = frozenset({1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 24, 25, 31})
ENTRY = -1
WORD = 0xFFFFFFFF


@dataclass(frozen=True)
class Definition:
    """A write that may reach a use: the instruction at `va`, or ENTRY."""

    va: int
    location: int | tuple[str, int]
    call: bool = False  # the call at `va` clobbered the location

    @property
    def known(self) -> bool:
        return self.va != ENTRY and not self.call


def _gpr_written(word: int) -> int | None:
    # Shared with the switch prover's straight-line scan.
    from scripts.kf.sema.cfg import _gpr_written as written

    return written(word)


def _is_store(opcode: int) -> bool:
    return 0x28 <= opcode <= 0x2E or 0x38 <= opcode <= 0x3B  # sb..swr, swc0..swc3


def _reads_sp_as_value(word: int) -> bool:
    """Whether WORD reads `$sp` other than as a load/store base or an
    `addiu` address/adjust."""
    opcode, rs, rt = word >> 26, (word >> 21) & 0x1F, (word >> 16) & 0x1F
    if opcode >= 0x20:  # loads and stores: rs is the base, rt the data
        return 0x28 <= opcode <= 0x2E and rt == SP
    if opcode == 0x09:  # addiu
        return False
    if opcode == 0:
        return SP in {rs, rt}
    if opcode in {0x01, 0x04, 0x05, 0x06, 0x07}:
        return rs == SP or opcode in {0x04, 0x05} and rt == SP
    if 0x08 <= opcode <= 0x0E:
        return rs == SP
    return False


class Frame:
    """Which `$sp`-relative word slots the analysis may track."""

    def __init__(self, graph) -> None:
        entry = graph.blocks[0] if graph.blocks else None
        entry_vas = {row.va for row in entry.instructions} if entry else set()
        returns = {
            row.va + 4 for block in graph.blocks for row in block.instructions
            if row.control is not None and row.control.kind == "return"
        }
        self.tracked = True
        self.escaped: set[int] = set()
        self.outgoing: set[int] = set()
        for block in graph.blocks:
            stored: set[int] = set()
            for row in block.instructions:
                word = row.word
                opcode, rs = word >> 26, (word >> 21) & 0x1F
                if _gpr_written(word) == SP:
                    if not (opcode == 0x09 and rs == SP
                            and (row.va in entry_vas or row.va in returns)):
                        self.tracked = False
                elif opcode == 0x09 and rs == SP:
                    self.escaped.add(sign_extend_16(word & 0xFFFF))
                if _reads_sp_as_value(word):
                    self.tracked = False
                if opcode == 0x2B and rs == SP:
                    offset = sign_extend_16(word & 0xFFFF)
                    if offset >= 16:
                        stored.add(offset)
                if row.control is not None and row.control.call:
                    self.outgoing.update(stored)
                    stored = set()

    def slot(self, offset: int) -> tuple[str, int] | None:
        if not self.tracked or offset & 3:
            return None
        if any(offset <= taken < offset + 4 or taken == offset for taken in self.escaped):
            return None
        return ("stack", offset)

    def clobbered_by_call(self, location) -> bool:
        if isinstance(location, int):
            return location in CALLER_SAVED
        offset = location[1]
        return offset < 16 or offset in self.outgoing


class ValueFlow:
    """Reaching definitions over one complete function CFG."""

    def __init__(self, graph, *, strict: bool = True) -> None:
        self.graph = graph
        if strict and (graph.issues or not graph.reachability_complete):
            raise ValueError(f"{graph.function.name}: value flow needs a complete CFG")
        self.frame = Frame(graph)
        self.blocks = {block.start: block for block in graph.blocks}
        self.successors: dict[int, list[int]] = {start: [] for start in self.blocks}
        for block in graph.blocks:
            for edge in block.successors:
                if edge.kind == "call" or edge.target_block not in self.blocks:
                    continue
                self.successors[block.start].append(edge.target_block)
        self.block_of = {
            row.va: block.start for block in graph.blocks for row in block.instructions
        }
        self._in = self._solve()
        self._memo: dict[tuple, frozenset[int] | None] = {}

    # -- transfer -----------------------------------------------------------

    def _writes(self, word: int):
        """Locations one instruction writes (calls handled by the caller)."""
        opcode, rs = word >> 26, (word >> 21) & 0x1F
        if _is_store(opcode):
            if rs != SP:
                return []
            offset = sign_extend_16(word & 0xFFFF)
            # Any store but a GPR `sw` (a byte/half/unaligned or coprocessor
            # store) leaves the word slot unknown: a definition with no
            # value (`_value` accepts only `sw`).
            slot = self.frame.slot(offset if opcode == 0x2B else offset & ~3)
            return [slot] if slot else []
        register = _gpr_written(word)
        return [register] if register else []

    def _step(self, state: dict, rows, upto: int | None = None) -> dict:
        state = dict(state)
        pending_call: int | None = None
        for row in rows:
            if upto is not None and row.va == upto:
                break
            for location in self._writes(row.word):
                state[location] = frozenset({Definition(row.va, location)})
            if pending_call is not None:
                self._clobber(state, pending_call)
                pending_call = None
            if row.control is not None and row.control.call:
                pending_call = row.va
        if pending_call is not None and upto is None:
            self._clobber(state, pending_call)
        return state

    def _clobber(self, state: dict, site: int) -> None:
        for location in list(state):
            if self.frame.clobbered_by_call(location):
                state[location] = frozenset({Definition(site, location, call=True)})
        for register in CALLER_SAVED:
            state[register] = frozenset({Definition(site, register, call=True)})

    def _solve(self) -> dict[int, dict]:
        if not self.graph.blocks:
            return {}
        locations: set = set(range(1, 32))
        for block in self.graph.blocks:
            for row in block.instructions:
                locations.update(self._writes(row.word))
                word = row.word
                if word >> 26 == 0x23 and (word >> 21) & 0x1F == SP:
                    slot = self.frame.slot(sign_extend_16(word & 0xFFFF))
                    if slot:
                        locations.add(slot)
        entry = {loc: frozenset({Definition(ENTRY, loc)}) for loc in locations}
        start = self.graph.blocks[0].start
        ins: dict[int, dict] = {start: entry}
        outs: dict[int, dict] = {}
        pending = [start]
        while pending:
            current = pending.pop()
            out = self._step(ins[current], self.blocks[current].instructions)
            if outs.get(current) == out:
                continue
            outs[current] = out
            for successor in self.successors[current]:
                merged = dict(ins.get(successor, {}))
                changed = successor not in ins
                for loc, defs in out.items():
                    joined = merged.get(loc, frozenset()) | defs
                    if joined != merged.get(loc):
                        merged[loc] = joined
                        changed = True
                if changed:
                    ins[successor] = merged
                    pending.append(successor)
        return ins

    # -- queries ------------------------------------------------------------

    def reached(self, va: int) -> bool:
        return self.block_of.get(va) in self._in

    def definitions(self, va: int, location) -> frozenset[Definition]:
        """Definitions of LOCATION that may reach the instruction at VA."""
        start = self.block_of.get(va)
        if start is None or start not in self._in:
            return frozenset()
        state = self._step(self._in[start], self.blocks[start].instructions, upto=va)
        return state.get(location, frozenset({Definition(ENTRY, location)}))

    def word_at(self, va: int) -> int:
        start = self.block_of[va]
        return next(row.word for row in self.blocks[start].instructions if row.va == va)

    def values(self, va: int, register: int) -> frozenset[int] | None:
        """Every value REGISTER may hold just before VA, or None if unknown."""
        if register == 0:
            return frozenset({0})
        return self._location_values(va, register, frozenset())

    def _location_values(self, va: int, location, active: frozenset) -> frozenset[int] | None:
        key = (va, location)
        if key in self._memo:
            return self._memo[key]
        if key in active:
            return None
        defs = self.definitions(va, location)
        result: set[int] = set()
        for definition in defs:
            found = self._value(definition, active | {key})
            if found is None:
                result = None
                break
            result |= found
        value = None if result is None or not defs else frozenset(result)
        if not active:
            self._memo[key] = value
        return value

    def _value(self, definition: Definition, active: frozenset) -> frozenset[int] | None:
        if not definition.known:
            return None
        word = self.word_at(definition.va)
        opcode, rs, rt = word >> 26, (word >> 21) & 0x1F, (word >> 16) & 0x1F
        funct, imm = word & 0x3F, word & 0xFFFF

        def operand(register: int) -> frozenset[int] | None:
            if register == 0:
                return frozenset({0})
            return self._location_values(definition.va, register, active)

        if isinstance(definition.location, tuple):  # a word store to the slot
            if opcode != 0x2B:
                return None
            return operand(rt)
        if opcode == 0x0F:  # lui
            return frozenset({(imm << 16) & WORD})
        if opcode == 0x09:  # addiu
            base = operand(rs)
            return None if base is None else frozenset(
                (value + sign_extend_16(imm)) & WORD for value in base)
        if opcode == 0x0D:  # ori
            base = operand(rs)
            return None if base is None else frozenset(value | imm for value in base)
        if opcode == 0 and funct in {0x21, 0x25} and (word >> 6) & 0x1F == 0 and 0 in {rs, rt}:
            return operand(rt if rs == 0 else rs)  # move (addu/or with $zero)
        if opcode == 0x23 and rs == SP:  # reload from a tracked slot
            slot = self.frame.slot(sign_extend_16(imm))
            if slot is None:
                return None
            return self._location_values(definition.va, slot, active)
        return None


@dataclass(frozen=True)
class ConstantTargets:
    """A proved value set for an indirect transfer register."""

    register: int
    values: tuple[int, ...]  # sorted; 0 is a null pointer and adds no edge

    @property
    def targets(self) -> tuple[int, ...]:
        return tuple(value for value in self.values if value)


def constant_targets(flow: ValueFlow, site: int) -> ConstantTargets | None:
    """The values a `jr`/`jalr` register can hold at SITE, if all are constants."""
    if not flow.reached(site):
        return None
    word = flow.word_at(site)
    if word >> 26 != 0 or word & 0x3F not in {0x08, 0x09}:
        return None
    register = (word >> 21) & 0x1F
    values = flow.values(site, register)
    if not values:
        return None
    return ConstantTargets(register, tuple(sorted(values)))


@dataclass(frozen=True)
class HoistedSwitch:
    table: int
    count: int


def hoisted_switch(flow: ValueFlow, site: int) -> HoistedSwitch | None:
    """A GCC tablejump whose table base or entry address was hoisted.

    GCC's loop optimizer moves the switch table address (and, when the
    index is loop-invariant, the scaled entry address) out of the loop, so
    the straight-line rule in `cfg.switch_table` misses the dispatch. The
    accepted chain, each link the only definition reaching its use:

        D:  (any) ri           ; the index
        S:  sll rx, ri, 2
        A:  addu ra, rx, rb    ; rb holds one constant, the table base
        C:  sltiu rc, ri, N    ; reached by D alone
        B:  beqz rc, default   ; reached from C by fall-through only
        L:  lw rt, off(ra)
        J:  jr rt              ; on B's fall-through run, no other entry

    and D is neither between C and B (`_chain`) nor on the dispatch run. Then either
    D, S, A lie in that order on one straight-line run (the whole entry
    address is hoisted: every D is followed by its S and A before control
    can leave, so the A that L reads scaled the D that C bounds), or D lies
    before C on B's own run, S follows D on it (or on the dispatch run) and
    A is on the dispatch run (only the base is hoisted). The table is N
    words at base + off.
    """
    word = flow.word_at(site)
    if word >> 26 != 0 or word & 0x1FFFFF != 0x08:
        return None
    bound = _bound(flow, site)
    if bound is None:
        return None
    branch, dispatch = bound

    def only(va: int, location) -> int | None:
        defs = flow.definitions(va, location)
        if len(defs) != 1:
            return None
        definition = next(iter(defs))
        return definition.va if definition.known else None

    compare = only(branch, (flow.word_at(branch) >> 21) & 0x1F)
    between = None if compare is None else _chain(flow, compare, branch)
    if between is None:
        return None
    sltiu = flow.word_at(compare)
    count = sltiu & 0xFFFF
    if sltiu >> 26 != 0x0B or not 0 < count <= 0x400:
        return None
    index = (sltiu >> 21) & 0x1F
    defining = only(compare, index)
    if (defining is None or flow.block_of[defining] in dispatch
            or flow.block_of[defining] in between
            and not (flow.block_of[defining] == flow.block_of[compare] and defining < compare)):
        return None
    load = only(site, (word >> 21) & 0x1F)
    if load is None or flow.word_at(load) >> 26 != 0x23:
        return None
    lw = flow.word_at(load)
    add = only(load, (lw >> 21) & 0x1F)
    if add is None:
        return None
    summed = flow.word_at(add)
    if summed >> 26 != 0 or summed & 0x7FF != 0x21:
        return None
    for left, right in (((summed >> 21) & 0x1F, (summed >> 16) & 0x1F),
                        ((summed >> 16) & 0x1F, (summed >> 21) & 0x1F)):
        shift = only(add, left)
        if shift is None:
            continue
        sll = flow.word_at(shift)
        if sll & 0xFFE0003F != 0 or (sll >> 6) & 0x1F != 2 or (sll >> 16) & 0x1F != index:
            continue
        if only(shift, index) != defining:
            continue
        base = flow.values(add, right)
        if base is None or len(base) != 1:
            continue
        hoisted = _one_run(flow, (defining, shift, add))
        base_only = (
            _one_run(flow, (defining, compare))
            and flow.block_of[add] in dispatch
            and (flow.block_of[shift] in dispatch or _one_run(flow, (defining, shift)))
        )
        if not (hoisted or base_only):
            continue
        table = (next(iter(base)) + sign_extend_16(lw & 0xFFFF)) & WORD
        return HoistedSwitch(table, count)
    return None


def _predecessors(flow: ValueFlow, start: int) -> list:
    return [(block, edge) for block in flow.graph.blocks for edge in block.successors
            if edge.target_block == start and edge.kind != "call"]


def _bound(flow: ValueFlow, site: int) -> tuple[int, set[int]] | None:
    """The `beq rc,$zero` whose fall-through run reaches SITE, and that run.

    Walks back from SITE's block through blocks with exactly one
    predecessor, each entered by fall-through, to the first block that ends
    in a branch.
    """
    start = flow.block_of[site]
    dispatch = {start}
    while True:
        incoming = _predecessors(flow, start)
        if len(incoming) != 1 or incoming[0][1].kind != "fallthrough":
            return None
        block = incoming[0][0]
        terminal = next((row for row in reversed(block.instructions)
                         if row.control is not None and row.control.terminates_block), None)
        if terminal is None:
            start = block.start
            dispatch.add(start)
            continue
        branch = terminal.word
        if branch >> 26 != 0x04 or (branch >> 16) & 0x1F != 0:
            return None
        return terminal.va, dispatch


def _chain(flow: ValueFlow, first: int, last: int) -> set[int] | None:
    """The blocks from FIRST's to LAST's if LAST's is entered only that way.

    Walks back from LAST's block through blocks with exactly one
    predecessor, each entered by fall-through, until FIRST's block (FIRST
    before LAST when they share a block). Every execution of LAST is then
    preceded by FIRST with only the walked blocks in between.
    """
    target, start = flow.block_of[first], flow.block_of[last]
    walked = {start}
    if start == target:
        return walked if first < last else None
    for _ in range(64):
        incoming = _predecessors(flow, start)
        if len(incoming) != 1 or incoming[0][1].kind != "fallthrough":
            return None
        start = incoming[0][0].start
        walked.add(start)
        if start == target:
            return walked
    return None


def _one_run(flow: ValueFlow, vas: tuple[int, ...]) -> bool:
    """Whether VAS lie in this order on one straight-line run of blocks.

    A run is a chain of blocks, each entered only by fall-through from the
    previous one, none of which (but the last) ends in a branch.
    """
    if list(vas) != sorted(vas):
        return False
    first, last = flow.block_of[vas[0]], flow.block_of[vas[-1]]
    current = first
    while current != last:
        block = flow.blocks[current]
        nexts = [edge for edge in block.successors
                 if edge.kind != "call" and edge.target_block in flow.blocks]
        if len(nexts) != 1 or nexts[0].kind != "fallthrough" or nexts[0].target_block != block.end:
            return False
        current = block.end
        if len(_predecessors(flow, current)) != 1:
            return False
    return True

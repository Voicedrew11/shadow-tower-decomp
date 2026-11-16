"""Delay-slot-aware basic-block and control-flow graph construction."""

from __future__ import annotations

from collections.abc import Callable
from dataclasses import dataclass, field

from scripts.kf.sema.index import Binding, Index
from scripts.kf.sema.mips import (
    Instruction, constant_branch_outcome, sign_extend_16, with_controls, decode_words,
)


@dataclass
class Edge:
    source: int
    site: int
    kind: str
    target: int | None = None
    target_block: int | None = None
    confidence: str = "proven"

    def as_dict(self) -> dict[str, object]:
        return {
            "source": self.source,
            "site": self.site,
            "kind": self.kind,
            "target": self.target,
            "target_block": self.target_block,
            "confidence": self.confidence,
        }


@dataclass
class Block:
    start: int
    end: int
    instructions: tuple[Instruction, ...]
    predecessors: list[int] = field(default_factory=list)
    successors: list[Edge] = field(default_factory=list)
    flags: set[str] = field(default_factory=set)

    def contains(self, va: int) -> bool:
        return self.start <= va < self.end

    def as_dict(self) -> dict[str, object]:
        return {
            "start": self.start,
            "end": self.end,
            "instructions": [
                {"va": row.va, "word": row.word} for row in self.instructions
            ],
            "predecessors": sorted(self.predecessors),
            "successors": [edge.as_dict() for edge in self.successors],
            "flags": sorted(self.flags),
        }


@dataclass
class Graph:
    function: Binding
    blocks: tuple[Block, ...]
    edges: tuple[Edge, ...]
    delay_slots: dict[int, int]
    issues: tuple[str, ...]
    reachability_complete: bool = True

    def as_dict(self) -> dict[str, object]:
        return {
            "function": self.function.as_dict(),
            "blocks": [block.as_dict() for block in self.blocks],
            "edges": [edge.as_dict() for edge in self.edges],
            "delay_slots": [
                {"slot": slot, "owner": owner}
                for slot, owner in sorted(self.delay_slots.items())
            ],
            "issues": list(self.issues),
            "reachability_complete": self.reachability_complete,
        }


def _block_for(blocks: list[Block], va: int | None) -> Block | None:
    if va is None:
        return None
    return next((block for block in blocks if block.contains(va)), None)


def _gpr_written(word: int) -> int | None:
    """The GPR an ordinary MIPS-I instruction writes, for a backward def scan.

    Stores, branches, plain jumps, `jr`, HI/LO writes and traps write none;
    `jal` writes $ra. The switch proof scans only straight-line integer code
    between a bound check and its `jr`.
    """
    opcode, rt, rd, funct = word >> 26, (word >> 16) & 0x1F, (word >> 11) & 0x1F, word & 0x3F
    if opcode == 0:
        if funct in {0x08, 0x0C, 0x0D, 0x11, 0x13, 0x18, 0x19, 0x1A, 0x1B}:
            return None  # jr, syscall, break, mthi, mtlo, mult(u), div(u)
        return rd or None
    if opcode == 0x03:
        return 31
    if opcode in {0x01, 0x02, 0x04, 0x05, 0x06, 0x07} or opcode >= 0x28:
        return None
    if opcode in {0x10, 0x12}:  # mfc/cfc write rt; other coprocessor forms write no GPR
        return rt if (word >> 21) & 0x1F in {0x00, 0x02} else None
    return rt or None


def switch_table_targets(
    instructions: list[Instruction],
    terminal: Instruction,
    function: Binding,
    read_word: Callable[[int], int | None],
) -> tuple[int, ...] | None:
    """Prove a GCC MIPS tablejump and return its case targets, else None.

    Accepted value chain, straight-line from the bound check to the jump:

        sltiu rc, ri, N ; beqz rc, default ; [delay slot]
        lui rb, hi ; addiu rb, rb, lo ; sll rs, ri, 2 ; addu ra, rs, rb
        lw rt, off(ra) ; jr rt

    (the `lui` may sit in the bound branch's delay slot; operand order of the
    `addu` is free). The table is the N words at hi:lo + off. No branch may
    enter the scanned code and the index may not change between the bound and
    the shift, so both see the same value. Every table word must be a decoded
    instruction start inside this function. Anything else stays an
    unresolved indirect jump.
    """
    word = terminal.word
    if word >> 26 != 0 or word & 0x1FFFFF != 0x08:  # jr rs
        return None
    position = next(i for i, row in enumerate(instructions) if row.va == terminal.va)
    entered = {
        row.control.target for row in instructions
        if row.control is not None and row.control.target is not None
    }

    def last_def(register: int, before: int, floor: int) -> int | None:
        for i in range(before - 1, floor - 1, -1):
            if _gpr_written(instructions[i].word) == register:
                return i
        return None

    def opcode(i: int) -> int:
        return instructions[i].word >> 26

    # The bound branch is the last control transfer before the jump; its
    # delay slot and the dispatch block form one straight-line run.
    bound = next((i for i in range(position - 1, -1, -1) if instructions[i].control), None)
    if bound is None or bound + 1 >= position:
        return None
    branch = instructions[bound].word
    if branch >> 26 != 0x04 or (branch >> 16) & 0x1F != 0:  # beq rc, zero
        return None
    if any(row.va in entered for row in instructions[bound + 1:position + 1]):
        return None
    floor = bound
    while floor > 0 and instructions[floor - 1].control is None \
            and instructions[floor].va not in entered:
        floor -= 1
    load = last_def((word >> 21) & 0x1F, position, bound + 1)
    if load is None or opcode(load) != 0x23:  # lw
        return None
    lw = instructions[load].word
    add = last_def((lw >> 21) & 0x1F, load, bound + 1)
    if add is None or opcode(add) != 0 or instructions[add].word & 0x7FF != 0x21:  # addu
        return None
    summed = instructions[add].word
    found = None
    for left, right in (((summed >> 21) & 0x1F, (summed >> 16) & 0x1F),
                        ((summed >> 16) & 0x1F, (summed >> 21) & 0x1F)):
        shift, low = last_def(left, add, bound + 1), last_def(right, add, bound + 1)
        if shift is None or low is None:
            continue
        sll, addiu = instructions[shift].word, instructions[low].word
        if sll & 0xFFE0003F == 0 and (sll >> 6) & 0x1F == 2 and addiu >> 26 == 0x09:
            found = shift, (sll >> 16) & 0x1F, low
            break
    if found is None:
        return None
    shift, index, low = found
    addiu = instructions[low].word
    high = last_def((addiu >> 21) & 0x1F, low, bound + 1)
    if high is None or opcode(high) != 0x0F:  # lui
        return None
    table = (((instructions[high].word & 0xFFFF) << 16) + sign_extend_16(addiu & 0xFFFF)
             + sign_extend_16(lw & 0xFFFF)) & 0xFFFFFFFF
    compare = last_def((branch >> 21) & 0x1F, bound, floor)
    if compare is None or opcode(compare) != 0x0B:  # sltiu
        return None
    sltiu = instructions[compare].word
    count = sltiu & 0xFFFF
    if (sltiu >> 21) & 0x1F != index or not 0 < count <= 0x400:
        return None
    if last_def(index, shift, compare + 1) is not None:
        return None
    addresses = {row.va for row in instructions}
    targets = []
    for entry in range(count):
        target = read_word(table + 4 * entry)
        if target is None or target not in addresses or not function.contains_body(target):
            return None
        targets.append(target)
    return tuple(targets)


def build_graph(
    function: Binding,
    payload: bytes,
    idx: Index,
    read_word: Callable[[int], int | None] | None = None,
) -> Graph:
    if function.space != "text":
        raise ValueError(f"{function.name}: CFG requires a function")
    if function.fragments != 1:
        raise ValueError(
            f"{function.name}: CFG unavailable for fragmented function; "
            "curate fragment ranges first"
        )
    instructions = with_controls(decode_words(function.va, payload[:function.body_size]))
    if not instructions:
        return Graph(function, (), (), {}, ("no complete instruction decoded",), False)

    addresses = {row.va for row in instructions}
    instruction_by_va = {row.va: row for row in instructions}
    leaders = {instructions[0].va}
    delay_slots: dict[int, int] = {}
    issues: list[str] = []
    if len(payload) < function.body_size or function.body_size % 4:
        issues.append("function extent is not completely decoded; reachability is incomplete")

    for row in instructions:
        control = row.control
        if control is None:
            continue
        after = row.va + (8 if control.delay_slot else 4)
        if control.delay_slot:
            if row.va + 4 in addresses:
                delay_slots[row.va + 4] = row.va
            else:
                issues.append(f"0x{row.va:08x} has no decoded delay slot")
        if control.annulled:
            issues.append(f"0x{row.va:08x} has an unmodeled annulled delay slot")
        if control.target in addresses:
            leaders.add(control.target)
        if control.terminates_block and after in addresses:
            leaders.add(after)

    # A proven switch table makes each case target a block leader.
    switch_cases: dict[int, tuple[int, ...]] = {}
    if read_word is not None:
        for row in instructions:
            if row.control is not None and row.control.kind == "indirect-jump":
                cases = switch_table_targets(instructions, row, function, read_word)
                if cases is not None:
                    switch_cases[row.va] = cases
                    leaders.update(cases)

    for slot in delay_slots:
        if instruction_by_va[slot].control is not None:
            issues.append(f"0x{slot:08x} contains a control transfer in a delay slot")

    targeted_delay_slots = sorted(leaders & set(delay_slots))
    for va in targeted_delay_slots:
        leaders.remove(va)
        issues.append(
            f"branch target 0x{va:08x} enters the delay slot owned by "
            f"0x{delay_slots[va]:08x}; represented as an interior target"
        )

    ordered_leaders = sorted(leaders)
    blocks: list[Block] = []
    for position, start in enumerate(ordered_leaders):
        next_leader = (
            ordered_leaders[position + 1]
            if position + 1 < len(ordered_leaders)
            else function.body_end
        )
        rows: list[Instruction] = []
        va = start
        while va < next_leader and va in instruction_by_va:
            row = instruction_by_va[va]
            rows.append(row)
            control = row.control
            if control is not None and control.terminates_block:
                if control.delay_slot and va + 4 in instruction_by_va:
                    rows.append(instruction_by_va[va + 4])
                break
            va += 4
        if rows:
            blocks.append(Block(start, rows[-1].va + 4, tuple(rows)))

    edges: list[Edge] = []
    for number, block in enumerate(blocks):
        terminal = next(
            (
                row for row in reversed(block.instructions)
                if row.control is not None and row.control.terminates_block
            ),
            None,
        )
        for row in block.instructions:
            control = row.control
            if control is None or not control.call:
                continue
            if control.kind == "indirect-call":
                edges.append(Edge(block.start, row.va, "indirect-call"))
            else:
                target_block = _block_for(blocks, control.target)
                edges.append(Edge(
                    block.start,
                    row.va,
                    "call",
                    control.target,
                    target_block.start if target_block else None,
                ))

        if terminal is None:
            if number + 1 < len(blocks) and block.end == blocks[number + 1].start:
                edges.append(Edge(
                    block.start,
                    block.instructions[-1].va,
                    "fallthrough",
                    blocks[number + 1].start,
                    blocks[number + 1].start,
                ))
            continue

        control = terminal.control
        assert control is not None
        after = terminal.va + (8 if control.delay_slot else 4)
        target_block = _block_for(blocks, control.target)
        if control.kind == "branch":
            outcome = constant_branch_outcome(terminal.word)
            if outcome is not False:
                edges.append(Edge(
                    block.start,
                    terminal.va,
                    "taken" if target_block else "taken-external",
                    control.target,
                    target_block.start if target_block else None,
                ))
            fallthrough = _block_for(blocks, after)
            if outcome is not True and fallthrough is not None:
                edges.append(Edge(
                    block.start,
                    terminal.va,
                    "fallthrough",
                    after,
                    fallthrough.start,
                ))
        elif control.kind == "call" and control.conditional:
            fallthrough = _block_for(blocks, after)
            if fallthrough is not None:
                edges.append(Edge(
                    block.start,
                    terminal.va,
                    "fallthrough",
                    after,
                    fallthrough.start,
                ))
        elif control.kind == "jump":
            external = idx.function(control.target or 0)
            kind = "jump" if target_block else "tail" if external else "jump-external"
            edges.append(Edge(
                block.start,
                terminal.va,
                kind,
                control.target,
                target_block.start if target_block else None,
            ))
        elif control.kind == "return":
            edges.append(Edge(block.start, terminal.va, "return"))
        elif control.kind == "indirect-jump":
            cases = switch_cases.get(terminal.va)
            if cases is None:
                edges.append(Edge(block.start, terminal.va, "indirect-jump"))
            for case in dict.fromkeys(cases or ()):
                case_block = _block_for(blocks, case)
                edges.append(Edge(
                    block.start,
                    terminal.va,
                    "switch",
                    case,
                    case_block.start if case_block else None,
                ))
        elif control.kind == "trap":
            edges.append(Edge(block.start, terminal.va, "trap"))

    for edge in edges:
        source = _block_for(blocks, edge.source)
        if source is not None:
            source.successors.append(edge)
        target = _block_for(blocks, edge.target_block)
        if target is not None and edge.kind not in {"call"}:
            target.predecessors.append(edge.source)

    reachable: set[int] = set()
    pending = [blocks[0].start] if blocks else []
    while pending:
        start = pending.pop()
        if start in reachable:
            continue
        reachable.add(start)
        block = _block_for(blocks, start)
        if block is None:
            continue
        for edge in block.successors:
            if edge.target_block is None:
                continue
            if edge.kind == "call" and not function.contains_body(edge.target_block):
                continue
            pending.append(edge.target_block)

    for edge in edges:
        if edge.kind == "indirect-jump" and edge.source in reachable:
            issues.append(
                f"0x{edge.site:08x} has an unresolved indirect jump; "
                "reachability is incomplete"
            )
    reachability_complete = not issues
    for block in blocks:
        if block.start not in reachable:
            block.flags.add(
                "unreachable" if reachability_complete else "reachability-unknown"
            )
        if any(edge.kind == "indirect-jump" for edge in block.successors):
            block.flags.add("unresolved-successor")
        if any(source > block.start for source in block.predecessors):
            block.flags.add("loop-head")
        if len(set(block.predecessors)) > 2 and any(
            row.control is not None and row.control.kind == "return"
            for row in block.instructions[:8]
        ):
            block.flags.add("common-tail")
        if any(row.va in delay_slots for row in block.instructions):
            block.flags.add("has-delay-slot")

    return Graph(
        function,
        tuple(blocks),
        tuple(edges),
        delay_slots,
        tuple(dict.fromkeys(issues)),
        reachability_complete,
    )

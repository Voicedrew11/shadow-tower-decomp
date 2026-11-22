# Compiler notes for matching

How the compiler that builds this project's game code makes the choices that
decide whether a function byte-matches. The compiler is `cc1psx-272cdk`, the
rebuild of SN Systems' `cygnus-2.7.2-970404 SN32` that the `gcc-2.7.2-cdk`
profiles run with `-O2` and `-G0` or `-G8` (see [the toolchain
page](toolchain.md)). Every rule here was found on a function that did not
match and is stated with the function of this repository where the retail
source form now stands. Causes name the function in GCC's source (the cygnus
2.7.2-970404 tree the rebuild comes from) where they were read there, and were
checked against the RTL dumps; a statement that rests only on the shape of the
output is marked as inference.

The rules explain residues; they do not excuse one, and
[AGENTS.md](../AGENTS.md) still decides what a source may contain. Repeated
statements, extra locals and duplicated tails below are the ordinary source
forms that produced retail's code. Where `volatile` appears, retail evidence
put it there.

Examples give the image and the function's address; the source is found with
`grep -rn 'ADDRESS(0x...' src`. Register names are the assembler's (`v0`,
`a0`, `s0`); "pseudo" is GCC's name for a virtual register before allocation.

## Method

### Dumps

`nix develop` provides `cpppsx-272cdk` and `cc1psx-272cdk`. Preprocess a unit
as the profile does and run `cc1psx-272cdk` on the result with the profile's
flags (`-O2` and `-G0` or `-G8`) plus `-da`. Each pass then writes its RTL to
a file named after the input with one suffix, one section per function:

- `.rtl`: the expansion of the C source, before any optimization.
- `.jump`, `.cse`, `.loop`, `.cse2`: the first jump pass, cse1, loop
  optimization and cse2. `.loop` lists every invariant considered for hoisting
  ("moved" or "not desirable") and every induction value considered for
  strength reduction ("giv of insn N not worth while, A vs B").
- `.flow`, `.combine`: life analysis and instruction combination.
- `.sched`: the first scheduling pass, with its ready lists (`;; ready list at
  T-n`, "insn N has a greater potential hazard").
- `.lreg`: local-alloc. Per pseudo, "Register N used R times across L insns",
  the basic block it lives in, "crosses 1 call", "dies in 2 places".
- `.greg`: global-alloc and reload. ";; N regs to allocate:" is the global
  allocation order, "N conflicts:" and ";; N preferences:" list hard
  registers, and "Spilling reg 64. Register N now in R." records a pseudo
  moved out of HI/LO by reload.
- `.sched2`, `.jump2`, `.dbr`: the second scheduling pass, the second jump
  pass (the only one that cross-jumps) and delay-slot filling.

`kf try --unit UNIT` compiles one unit and diffs it per function; the dumps
answer why the diff is there.

### Pass order

This compiler runs: expansion, jump1, `thread_jumps` and cse1, loop, jump and
cse2, flow, combine, sched1, local-alloc, global-alloc (which runs reload, and
reload runs `reload_cse_regs` at its end), sched2, jump2, reorg. Three
consequences come up again and again:

- sched2 runs before jump2. Cross-jumping merges identical code after
  allocation and after both scheduling passes, so every copy it removes was
  still there for the allocators and the schedulers.
- Reference counts are taken by flow and never lowered (see below), so a use
  that a later pass deletes still counts for allocation.
- The prologue is RTL and is scheduled by sched2 like the body; the epilogue
  is written out as text afterwards and is never scheduled.

### Reading allocation priority

global-alloc sorts pseudos by `floor_log2(refs) * refs / live_length` (times
the register size; `allocno_compare` in `global.c`); ties go to the lower
pseudo number, which is the earlier declaration or first use. local-alloc
ranks its block-local quantities by `floor_log2(refs) * refs * size / (death -
birth)`, with births and deaths counted at two per insn, and ties go to the
earlier birth. Read refs and length from the `.lreg` line of each pseudo.
Facts that decide the numbers:

- flow adds the loop depth for each reference (`reg_n_refs[regno] +=
  loop_depth`), so a reference inside a loop counts two.
- `reg_n_refs` is computed once by flow. combine does not lower it when a use
  disappears (its header says so) and no later pass recounts it. A use that
  combine, `reload_cse_regs`, jump2 or reorg deletes still counts.
- Live length is recomputed by sched1, so it reflects sched1's order. It is
  not a span in the listing: a pseudo gains one for every insn at which it is
  live, block by block, so code that jump2 later merges away still lengthens
  every pseudo that is live through it.
- `update_equiv_regs` in `local-alloc.c` doubles the live length of every
  pseudo with a `REG_EQUIV` note: a pointer initialized to a constant address,
  or a parameter (equivalent to its stack slot). Such pseudos lose ties they
  appear to win. `.lreg` prints the doubled value.
- A divide-and-modulo of one dividend counts the dividend twice.

To fix an allocation, compute what the misplaced pseudo needs (one more
reference, two insns less life) and then look for the source form that gives
exactly that. Most rules in the allocation sections below are such forms.

## Expansion and declarations

**The function's own return type.** Symptom: a branch to the epilogue whose
delay slot holds a later instruction or a nop in retail, where this build
hoists an instruction that writes `v0` (for example a switch table's `lui
v0`). Cause: reorg (`mark_target_live_regs`, `end_of_function_needs`) treats
`v0` as needed at the exit whenever the function's return type is not `void`,
even if no path returns a value. Source: declare the function `int` and let it
fall off the end. Example: ST `0x800168e0`.

**A callee's return type.** A callee declared non-`void` is called as `(set
(reg v0) (call ...))` whether or not the result is used, which puts `v0` among
the registers reload may not hand out again; see "HI/LO pseudos re-allocated
by reload". The source form is the callee's declaration in the caller's file.
Example: VS `0x80021140`.

**Calls without a prototype.** Symptom: a `u8` or `s16` argument passed raw
(`move a0,sN`, a stack argument loaded with `lw` or `lb`) where the prototype
would mask or extend it. Cause: with no prototype in scope the default
promotions apply. Source: an old-style declaration in the caller (`void
fighter_apply_attack_hit();`) or an old-style definition earlier in the file, with
explicit casts where retail does narrow. Examples: VS `0x8002f334`; ST
`0x80036484` (the effect presets above it are old-style definitions); ST
`0x80050768` (a stack argument loaded with `lb`).

**A narrow value passed to an unprototyped callee.** Symptom: `sll v1,a0,1` at
the top of a block and `andi v1,v1,0xff` much later, in the same register.
Cause: a `u8` block-local is computed as `(set (subreg:SI (reg:QI n) 0) ...)`,
which sched1 does not boost (see sched1), and the promotion at the call is a
zero-extension into a fresh temporary that local-alloc ties to the dying
variable. Source: `u8 level = elapsed * 2;` declared in the arm and passed as
is. Example: ST `0x80021654`.

**Narrow parameters and the prologue order.** Symptom: the `sw sN`/`move
sN,aN` pairs of the prologue in an order that is not the argument order.
Cause: each parameter is copied raw in argument order; a narrow parameter is
converted by an insn emitted after all the copies, and combine merges copy and
conversion at the conversion's late position only when the narrow pseudo
survives cse, which it does when its use needs an extension (a `u8` stored to
a `u16` field). A same-width use or an `int` parameter keeps the copy early.
Source: try each parameter as `u8`, `s16` or `int` until the order matches.
Example: ST `0x80050288`. Related: a K&R `short` parameter passed on the stack
is loaded once with `lhu` and extended at each use (ST `0x8004cc70`).

**A local initialised from a parameter.** Symptom: the `move sN,aN` of some
parameters come after the others. Cause: a parameter used directly keeps its
doubled live length; a local copy (`int step = delta;`) does not, outranks the
parameters, and its copy insn comes in the locals' initialisation order.
Source: decide per parameter whether the body uses it or a copy. Examples: ST
`0x800500ac`, ST `tim_load_images` (`0x80031efc`, where using the parameter
directly restored the incoming order), VS `0x800297fc` (a working value that
is the parameter itself, `time += 59;`, keeps the incoming copy as one more
reference).

**A `void` function that returns its last pointer.** Symptom: `lui v1; addiu
v1; subu a1,v0,v1` in retail against `lui a1; addiu a1; subu a1,v0,a1`. Cause:
with `return b;` the copy `v0 = b` lets local-alloc substitute `v0` for `b` in
earlier insns, which unties a later difference from its dying operand. Source:
return the pointer (`Vis88 *` in the prototype). Example: ST `0x800549c8`.

**Variadic functions.** Symptom: `addiu s3,sp,112` and reads `lw/lhu/lbu
N(s3)` of the stacked arguments, the last named one read from its home slot.
Cause: the original took the address of its last named stacked parameter and
indexed it as an `int` array; `va_start` on a register parameter would home
it. Source: `int *args = (int *)&dir;` and `args[1]`, each narrowed at its
use. Example: ST `0x80036484`.

**A member of a global versus a dereferenced constant address.** Symptom: `lui
r,%hi(obj+16); lw r,%lo(obj+16)(r)` against `lui/addiu &obj` and `lw 16(reg)`,
or the reverse; inside a loop one form keeps the base in a saved register.
Cause: a member access (`obj.field`, also a constant index) expands as `R =
&obj; mem(R + 16)`; a dereference of a constant address expression
(`(&obj.first)[2]`, `*(int *)((char *)&obj + 16)`) expands as `R = obj+16;
mem(R)`, which loop.c folds back into the load. A struct `VAR_DECL` is
`BLKmode`, so `viewer.unk38[i]` forces its address into a register, while
`*(viewer.unk38 + i)` goes through `ADDR_EXPR` and yields the constant
`sym+56`. Outside loops the two spellings usually give the same code.
Examples: OPTXT `0x80012d04` (`(&pad.trigger)[2]`), ST `0x8005ad00`
(`*(viewer.unk38 + 2)`), ST `player_update_status` (`0x8003c24c`): `addiu
a0,s0,-400; lw v0,400(a0)` (a base re-derived from a pointer into the object)
is a member access that cse rewrote through a related register, which needed
the member to exist in the type.

**Element addresses assigned to a pointer.** Symptom: the index scaled first
and `lui/addiu sym+K` formed after it, reusing the dead index register. Cause:
`sym.arr[i].field` and its cast variants expand the symbol address first; `p =
&sym.arr[i]` expands the `ADDR_EXPR` as `(plus (mult i size) (const sym+K))`,
which `force_operand` emits multiply first, constant last. Source: give the
element a pointer local. Examples: ST `0x80040024` (`item =
&vis_table_c.unk0118[...]`), ST `0x80023c6c`. A member read whose address is
`sym+OFF` with the base loaded before the index is scaled is a cast
dereference, `*(u16 *)&ARR[i].member` (VS `0x8001c5e0`).

**The address of an element as a value.** Symptom: `sll; addiu v1,v1,412; addu
v1,s3,v1; lhu 0(v1) ... sh 0(v1)` against `addu a0,a0,s3; lhu 412(a0)`. Cause:
fold associates `(sp + 412) + k*2` into `sp + (k*2 + 412)` and a value expands
with the base added last, while a memory operand expands in `EXPAND_SUM` mode
and keeps the displacement. Source: `s16 *len = &sp->unk19C[k];`. Example: ST
`0x8005ad00`.

**Operand order of `base + index`.** `p->arr[i]` gives `addu d,base,idx`;
`*(p->arr + i)` and a real global array give the index first; `ptr_var + off`
always puts the pointer first (`pointer_int_sum` in `c-typeck.c`); an
integer-typed base keeps the written order. Example: ST `0x80065de0`. For a
pointer variable plus a scaled index plus a member offset, only `(ptr + idx*4)
+ offset` keeps the offset last; any expression on the global itself is
re-associated to `(sym+off) + idx*4` (ST `0x8005408c`).

**Assignment chains.** `a.x = a.y = b.z = 0;` forms the outermost
destination's base first (`expand_assignment`), so that base has the lower
insn number although its stores come last, which decides sched1 ties between
two address insns. Example: ST `player_update_status` (`0x8003c24c`). Chained
byte assignments `c.r = c.g = c.b = v;` store the innermost member and re-read
it for the others (VS `0x8002b078`).

**Initialised aggregates.** Symptom: a split constant (`lui; ori`) or argument
loads sitting between groups of constant stores. Cause: an aggregate
initialised at its declaration is stored at the declaration, ahead of later
statements; assignments are scheduled as ordinary statements. With several
initialised aggregates in a block, the stores of all but the last one declared
sink above the argument loads of a call that follows. Source: move constant
member assignments into initialisers, in a block of their own if needed.
Examples: ST `0x80028784`, VS `0x80021140`.

**Conditional stack arguments.** A `?:` passed as a stack argument puts the
register argument loads before the condition (register arguments are
precomputed). Retail's `cond; li v0,2; sw v0,16(sp); addiu a0..; jal` is two
calls in an `if`/`else`. Example: ST `0x80028784`.

**Evaluation order.** The first operand of a comparison or subtraction is
expanded first: `halves[hand] > limit - cost`, not `limit - cost <
halves[hand]` (ST `0x800402ac`); `x = -f->rot + cam` loads the two operands in
the opposite order to `x = cam - f->rot` (VS `0x80024b5c`); `if (lo + 4096 >
angle && ...)` loads the bound first (VS `0x8002dec4`).

**Multi-bit bit-field stores.** Symptom: `li v0,~MASK; and; ori; sw` with the
call's argument load scheduled before it in this build. Cause: `w = (w &
~0x1c00) | 0x400` computes into single-set temporaries, which sched1 boosts; a
bit-field store expands as one pseudo set twice, which it does not. Source: a
bit-field member (`x.field.gate = 1`). Example: FDAT_272 `0x801ed4d0`.

**Shifts and multiplies narrowed at expansion.** A shift whose result is only
stored to a 16-bit field is narrowed to `HImode` and loads its operand
zero-extended (`lhu`), which cse cannot share with a signed test of the same
field; a multiplication is expanded in `int` from the sign-extended operand.
Retail's single `lh` for test and value means `x * 128`, not `x << 7` (VS
`0x800335e8`). Likewise `int h = e->short_field / 2;` divides in 16 bits and
re-extends, `h = e->short_field; h /= 2;` divides the `int` (ST `0x800385b0`),
and `short speed = step; speed *= 4;` keeps the product a narrow copy that cse
does not share with `step * 4`, while `short speed = step * 4;` folds the copy
away (ST `0x80050768`).

**Narrow accumulators.** Symptom: `addu a1,a0,v0; move a0,a1` in a sum loop.
Cause: there is no `QImode` add on MIPS, so `check += block[i]` on a `u8` adds
the paradoxical subregs into a fresh `SImode` temporary and truncates back
with a move. Source: `u8 sum;`. Example: VS `0x8001b3f8`. The `s16` form, a
load into the variable's register, the difference in a scratch and a copy
back, is `s16 left = x; left -= n;` (ST `0x800385b0`).

**Decrement tests.** fold rewrites `n-- != 0` as `--n != -1` (or `!= 0xffff`
for a `u16`), so the body sees the decremented value: `addiu v1,a3,-1; move
a3,v1; andi 0xffff; beq v1,0xffff` is `while (n--)` on a `u16` (ST
`0x80050768`). Ordered comparisons are left alone and only afterwards an
unsigned `> 0` becomes `!= 0`, so `move v0,n; bnez v0,top; addiu n,n,-1` (the
old value tested) is `while (n-- > 0)` with an unsigned `n` (ST `0x800468c0`).

**Constants that fold moves.** fold re-associates constants within one
expression of one type: `(VAR + CON) - ARG` becomes `VAR - (ARG - CON)`, `(CON
- VAR) - ARG` becomes `(CON - ARG) - VAR`, `ARG0 + (CON - VAR)` becomes `(ARG0
+ CON) - VAR`, `a - (b - 16)` becomes `(a + 16) - b`, and `(A + B - k) + k`
cancels to `(plus B A)` with `A` evaluated first. When retail keeps a constant
on the operand where fold would not, write the form that fold maps from, split
the expression into two statements with a block-local intermediate, or give
the inner term another type:

- `vy += 32767 - vx;` for `addiu v0,A,32767; subu v0,v0,B` (VS `0x80014634`).
- `u32 rest = 16 - E; f(rest - c + 2);`, initialised in one statement (VS
  `0x80015020`).
- `row * 4 + 20 - n` for `addiu v0,n,-20; subu i,row4,v0` (ST `0x8005d25c`).
- `top = p->vy - 1600; dy = viewer.vy - top;` (ST `0x800385b0`), `base = Y +
  10; points = X + base;` (ST `0x80054a54`).
- `- (s16)(rand() / 1024 - 16)` keeps `a - (b - 16)` as written (VS
  `0x800335e8`).
- the `(x) + (w)` corners of a rectangle macro give the loads of `r->x + r->w
  - w + w` in retail's order (ST `0x80027d18`).

**Logical tests folded into one mask.** `(x & 2) || (x & 4)` is merged by
`fold_truthop` into one test that reads the field in the target's preferred
mode, a whole word (`lw; andi 6`); `x & 6` reads the halfword (ST
`0x8005f54c`). A two-sided range test on a `short` variable becomes `addiu
1023; andi 0xffff; sltiu 2047` (ST `0x80050768`). A flag argument `andi 0x20;
sltu a0,zero,a0` is `((flags & 0x20) >> 5) != 0`; `!= 0`, `!!` and `?:` all
give `srl 5; andi 1` (ST `0x800385b0`). A `multu` by `0xaaaaaaab` with `srl`
is an unsigned modulo, here `(u32)clock % 96` of a signed field (VS
`0x80015020`).

**Switch statements.** Case bodies are emitted in source order and the jump
table only points at them, so read the table: the order of the bodies in the
listing is the order of the cases in the source (ST `0x8004cee8`, ST
`0x8005ad00`). For a compare tree, `balance_case_nodes` in `stmt.c` takes the
pivot at `(nodes + ranges + 1) / 2` of the sorted list (a range counts two); a
case label that only breaks is a node and leaves no code, because its branch
has the default's target and jump1 deletes it. A tree that pivots one case off
retail is missing an empty `case K: break;` (ST `0x80055374`, ST
`0x80050768`); a `slti/beqz` straight to a case label means both neighbours of
that node are case values. The case that directly follows the tree in the
listing is the first case in the source.

**Stack slots of locals.** Every `BLKmode` local gets an 8-aligned slot at its
declaration, used or not, in declaration order from `sp+16`; reload's own
slots come after them. Two 4-byte structs 4 bytes apart are one array
(`CVECTOR ring[2]`, VS `0x8002b078`); two shorts 4 bytes apart are the `vx`
and `vz` of an `SVECTOR` (VS `0x8002f334`); an untouched 8-byte slot is a
declared, unused aggregate (VS `0x8001fcac`, whose ST copy declares a `RECT
unused`). Initialised local arrays are copied from `.rodata` at the top of the
function in declaration order, which gives their types and order.

**Block structure and frame size.** Each memory-homed local gets a temporary
slot that is freed at its block's `}`, and adjacent free slots are merged, so
sibling blocks share stack. `preserve_temp_slots`, called from
`expand_expr_stmt`, moves every slot whose address was taken and that belongs
to the current brace level up one level, so a local whose address is taken
leaks out of its block as soon as any expression statement runs at the block's
own level after that, and is then never freed. A local is freed only if no
expression statement runs at its declaring level after its address is first
taken. A variable index into a local array counts as taking its address.
Reading retail: locals of different cases side by side are passed by address
at their own level; a group overlapped by later cases' locals was declared one
level above the calls that use it, in a block that closes straight after them;
a later block's request takes the freed region first-fit by size, else splits
the smallest larger slot. `if (c) for (...) {...}` leaks the loop's locals to
the next statement, `if (c) { for (...) {...} }` does not. Examples: VS
`0x800335e8`, ST `0x800385b0`.

## cse and the jump passes before it

**cse canonicalizes to the oldest register.** After a copy `B = A` inside one
extended block every later use goes to the first register of the equivalence
class (`make_regs_eqv`), so "store A, then use B" cannot survive in one block.
The newer register becomes canonical only when it is referenced outside the
cse block and its last use in the function is later. In that case cse also
swaps `A = mem; B = A` into `B = mem; A = B` when the copy directly follows
the load: the variable used last gets the load. Example: ST `0x80036484`. The
same rule decides what cse2 copies a loop's second pointer from (`move s0,s1`
against `move s0,s5`): a hoisted address becomes canonical only if it is used
after the pointer's last use, so retail's form steps the counter first and the
pointer last (`for (cnt = 0; cnt < 64; cnt++, e++)` rather than `do { ...;
e++; } while (++cnt < 64);`, ST `0x800402ac`).

**Known values reused as registers.** cse uses a register known to hold a
value instead of loading the constant: after `while (p->state != END)` the
register compared against `END` is reused for the next `x != END` (ST
`0x80015024`: a plain `while` gives retail's order, a hand-rotated `if` plus
`do`/`while` the other one); `x != -1` after a path where another register
holds -1 becomes `beq x,thatreg` (ST `0x800492f4`); a compare against a
register instead of a loaded constant means a variable known to hold that
value in the block (ST `0x8005ad00`: the constant is the variable's value, `%
3 != 1`). cse also looks constants up in wider modes, so an assignment of a
narrow constant reuses a variable's register when written after it.

**cse only shares registers within one mode.** `insert_regs` looks for a
register of the same mode, so a `QImode` store of 1 and an `SImode` 1 each get
a `li` (VS `0x8001c5e0`, `int on = 1;`).

**Expressions cse does not equate.** cse merges only identical RTL.
`zero_extend(mem)` is not equated with a `QImode` load plus
`zero_extend(reg)`, so `list->entries[k]` read for a test and again as an
index keeps two loads until `reload_cse_regs` deletes one, and the address
stays live across the branch (ST `0x80065de0`; a `u8` local would share the
value and give another allocation); a narrowed `HImode` sum is not equated
with the `SImode` one, so `addu v0,s0,v1 ... addu v0,v1,s0` (one sum formed
twice) means one operand is a 16-bit variable (ST `0x80050768`); `x >> 2` on a
`u8` and the same value read back from a `u8` variable differ (ST
`0x8004e960`); cse never looks inside `zero_extend(mem)`, so `viewer.unkB6 /=
2` reloads while `b6 = viewer.unkB6; viewer.unkB6 = b6 / 2;` copies the stored
register (ST `0x8005a3a8`).

**cse follows jumps only into single-use labels.** A conditional jump is
followed into a label with one use; `-fcse-skip-blocks` also carries the table
across a block that a jump skips when the target label has one use and no
label sits in between. `if (a || b) { G }` is that shape, so `G` still sees
the entry path; `if (a && b && c) {...} else {X}` (one label, three uses) does
not. Symptom: an offset-0 store through a constant pointer comes out as `sb
x,%lo(sym)(hi)` only on the path cse followed from the pointer's
initialisation (`find_best_addr` prefers the `lo_sum` form). Example: FDAT_262
`0x801ed4a0`. With `x = 105; if (c) x = 167;` the table survives the join and
`&sp->rect` folds to a constant; a real `if`/`else` with the stores in the
arms flushes it (ST `0x8002dd70`). A constant assigned before an earlier join
is lost at the join's label (`far = 8000;` ahead of an `if`), so `-far * 2`
stays `negu; sll` (VS `0x80014634`).

**cse1 stops at the end of a loop.** cse.c does not follow a jump to a label
that sits directly after `NOTE_INSN_LOOP_END`, so a `goto` out of a loop
rematerialises constants that code written inside the loop would share with a
hoisted register. If retail reuses a loop-hoisted register in code that runs
after the exit, that code is textually inside the loop. Example: ST
`0x80054a54`. cse2 ignores the note.

**Store forwarding.** cse forwards a just-stored value into a later read
unless another store through the same pointer sits between them. When retail
reloads a field right after storing it, look for a later store through the
same base written between them in the source (ST `world_transform_parts`,
`0x8001cb88`: `sp->part_shift = 0;` as the last statement before the loop).

**Related addresses.** cse expresses an address through the first related one
it meets, so statement order decides which address is formed and which derived
(`S.cursor = buf = S.buffer;` forms the base first; VS `0x80018c84`). A
hoisted copy carries a `REG_EQUAL` note that cse2 can derive from; a user
variable holding the same address cannot be used that way. cse shares only the
high part of an address when a register holding the full address is no longer
valid on the path (ST `0x800456f8`: a variable set to the address at the top
and changed before the switch, both sets dead afterwards; only the high half
survives, so the dead statement itself is inference).

**Volatile memory.** cse does not touch volatile memory, so a field of a
volatile struct keeps `base + offset`; retail's `lhu v0,%lo(S+2)(hi)` for such
a field was a scalar at a constant address, `*(volatile s16 *)&DAT.field` (ST
`0x80068440`).

**Sign extension folded with a shift.** With `a = (short)x; ... a >>= 1;` the
extension is a shift pair through a temporary and cse folds the later `>> 1`
into `>> 17`, which keeps the temporary alive and stops combine from forming
`lh`. Retail's `lh` followed by `sra a,a,1` needs the extension done in the
variable: `a = x << 16; a >>= 16;` (FDAT_012 `0x801ed76c`). `(x << 16) >> 16`
in one expression folds back to the cast.

**Guards threaded into a switch shape.** `if (x != 0 && x != 14) return; if (x
== 0) A; else B;` comes out as `bne x,14,default; j B` followed by A then B,
which looks like a switch with its last test inverted: on the fall-through of
`x != 14` cse knows `x == 14` and folds the second test. Check this whenever a
"switch" has two or three cases and a return default (ST `0x8005975c`).

**jump1 rewrites small if/else.** `if (c) x = 28; else x = 342;` becomes `x =
342; if (c) x = 28;`, whose label has one use, so cse2 walks through it; with
two statements per arm (`y = 35` in both) the join stays a real label (ST
`0x80067bec`). jump1 places the default of `if (...) n = x; else n = 4;` just
ahead of the branch, after the compare's operands; written as `n = 4; if (...)
n = x;` the `li` sits in the middle of the first operand's chain (VS
`0x80020aa4`). Two separate early `return -1` tests of one value get the `v0 =
-1` hoisted for the first; `if (a == -1 || a == 254) return -1;` gives the
uniform shape (ST `0x80058318`). A function whose final `return K` is also the
fall-through of its last arm should write every arm as "return the good value
early": jump1 turns the last arm into `v0 = d; if (r < d) v0 = -1` and the
earlier arms cross-jump (ST `0x8004b8ec`).

**Loop entry tests.** jump1 copies a `while`/`for` test to the loop entry;
jump2 later merges the copy's tail with the bottom test when the two are
textually identical, leaving `j` into the bottom test (ST `0x8001cb88`, ST
`0x8005f54c`). They are identical only if both reload what they test (see
store forwarding above).

## loop.c

**The hoisting budget.** `move_movables` hoists an invariant when `threshold *
savings * lifetime >= insn_count`. `threshold` starts at `(loop_has_call ? 1 :
2) * (1 + n_non_fixed_regs)`, which is 29 in a loop with a call and 58
without, and drops by 3 for every insn already moved from that loop; savings
is the number of uses, lifetime the insns between set and last use,
`insn_count` the loop's RTL insns at loop time. A `lo_sum` "forces" its
`high`, so a two-insn address has doubled savings and summed lifetimes and
needs a loop of more than 116 insns to stay inside. A hoisted value that then
gets no saved register is spilled, and reload rematerialises it in its first
free temporary (`li t0,0xffff` where retail has the constant in `v0` inside
the loop). So a loop constant hoisted against retail, or not hoisted, is
decided by the loop's size and by every invariant moved before it; print the
`.loop` dump and do the arithmetic for each "moved" line before the one that
differs. Forms that changed the count:

- `s++; s++;` instead of `s += 2` adds one RTL insn (combine merges them
  later): ST `0x8002732c`.
- reaching a table through a pointer set before the loop removes two movables:
  ST `0x8002b84c` (`mask = pad_state.unk22;`, which let the frame address
  be hoisted), ST `0x800642e8` (`table = &vis_table_c;` lifts the threshold by
  6, so three viewer addresses are hoisted and the fourth stays, as in
  retail).
- the reverse: a global expression written at each use (VS `0x800182ec`, VS
  `0x800140d8`, `shared_buffer.fighter[i]`) makes its 6-insn chain the first
  movable, which uses up the budget before a later constant.
- the order of two subexpressions: `X[area][i / 32] & BIT(i)` forms the row
  address before the division, lengthens its life, gets it hoisted, and leaves
  a later constant inside (ST `0x80055f9c`, where the extra hoist had cost a
  frame in a leaf).

**User variables are not moved after a conditional.** A user variable's set
inside the loop after a conditional branch is moved only if
`reg_in_basic_block_p`, whose first test is that this insn is the variable's
first reference in the function. A variable referenced anywhere earlier stays:
`u8 *names = NULL;` at the declaration and `names = magic_name_messages;` inside the
`if` keeps the address in the loop, and flow deletes the dead initialiser
afterwards (ST `0x80065de0`). Likewise an assignment to a variable whose first
use is earlier keeps a sign extension in the loop (ST `0x8004cc70`).
Conversely `int y = 29;` declared in the arm that uses it is moved where the
literal is not (VS `0x8002ad40`).

**Loads and aliasing.** loop.c hoists a load only if no store in the loop can
alias it. Its test (`init_alias_analysis` in `sched.c`) knows a pointer's
value only when the pseudo has a `REG_EQUAL` constant note and exactly one
set. `u16 *p = NULL; ... p = viewer.unkC0;` has two sets at loop time, so `*p`
may alias everything and a load retail repeats every pass stays in the loop
(ST `0x8005a3a8`). A load that retail does before a loop with a call is a
statement before the loop (VS `0x80020aa4`).

**Phony loops.** loop.c skips `do { } while (0)` as a phony loop; its notes
still affect cse1 and the schedulers (see sched1).

**Loop exits moved earlier.** `find_and_verify_loops` moves a block of the
form `if (cond) { ...; break; }` out of the loop to the first `BARRIER` found
walking backwards from the exit target. So exit blocks placed before the loop
(in front of the last switch case before it, or before an earlier arm) were
inside the loop in the source, in that order; and a `BARRIER` there means the
arm before the loop ended in its own jump at loop time, i.e. the tail was
written out. Examples: ST `0x80058318`, ST `0x8005335c`, VS `0x800182ec`.

**Members in a loop.** `S.member = x` inside a loop keeps `base + offset`
because loop.c has hoisted the base before combine could fold it, while
outside loops the same access is `%lo(S+OFF)`. No pointer local is needed (ST
`0x80054a54`). A constant address cast (`*(int *)0x8001f7ec`) gives the same
code outside loops but is hoisted inside one, because a symbol's address costs
less than a register and a constant does not (OPTXT `0x80012d04`).

**Strength reduction.** A giv (an induction value derived from a counter) is
reduced when `lifetime * threshold * benefit >= insn_count`, with a threshold
of `(loop_has_call ? 1 : 2) * (3 + n_non_fixed_regs)` (31 with a call). An
inline `64 + i * 36` used in one block has lifetime 1 and stays at its use;
the same expression assigned to a user variable at the loop top lives to its
last use and is reduced. Reduced givs are pseudos numbered after every user
variable, initialised in the pre-header after the hoisted movables, and the
counter itself may disappear. Readings:

- A stepped value whose initialisation sits after the hoisted invariants is a
  giv, not a user variable: user assignments before the loop always precede
  hoisted insns. Write `offset + block * 128` with a counter instead of
  `offset += 128` (ST `0x80061d34`), `y = 48 + i * 36` inside the loop instead
  of a stepped `y` (ST `0x80065770`), `i * 15 + 16` inline at each call (ST
  `0x8002e94c`), `256 + i * 64` in the call (ST `0x8004cee8`), `f =
  &DAT.fighter[i]` in each loop instead of `f++` (VS `0x80029b10`).
- The giv list is built by prepending, so givs are initialised in reverse
  order of their first appearance in the body, and `combine_givs` bases all
  address givs of a counter on the last memory access in the loop's insn
  order. A loop pointer based on the wrong field (`addiu v1,v0,4` against
  `addiu v1,v0,80`) means a different statement was last (FDAT_202
  `0x801ed560`, ST `0x80050768`).
- A giv's step is inserted before its counter's increment. A table address
  stepped after the counter is a giv of a pointer base set before the loop (ST
  `0x800642e8`).
- `basic_induction_var` accepts a constant reset of a counter in an innermost
  loop, and at each reset strength reduction re-initialises every reduced giv
  with `li`. Values initialised after the hoisted constants and re-set to
  their start constants inside the loop are givs of a counter reset with `col
  = 0;` (VS `0x8002ad40`).
- A giv whose insn was moved by loop opt is never "replaceable"
  (`record_giv`), so a hoisted inner invariant that the outer loop reduces
  becomes a copy of the reduced register (`move s8,s6`). In ST `0x8002f1f8`
  and VS `0x800207d4` the layout numbers are eight local variables (`cols =
  25; ... dy = 12;`): `row * th` with `th` in a register is one `mult` that
  forces the user variable's set and is always hoisted, where the literal `row
  * 10` is a three-insn chain that is hoisted only in a loop of at most 116
  insns. The never-allocated constant pseudos have `REG_EQUIV` notes, so
  reload puts the literals back. A `li t2,25; beqz t2` at a loop entry is what
  such a constant-valued local leaves.
- `check_dbra_loop` reverses an up-counting loop with exactly one memory
  store, no memory reads and no calls into a countdown (`li v1,N-1; ... bgez
  v1; p--`): the source is `for (i = 0; i < N; i++) a[i] = 0;` (ST
  `0x80055990`). Written reversed it is not strength-reduced. Bit-field
  read-modify-writes block it; through a pointer local it does not fire (ST
  `0x800642e8`).
- A spilled loop pointer reloaded at every use, with its counter in the next
  slot, is a giv of the counter written as `DAT.fighter[n].field` at each use
  (VS `0x8002b078`).
- Counters written in the `for` header are born before the pre-header and
  outlive loop-made givs; written as `i * 4`, `i * 12` at the call they are
  givs born later, which changes which value is spilled (VS `0x800140d8`).

## flow and combine

**One log link per set.** flow gives a set a `LOG_LINK` only from its first
following use, and combine merges only along those links. `t = a + b; use(t);
x = t;` keeps the copy (`addu v0,v0,s2; move a0,v0`), `t = a + b; x = t;
use(...)` loses it. When retail copies a just-computed value and then uses
only the copy, look for "compute, use, copy" with a copy destination that is
set more than once (ST `0x80067bec`, `row = sp->row0 + i; id = ids0[row]; idx
= row;`).

**Two- and three-insn combinations.** combine may merge a constant pointer
set, a load and an extension as three insns while re-emitting the still-live
pointer set; a two-insn combination that must keep the pointer alive is
rejected. So `if (sp->field == 0)` folds the load to an absolute address, `n =
sp->field; if (n == 0)` with an `int n` keeps the register base (ST
`0x8002dd70`).

**Extensions combine drops.** combine drops a `zero_extend` of an `HImode`
register only when it merges it into a single user. `andi t3,t4,0xffff` before
a multiply means a `u16` variable whose extension cse shared between two uses,
and `addu v0,t4,v0` (the 16-bit variable first) is the paradoxical-subreg form
of a `u16` in an `int` sum (ST `0x80026ecc`, layout values `u16`). An `int`
assigned to a `u16` local and compared as `int` folds to a plain `move`
because the `int`'s nonzero bits are known (VS `0x80013c5c`). A flag known to
be 0 or 1 tested through a narrowing cast (`if (!(short)none)`) or stored in a
`short` reduces to a copy, `move v0,a2; bnez v0` (ST `0x80031674`, ST
`0x80067624`).

**Adjacent repeats removed by combine.** `w &= ~0x20000; w &= ~0x20000;`: cse1
cannot fold the pair because the folded constant is not a valid immediate,
flow counts three references of the mask, combine then merges the two `and`s
into one. The final code has one `and`, the mask ranks with one reference
more. Only an adjacent repeat does this (a non-adjacent one leaves a second
`and`). Example: ST `0x800559bc`, where three masks needed exactly one more
reference each.

**`reg_n_sets` is not always the number of sets.** combine decrements it when
a set disappears into its user, except when the set's source mentions its own
destination or the set survives in the combined insn; sched1 adds one for each
insn it splits, so the `lui` and `ori` halves of a 32-bit constant are two
sets of one pseudo, never boosted, and float to the top of their block.

## sched1

sched1 schedules each block backwards from its end. Equal-priority insns keep
their original order (`rank_for_schedule` falls back to the insn number), so
whatever floats does so past insns of lower priority.

**Single-set destinations are scheduled late.** When an insn becomes ready,
`adjust_priority` raises it to the maximum priority if `birthing_insn_p`
holds: a `(set (reg R) ...)` with `R` a plain register that is live afterwards
and set exactly once in the function. Boosted insns are placed first in the
backward pass, so they stay late in the block; insns that are not boosted sink
to the top of the block in source order. Not boosted: a variable assigned more
than once, a hard-register destination (argument loads), and a `SUBREG`
destination, which is what combine leaves for a narrow user variable computed
in `SImode`. Read `.sched` against `.combine`: the insns that moved up are the
non-boosted ones. Examples:

- `u8 n` instead of `int n` for a digit count keeps its `srl; andi` ahead of a
  `move a2,a0` (ST `0x80027440`, VS `0x8001d9cc`).
- one `next = (u_long *)(p + 1);` variable shared by two branches (the macro
  `NEW_PRIM_NEXT`) is not boosted where a temporary per use is, which moves a
  multiply two insns and every register of the function (ST `0x80026ecc`).
- `int u, v;` declared in each branch is single-set and scheduled after the
  address temporaries (VS `0x8001d3f0`).
- `short w = border >> 8;` instead of `int` (ST `0x80027d18`).
- a size declared once in a loop body and set in two arms sinks to just after
  the call (`sll s0,v0,11`), `int size = f(...) << 11;` in each loop stays in
  the next call's delay slot (VS `0x80018c84` against VS `0x8001789c`, both
  forms in ST `0x8005d25c`).

**Statements after a call show above it.** A non-boosted insn whose pseudo
already crosses calls may move above a call (no dependence on the last call
when `reg_n_calls_crossed > 0`). So an `i = 0` right after a call's argument
loads is the init of a `for` that follows the call (ST `0x8002b2e8`, ST
`0x8002b84c`), loop set-up after the previous call's argument loads was
written after that call (ST `0x80031674`), and a copy store before a call
whose source is in a saved register was written after the calls (ST
`0x8005d25c`).

**Selection within a priority group.** `schedule_select` first queues blocked
insns: a load is blocked for one tick when the insn just placed was a store,
so a load is never placed directly in front of a store. It then takes the insn
with the largest potential hazard, and a ready store beats a ready load, which
beats any other insn. A non-boosted priority-1 load is therefore placed only
when no priority-1 store is ready and ends above every independent store of
its block, dragging up whatever must precede it. Priority is the maximum over
successors of their priority plus latency minus one for data links and their
priority for anti and output links; an insn with no dependents is ready from
the start and is used as filler wherever a latency gap opens (between `mult`
and `mflo`, for example). Examples: ST `0x80062228`, FDAT_012 `0x801eda38`.

**Memory dependences.** sched1's rules (`true_dependence`, `anti_dependence`,
`memrefs_conflict_p`) decide which statements may cross:

- A load through a varying in-struct pointer may pass a store to a
  fixed-address scalar, but not a store to a fixed-address aggregate member,
  and an in-struct read stays ahead of a later store. So `DAT_801cd0e0 = 1`
  spelled as a member keeps retail's order where the scalar extern reorders
  (ST `0x80054a54`, ST `0x80055374`: re-run every function that stores to a
  word after it is folded into an aggregate), an extern read as `X[0]` of
  `extern u8 *X[1]` stays ahead of a store (ST `0x800456f8`), and a tag store
  through `*(u_long *)p` floats while `poly.tag = ...` stays (ST `0x8005aa80`,
  which then reads literally: a store sits before every load that follows it
  in the source).
- A frame address never conflicts with a constant address, and same-base
  stores at other constant offsets never conflict. A stack store may move past
  a load from a global, so "this load came first in the source" does not hold
  between stack locals and globals (VS `0x80014634`).
- A base pseudo with two sets is unknown and conflicts with everything (the
  same rule as loop.c's).
- Volatile references always conflict.
- sched never moves a load above a store through a different base register, so
  a load above stores of the same block was written first (ST `0x8002b84c`,
  where the `vx = level * 8 + 544` statement precedes the constant stores).

**Loop notes are barriers.** `sched_analyze_insn` makes the first insn after a
`NOTE_INSN_LOOP_BEG` or `LOOP_END` depend on every earlier insn and every
later one depend on it, in both passes (the note must lie inside the block,
after its label). A `do { ... } while (0)` (which loop.c ignores) produces it;
anything retail moves behind such a barrier must come after the first insn
that follows the note. Example: ST `0x800402ac`.

**Calls crossed are never reset.** A constant-address local set before a call
is live across it when flow runs; sched1 sinks the set to its use, but
`reg_n_calls_crossed` is not reset for a pseudo that lives in more than one
block, so global-alloc still gives it a saved register. Retail's `addiu
s2,s1,48` (not `a1`) means the pointer was initialised at its declaration
(FDAT_022 `0x801ed8f0`, `Vis48 *c6 = &table.entries[6];`).

**Reading statement order from sched1's output.** sched1 keeps a load early
and sinks a dependent add and store behind the following independent stores,
so a statement sits where its load is, not where its store is (ST
`0x80036484`). Two independent constant loads at the top of a block are in
source order unless one is boosted (ST `0x80048ad0`). A non-boosted assignment
to a variable that sched1 sinks far up makes a function-wide counter conflict
with that block's temporaries, so the loop after such a block had its own
counter (ST `0x800456f8`).

## local-alloc

**Which pseudos local-alloc takes.** Only a pseudo that lives in one basic
block and dies exactly once (`local_alloc`: `reg_basic_block[i] >= 0 &&
reg_n_deaths[i] == 1`). A variable assigned twice in one block ("dies in 2
places") waits for global-alloc, which runs after every local temporary has
taken `v0`/`v1`, so it lands in `a0`/`a1`. A register that avoids `v0`/`v1`
for no other reason is such a variable; a value in `v0` twice with a gap is
two temporaries. Example: FDAT_012 `0x801eda38`. `optimize_reg_copy_2` can
bring a variable down to one death while `reg_n_sets` still counts the
copy-back.

**Ties.** An insn whose operand 0 is a non-earlyclobber output ties it to an
operand that dies there: an operand that must match operand 0 if there is one,
otherwise any dying operand, operand 1 first. A pseudo that is not local
(multi-block, `reg_qty` -1) never ties, so the result falls to the other
operand: `addu v0,v1,v0` with the result in the second operand's register
means the first operand is a function-wide variable (ST `0x800666b0`). An
untied input/output pair means the result already had a quantity. A sum built
step by step in its final register (`lh s0; addu s0,s0,a1; ...; sra s0,s0,7`)
is a block-local set by one expression, to which local-alloc ties the whole
chain (VS `0x8002c2c4`, `int z;` inside each macro expansion).

**A block-local that crosses a call takes `s0` first.** local-alloc runs
before global-alloc and gives a pseudo that lives in one block and crosses a
call the first free saved register. Every global pseudo live across that block
then conflicts with `s0`. Symptom: a pure rotation of saved registers among
long-lived values. Source: give the block-local value its own variable instead
of reusing a long-lived one (`move = ...; f(a); g(move)`), or one variable per
copy of repeated code (ST `tim_load_images`, `0x80031efc`: `data` taken
twice). Example: the shared FDAT handler at `0x801ebec4`
(`src/lib/fdat_common.inc`).

**Copies rewritten by local-alloc.** `optimize_reg_copy_1` (from
`update_equiv_regs`), given `dest = src` with `src` used again later in the
same block, rewrites the later uses to `dest` so that `src` dies in the copy;
it stops at a label, a jump, a loop note or a second set of `dest`. A copy
that survives in a scratch register (`sll v0,v0,7; move a1,v0`) had a second
use when combine ran: the same expression written twice, the first time in a
block-local's initialiser (ST `0x800246e4`, `int inner = (e->unk48 << 7) -
...; range = e->unk48 << 7;`). A loaded word kept in two registers, the first
stored and the copy used, needs the store behind a label with two
predecessors: two variables meeting at a join whose predecessors jump2 then
merges (ST `0x80036484`, `entry = owner; goto mark;`).

**Constants used once are moved to their use.** `update_equiv_regs` tries to
substitute a pseudo set once to a constant and used exactly once, in another
block and outside any loop, into its user; when the constant is not a valid
operand there (a store needs a register) it moves the initialising insn to
just before the use. sched1 has then ordered the block without it. Tell: a `li
K; store` pair one or two insns later than a literal's would be. Source: `int
blocks = 2;` among the declarations and `header.blocks = blocks;` (ST
`0x80062228`).

**A short-lived constant in `v1` instead of `v0`.** Another value was live
across it when local-alloc ran. In ST `0x80066004` the colour stores were
written per style, interleaving the values; cse made every `x = y = 48` reuse
the first 48, whose pseudo was then live across the 96 group, and the 96 (more
references over a shorter life) took `v0`. Retail's tidy grouping by register
is sched2's work (see sched2). Same fix in ST `0x80068258` and ST
`0x8002e310`.

**Results in the register of their constant.** A value that ends in the
register that first held a constant is one variable updated in place: `cap =
300; cap = base * cap / 100;` (ST `player_hit`, `0x8003dbe0`), a quotient
copied and the remainder computed back into the first register (`minutes =
time / 60; hours = minutes; minutes = time - hours * 60;`, ST `0x8002a954`),
`inc = 96; inc += k * inc / 800;` (ST `0x800402ac`). Read retail's register
reuse literally before adding helper temporaries.

**Scratch operands.** The `SCRATCH` operands of a block move (`movstrsi`) are
local quantities of priority 0, allocated last in their block, so the scratch
registers a block copy got show what was live across it.

## global-alloc

**One variable is one pseudo.** A variable used all over a function is one
pseudo with one hard register, the references of every use and the conflicts
of every block. Rotated registers often mean the variable count is wrong, in
either direction:

- function-wide scratch variables (`i`, `d`, `n`, `option`) reused in
  unrelated blocks rank by the sum of their references; block-local
  declarations give each use its own pseudo (ST `0x80042d74`, ST `0x80060334`,
  VS `0x80014634`, ST `0x800385b0` where every case's entry pointer became
  block-local).
- the reverse: loop counters shared by several loops (`int rows, columns;` at
  function scope) rank above hoisted addresses, which is what retail shows in
  ST `0x80029618`. Reading the saved registers tells which loops share a
  counter (VS `0x80020aa4`).
- a counter in different registers in different loop families is different
  variables (ST `0x8005ad00`, ST `0x8005d25c`).
- a scratch `int` reused for two things keeps the first use's register for the
  second (ST `0x800492f4`).
- a value cse has turned into a second read keeps two pseudos live across a
  branch: `lw v0; beqz v0; move a0,v0` was the member read twice in the source
  (ST `0x80023c6c`).

**Preferences.** global-alloc records hard-register preferences (";; N
preferences:" in `.greg`) and merges them between two pseudos when one dies in
the insn that sets the other (`expand_preferences`). Its first pass keeps a
pseudo off any register that a conflicting pseudo prefers, even a
higher-priority one. So a variable passed as a call's second argument
somewhere else in the function prefers `a1` everywhere (ST `0x8005ad00`), a
function-wide scratch drags the preferences of every case that uses it into
each other (VS `0x800335e8`), and a temporary copied into a variable that is
later an argument inherits that register unless the variable was already live
(ST `0x80057fc4`).

**Adding references that leave no instruction.** When the dump arithmetic says
a pseudo needs one or two more references, these source forms add them and are
removed after allocation:

- an assignment split across an `if`/`else if` chain (`if (i == 2) mode = 48;
  else if (A || B) mode = 48; else mode = 16;`): jump2 merges the two sets (ST
  `0x8002e4d0`).
- a tail repeated on a second path, for example the return expression on both
  paths (ST `vis_measure_position`, `0x8001c4c8`, where every value of the
  expression needed one more reference), an increment repeated before each
  `continue` (ST `0x80053aa4`, ST `0x800468c0`, VS `0x80024b5c`), or a value
  computed in both arms (ST `0x8005f54c`).
- a store repeated later in the same straight-line block, removed by
  `reload_cse_regs` (FDAT_202 `0x801ed560`, `b->unk05 = 2;` twice). A zero
  store is not deleted this way.
- an adjacent repeated `&=`, removed by combine (ST `0x800559bc`).
- a branch-not-taken default repeated in an `else`, removed by reorg (ST
  `0x80067bec`).
- one variable used in a second arm (VS `0x800182ec`, `id` from 7 to 10
  references).

**Live length from statement order.** With equal references the shorter life
wins, so the order of initialisers decides: `elemental = physical = 0;` (the
one assigned last lives one insn less; ST `player_hit`, `0x8003dbe0`), `for (i
= 0, name_x = count_x; ...)` against `name_x = count_x;` before it (ST
`0x80031674`), four flags zeroed at the top with the last one zeroed winning
(ST `0x8004c1b4`). In ST `0x8002f1f8` each single-set constant initialiser
adds an insn to the life of a value loaded before them, and eight were needed
to rank it behind a later one; a constant local that never gets a register
costs nothing in the final code.

**Pointers with constant addresses.** A pointer initialised to a constant
address and used once has priority `2 / (2 * len)`, so saved registers among
such pointers follow their live lengths. When no declaration order fits, look
for an ordinary load declared between them (FDAT_102 `0x801ed554`, `int state
= fdat_801eda78;` in the middle of the pointer declarations). Declaring one
pointer local per table entry, even for entries used once, makes retail's
"address rebuilt at every use" appear when the pointer gets no register
(FDAT_092 `0x801ed680`; see reload).

**Hoisted constants are ranked by life.** Equal-use constants hoisted from a
loop rank by live length, so the one hoisted last gets the best saved register
(VS `0x8002ad40`). A hoisted constant that takes a saved register where retail
rematerialises it at each use means a short-lived variable is missing
somewhere in the loop (ST `0x80050768`, a byte held in three `u8` variables).

## Reload and reload_cse

**HI/LO pseudos re-allocated by reload.** Symptom: a product's `mflo`/`mfhi`
lands in `v1`, `a2` or `a3`, never `v0` and never an argument register some
call uses, although `v0` looks free and nothing in the `.greg` conflict lists
explains it. Cause: the product's pseudo gets HI or LO in global-alloc; when
another multiply needs HI/LO as a reload register, `spill_hard_reg` evicts it
and `retry_global_alloc` re-allocates it avoiding `forbidden_regs`. That set
starts from `bad_spill_regs`, which contains every hard register that appears
explicitly in the RTL when reload starts (`regs_explicitly_used`, copied from
flow's `regs_ever_live`): the argument registers of the calls made, and `v0`
as soon as any call is a `call_value`, which is the case whenever the callee's
declared return type is not `void`, whether the result is used or not. Source:
a callee declared `int` in that file (an old-style `int f();`, no prototype,
or a real `int` result that is ignored). A `return x;` in the function also
makes `v0` explicit; a valueless `int` return type does not. Check with
`.greg`: "Spilling reg 64. Register N now in R" with `R` = 2 means `v0` was
not forbidden. Examples: VS `0x80021140` (`int ui_draw_cell_grid();`), FDAT_272
`0x801eec74`, ST `0x8005335c`. Changing a callee's type changes only
reload-register choices, but in every caller.

**Variables without a hard register.** A pseudo that gets no register and has
a `REG_EQUIV` constant is replaced by the constant at each use instead of
being spilled, so retail's `li t0,-1` at a compare (the reload register) is a
function-level variable set once and never changed (ST `0x8005c7fc`, `int
shown = -1;`), an address rebuilt at every use in the same temporary is a
pointer local initialised at its declaration (FDAT_092 `0x801ed680`), and
`$at` forms (`lui at,0x1f80; sh v0,852(at)`) are a scratchpad pointer variable
that got no register (ST `0x80067624`). When the use accepts a register or
memory but not an immediate (`zero_extendhisi2` for a `u16` argument), reload
puts the constant in the function's constant pool with `force_const_mem`
(after the function, padded to 4, in `.sdata` under `-G8`) and loads it
through the reload register: a one-entry "table" whose value some variable of
the function holds is this pool entry (VS `0x800222d8`, `count_x`).

**Spill slots.** Reload hands out stack slots in pseudo-number order, so
spilled user variables get slots in declaration order (`int i;` before `MATRIX
*m;`, ST `0x800468c0`); a `QImode` or `HImode` pseudo with a paradoxical
`SImode` use takes an 8-byte, 8-aligned slot (ST `0x80050768`, VS
`0x800222d8`); loop.c's pseudos come after every user variable (ST
`0x80065770`, ST `0x80067624`); a pseudo that lost its register to a spill
gets its slot after all of these. A pseudo spilled from HI shares one slot per
hard register; a caller-save area needs a call-crossing pseudo in a
call-clobbered register.

**Spilled scalars against arrays.** A spilled pseudo's zero store stays next
to the other register zeroings, while a real memory store floats to the head
of the entry block (ready stores beat everything in sched1). Where a later `x
= 1` lands says nothing: a set with no dependents is used as filler. The same
register on a stack load and on a `li` stored to the same slot is reload's
spill register. In ST `0x8004c1b4` two stack words are two of four scalar
flags that got no register, not an array.

**`reload_cse_regs`.** After reload, a load becomes a `move` when another hard
register is known to hold that memory, and a store of a register that already
holds the destination's value is deleted. Knowledge is dropped at every
`CODE_LABEL` but not at a jump; a store through another base register drops
every remembered memory value, a store through the same base at another offset
drops none. A constant load becomes a copy of a hard register holding the same
constant in the same mode. Readings:

- `move s7,s6` for a literal zero argument after `move s6,zero`: the first
  variable zeroed is an `int` and was zeroed first (ST `0x8005ad00`); `move
  s3,a3` after argument loads is a second zero (ST `0x8002b2e8`).
- a field re-read from the register right after other fields of the same base
  were stored is the natural statement order, not a read squeezed between the
  stores (ST `0x800385b0`).
- two loads of one address with a store through another base between them
  never fuse (ST `0x80036484`).
- the deleted store is the mechanism behind the repeated-store reference above
  (FDAT_202 `0x801ed560`).

## sched2

**Selection after reload.** sched2 uses the same selection as sched1 without
the boost. In each priority group a ready store beats a ready non-memory insn
("insn N has a greater potential hazard") whatever their order, and a load is
blocked for one tick behind a store. A load is lifted as far up as the
anti-dependences on its own hard register allow and settles in front of the
first non-memory insn above the stores, so loads need no help to sit early;
what sched1 must deliver is the order of the stores (equal-priority stores
keep their order through both passes) and the allocation. sched2 pulls a later
independent store up into a load-delay slot but never pushes an earlier one
down: zero stores sitting in the gap of a later statement's load were written
after it (ST `0x80050768`, ST `0x800468c0`).

**Prologue saves.** sched2 schedules the prologue block backwards; each `sw
sN` comes right before the insn that first sets the register. All saves have
priority 1 and normally come out `ra` first, `s0` last. The save emitted
first, ahead of `sw ra`, belongs to the register set by the function's first
body insn when that insn is not a load or store: the saves (stores) are picked
ahead of it one by one except the one that must wait for it. Source: declare
and initialise that variable first (ST `0x800666b0`, `MenuScratch *sp =
MENU_SCRATCH;` ahead of the initialised arrays). reorg later moves the `li s0`
into a call's delay slot, which hides it.

**Stores grouped by register.** Stores that use the same register must precede
the next load into it (anti-dependence), and the load is blocked in front of a
store, so sched2 groups stores by register and keeps source order within each
group: retail's tidy grouping can hide an interleaved source (ST
`0x80066004`), and the order within each register group is the source order
(ST `0x8002e94c`, channel by channel). The `li` of call-saved constants placed
in front of earlier stores, read alone, give the assignments' source order (ST
`0x8002b84c`).

**Memory dependences after reload.** sched2's dependences are exact for the
same base register and conservative between different ones (an `N(s4)` load
and `N(sp)` stores always conflict), and volatile references always conflict.
Three byte stores kept in source order where sched2 would reorder them are
stores to volatile memory (ST `0x80054000`, `volatile CdlLOC unk1DC;` in the
header, written by a CD callback); a `(volatile T *)&obj.member` cast gives
the constant-address form instead.

## jump2 and cross-jumping

**What jump2 merges.** Only jump2 cross-jumps, and it runs after allocation
and sched2. For an unconditional jump it first compares the code that falls
into the label (`find_cross_jump (insn, label, 1)`: one matching insn is
enough) and only then other jumps to the same label (two insns minimum), and
the second search runs only for labels that existed when the pass started
(`INSN_UID (JUMP_LABEL (insn)) < max_uid`). A label created by cross-jumping
gets no further merging. Readings:

- identical tails that retail did not merge were written out in every arm,
  without a source-level join label: `if (cond2) x = a / b; else x = 255;`
  with the stores in both arms rather than `x = cond ? 255 : a / b;` (ST
  `0x80028784`), the call written in each arm instead of `continue` to one
  call (VS `0x80013c5c`).
- the arm that falls through into the shared tail is the else arm; the
  then-arm's copy is deleted.
- arms that share only one final store with the last arm and are not merged
  with each other end with the same store from the same register; when this
  build merges the siblings instead, the last arm's final insn differs (VS
  `0x80014634`).

**Duplicated tails.** Code that appears once in retail was often written on
two or more paths. The copies are real for every pass before jump2: they add
references, they lengthen the life of pseudos live through them, and they put
the code in different basic blocks for cse and the schedulers. Signatures and
examples:

- an unfilled load-delay nop or unswapped loads at a loop latch or join: the
  tail (`face = next; continue;`) was repeated, so the loop test was its own
  block when sched2 ran (ST `mesh_div_draw_gt4`, `0x8001b5dc`).
- an argument move in the delay slot of the branch that skips a block, `jal;
  nop` after the join and a saved-register rotation: `if (c) { A; X; } else {
  X; }` (ST `0x80049544`).
- `j L+N` into the middle of another arm's call sequence: plain per-arm calls
  (ST `0x8004cee8`, ST `0x80054a54`). An argument loaded ahead of the common
  code is a variable set before a `goto` to one call (`effect = 24; goto
  hit;`).
- a store after a join ahead of the next call's argument loads, or reloads of
  spilled variables in both arms before a shared store: the statements were in
  both arms, and the merged tail starts at the first insn that differs (ST
  `0x8002dd70`, ST `0x80031674`, ST `0x80067624`, ST `0x80067bec`).
- a load that stays behind the previous statement's load with an unfilled nop
  where this build hoists it: a block boundary, not a memory dependence (ST
  `0x80031674`, ST `0x80067bec`).
- arms ending `mult; li a2,1; j <into the last arm's mflo>`: each arm has its
  own `/ 4096` and store (ST `0x8004c1b4`).
- a block shown once before two loops and entered from both: written in both
  loops; loop.c moved the exit blocks and jump2 merged them (ST `0x80058318`).
- an `if`/`else` chain of calls with the argument loaded above the first test:
  a variable set before the chain (ST `0x800385b0`).
- which of several identical tails cross-jump depends on the order of the
  stores before them through different base registers, which sched keeps (ST
  `0x80050768`).

Write every case's statements in full, in retail's body order, and jump2
reproduces retail's merges; which tails merge is not a source property to
imitate (VS `0x80032404`, ST `0x80036484`). When a residue needs "a label
here" or "this pseudo set twice", a duplicated arm or `case` group is a
candidate.

**Identical arms vanish.** A conditional with identical arms disappears with
its compare: jump2 merges the arms, the branch becomes a jump to the next
insn, and `delete_jump` removes the insns that fed it when their registers die
there (ST `0x80036484`).

**Returns.** An explicit `return 0;` in an arm is a set of `v0` that sched1
sinks to the top of the arm; `j epilogue; move v0,zero` at the end of an arm
that uses `v0` freely means the arm falls out to the function's final return
(ST `0x800642e8`).

## reorg (delay slots)

**Branch prediction and the `for` increment.** `relax_delay_slots` swaps a
conditional branch followed by an unconditional jump when `mostly_true_jump`
predicts the branch taken. One of its rules is "target label immediately
followed by `NOTE_INSN_LOOP_VTOP`", which is where the `continue` label of
`for (...; cond; inc)` ends up, so every `continue` there is predicted taken.
Retail's unswapped `bcond A; nop; j B` means the increment was in the body:
`for (...; cond;) { ...; p++; }`. EQ branches are predicted not taken, NE
taken. Example: ST `0x80016a14`.

**Filling from the target or the fall-through.** reorg fills a slot from the
target thread when the register is dead on the other path, so `move a2,zero`
in the slot of the branch to an else arm, dead on the fall-through, is the
first statement of the else arm (ST `0x8005d25c`). It fills a call's slot from
after the call too: `jal f; li s7,8` then `li v0,8; sb v0` means the store
came first in the source (ST `0x8005ad00`). `fill_slots_from_thread` skips
candidates whose register is live at the target and takes the first that is
not (ST `0x80029618`). The arm order of an `if`/`else` decides which arm's
first insn can fill the slot (ST `0x8005ad00`, `if (v >= 0) v--; else v = n -
1;`). `bnez tries,open; addiu tries,-1` with `j ret0; move v0,zero` after it
is `if (!tries) return 0; tries--; goto open;` (VS `0x8001c378`).

**Delay slots hide declaration order.** reorg fills the slot with the nearest
earlier insn it can move, so the last pointer initialiser of a prologue in the
first branch's slot may come from a declaration before others; check against
the `sw sN` positions (FDAT_262 `0x801ed4a0`).

**Redundant insns at the target.** When the first insn at a branch target
repeats the slot's insn, `relax_delay_slots` redirects the branch past it and
the repeat disappears. A constant stored before a test and again at the start
of an arm (`sp->r.x = 56;`) leaves only an odd register (`li t0,56`): the
repeat kept the constant alive across the arm's argument loads at allocation
time (VS `0x80021984`). A `j` around nothing, filled from its own target, is a
default repeated in an `else` (ST `0x80067bec`).

**Volatile resources.** reorg's resource sets carry a volatile flag and
`resource_conflicts_p` returns true as soon as either side has it;
`fill_simple_delay_slots` adds a rejected candidate's resources to the set it
checks the earlier ones against. So a store to volatile memory just before a
call leaves the call's slot `nop` and blocks every earlier candidate (ST
`0x800159d4`, `volatile CdlLOC *pos`).

**Placeholders make registers live.** An insn that a call or jump slot took
from in front of it leaves an `(use (insn))` placeholder (`update_block`), and
`find_dead_or_set_registers` treats the placeholder's sets as live for every
thread that runs into it, for the rest of reorg. So a slot left empty in front
of straight-line code ending in the function's last `li v0,K` means that value
was followed by a jump at reorg time: `...; return 1; } return 0; }` (ST
`0x8004fdd0`). `mark_target_live_regs` starts from flow's block information
and simulates from the previous `BARRIER`; a missing `REG_DEAD` note does not
keep an insn out of a slot.

**Shared high parts.** One `lui` in a delay slot can serve a low half in each
arm, so a second arm's `addiu` has no relocation of its own in the target
listing; `config/retail/relocs.tsv` needs a row for each pairing, and a `lui`
before a jump pairs with the low half at the jump's target.

## `-G8`, data layout and translation units

**Small data in a `-G8` unit.** cc1 writes all of a `-G8` unit's data (`.comm`
and `.lcomm` included, wherever the definition sits in the source) before its
text, so ASPSX knows every small variable the unit defines and addresses it
through `$gp`. A small variable the same code reaches with `lui at,hi; sw
x,lo(at)` or `lui r; lw r,lo(r)` is defined in another object: `extern int
name;` here, `int name;` there (OPTXT `0x80012d04`, `opening_state` in
`src/optxt/state.c`). Conversely a `$gp` read of another unit's `.sbss` word
proves the two share a translation unit, and constant pools that continue each
other confirm it (ST `0x800456f8`, which merged five units).

**Symbol size under `-G8`.** cc1 treats an extern as small data only when its
size is known and at most 8 bytes, and then emits one macro instruction (`lw
$3,X`, `la $5,S`) that ASPSX expands with the destination as its own temporary
(`lui a1; addiu a1,a1,lo`). A separate temporary (`lui v0; lw v1,lo(v0)`)
means cc1 saw an object larger than 8 bytes, a member of a bigger struct (ST
`0x80042d74`, where `DAT_801cd0e0` is spelled as the member
`game_progress.unk16D8` by a macro in `include/st/menu.h`). An `extern s32
X[];` has no size: declare `X[1]` or the scalar (VS `0x80029f28`,
`round_wins[1]`, after its unit moved to the `-G8` profile). In `-G0` units
both spellings give the same bytes.

**Small initialised locals.** cc1 puts any constant of at most 8 bytes in
`.sdata` under `-G8` but marks only `VAR_DECL`s as `$gp`-addressable, so a
small initialised local (`u8 frames[6]`, `RECT strip`) lives in `.sdata` and
is addressed with a split `lui/addiu`. Under `-G0` it would be in `.rodata`
(ST `0x800385b0`).

**`.lcomm` and COMMON order.** cc1 emits file-scope statics as `.lcomm` in
declaration order, including unreferenced ones; function-local statics before
their function's text, file-scope ones at the end of the file. A header
`extern int NAME;` that precedes a unit's `static int NAME;` makes cc1 emit
that word first, because its decl node is created at the extern. PSYLINK
places COMMON words 8-aligned after the first unit's `.lcomm` statics, in its
own order. So an FDAT overlay whose scene words or gaps do not match retail is
one translation unit with the words `static` in address order, and a 4-byte
hole before the first referenced static is an unreferenced `static int`
declared first (FDAT_172, FDAT_262, FDAT_202 and the ending overlays 442 to
472).

**Switch tables in overlays.** A pad word before a `.align 3` switch table
appears only when the table shares an object with the overlay's header, one
translation unit per overlay.

## Functions still not exact

Five functions of 1,934 are not exact at this point; the last known condition
for each:

- **ST `world_project_parts` (`0x8001cdf8`)**, same size and instructions.
  Retail keeps the `unk38` copy (`lw v0,52(t0)` ... `sw v0,56(t0)`) behind the
  two vertex pointer stores; sched1 here lifts it above the `header->vertices`
  load, which swaps `v0`/`v1` between them. With retail's sched1 order
  local-alloc gives every register by itself, but only while the offset, the
  model, the sum and the copy are single-death block locals. Raising the
  copy's store to the vertex stores' priority is not enough: its load is a
  boosted temporary that is blocked behind the two vertex stores and lands
  above the sum, where the offset still holds `v0`. Retail needs the copy's
  load to depend on the sum, and for these same-base constant offsets sched1
  has no memory or register rule that gives it.
- **FDAT_012 `0x801eda38`**, size-exact. Retail keeps the 1 of the two
  `bytes[]` stores and then the record index in `a0` and the `unk70` mask in
  `a1`; here they are swapped, and the three sit at the top of the block. The
  plain statements in retail's order reproduce the block once two pseudos are
  global-alloc's instead of local-alloc's: the mask constant (it must still be
  live when the record index is born) and the 1, in a variable set more than
  once so that its `li` is not boosted and rises to overlap `v0` and `v1`. One
  variable for the 1 and the index puts the stores above the constants. Open:
  a construct that references both again without leaving an instruction, or a
  block-local 1 with two sets and one death.
- **VS `0x800140d8`**, frame only: retail's frame is 80 bytes, this build's
  64. `sp+16..31` is never referenced and the loop's spill sits at `sp+32`:
  either a 32-byte outgoing argument area of a call with seven or eight
  arguments that left no code, or an unused 16-byte local. Nothing in the
  image names either.
- **VS `0x8001c5e0`**, frame only: 56 bytes against 48, with eight untouched
  bytes at `sp+24` after `zero`. Allocation matches, HI is not spilled and
  nothing is caller-saved, so it is a declared, unused local of up to 8 bytes
  that no twin names.
- **FDAT_332 `0x801ed9e8`**, frame only: retail reserves 40 bytes and saves
  the four registers from `sp+24`, this build 32 from `sp+16`. The eight bytes
  at `sp+16` are never accessed: an unused 8-byte local (sibling scenes keep
  an `SVECTOR` there) or the third word pair of outgoing arguments of a
  five-argument call that was optimized away.

The three frame residues need a real cause (a local, an aggregate, an argument
area); AGENTS.md rules out padding to close them.

"""Controls for image-qualified, confidence-preserving data ownership closure."""

from __future__ import annotations

import io
import unittest
from contextlib import redirect_stdout
from dataclasses import replace
from types import SimpleNamespace

from scripts.kf.cli import main as cli_main
from scripts.kf.data_reachability import DataExtent, access_span, audit
from scripts.kf.relocations import DataReferent, validate_relocation
from scripts.kf.sema.evidence import Evidence, Origin, Reference
from scripts.kf.sema.image import RetailImage
from scripts.kf.sema.index import Binding


IMAGE = "GAME.EXE"


def function(va=0x1000, *, vendor=False, image=IMAGE, fragments=1):
    return Binding(image, va, 0x20, "text", "function", f"fn_{va:x}", "",
                   "test", "test", body_size=0x20, fragments=fragments,
                   provider="psyq" if vendor else "")


def datum(va, size=8, *, source="DATA", image=IMAGE, storage="load"):
    return DataExtent(image, va, size, f"data_{va:x}", storage, source)


def ref(site, target, *, tier="validated", kind="address", image=IMAGE, origins=()):
    return Reference(image, site, target, kind, None, tier=tier, origins=origins)


def words(*values):
    import struct

    return struct.pack(f"<{len(values)}I", *values)


class ReachabilityTest(unittest.TestCase):
    def test_bios_vector_stub_is_a_witnessed_vendored_dispatch(self):
        # li $t2,0xb0; jr $t2; li $t1,0x32
        code = words(0x240A00B0, 0x01400008, 0x24090032)
        img = RetailImage.synthetic(IMAGE, 0x1000, code + b"\0" * 0x100)
        report = audit(IMAGE, (function(0x1100), function(0x1000, vendor=True)), (),
                       (ref(0x1100, 0x1000, tier="proven", kind="call"),
                        ref(0x1004, None, tier="proven", kind="jump")), img)
        self.assertEqual(report["issues"], [])
        self.assertEqual(report["vendored_dispatch"], [{
            "owner": "function:00001000", "site": 0x1004,
            "pattern": "vendored-indirect", "proof": "bios-vector-stub", "target": 0xB0,
        }])
        self.assertEqual(report["summary"]["proven_vendored_dispatch"], 1)

    def test_empty_link_time_table_loop_is_proven_dead(self):
        # lui $s1,0; addiu $s1,$s1,0; beqz $s1,+3; nop; jalr $t0
        code = words(0x3C110000, 0x26310000, 0x12200003, 0, 0x0100F809)
        img = RetailImage.synthetic(IMAGE, 0x1000, code + b"\0" * 0x100)
        report = audit(IMAGE, (function(0x1100), function(0x1000, vendor=True)), (),
                       (ref(0x1100, 0x1000, tier="proven", kind="call"),
                        ref(0x1010, None, tier="proven", kind="indirect-call")), img)
        self.assertEqual(report["issues"], [])
        self.assertEqual(report["vendored_dispatch"][0]["proof"], "empty-link-time-table")

    def test_unproven_vendored_indirect_control_is_an_exempt_witness(self):
        # jr $v0 through an unproven value: exempt by project decision
        # (2026-11-17), but still listed as a witness row.
        code = words(0x00400008, 0)
        img = RetailImage.synthetic(IMAGE, 0x1000, code + b"\0" * 0x100)
        report = audit(IMAGE, (function(0x1100), function(0x1000, vendor=True)), (),
                       (ref(0x1100, 0x1000, tier="proven", kind="call"),
                        ref(0x1000, None, tier="proven", kind="indirect-jump")), img)
        self.assertEqual(report["issues"], [])
        self.assertEqual(report["vendored_dispatch"], [{
            "owner": "function:00001000", "site": 0x1000,
            "pattern": "vendored-indirect", "proof": None, "target": None,
        }])
        self.assertEqual(report["summary"]["vendored_indirect_exempt"], 1)
        self.assertEqual(report["summary"]["proven_vendored_dispatch"], 0)

    def test_game_indirect_control_is_not_exempt(self):
        # The same unproven jr $v0 in a game function blocks closure.
        code = words(0x00400008, 0)
        img = RetailImage.synthetic(IMAGE, 0x1000, code + b"\0" * 0x100)
        report = audit(IMAGE, (function(0x1000),), (),
                       (ref(0x1000, None, tier="proven", kind="indirect-jump"),), img)
        self.assertEqual(report["summary"]["issues"]["unresolved-indirect-control"], 1)
        self.assertEqual(report["vendored_dispatch"], [])

    def switch_code(self, table_words):
        # andi v1,v0,0xff; sltiu v0,v1,3; beqz v0,+7; lui v0,0; addiu v0,v0,0x40;
        # sll v1,v1,2; addu v1,v1,v0; lw v0,0(v1); nop; jr v0; nop
        code = words(0x304300FF, 0x2C620003, 0x10400007, 0x3C020000, 0x24420040,
                     0x00031880, 0x00621821, 0x8C620000, 0, 0x00400008, 0)
        return code + b"\0" * (0x40 - len(code)) + words(*table_words)

    def test_bounded_game_switch_table_is_a_proven_dispatch(self):
        img = RetailImage.synthetic(IMAGE, 0x0, self.switch_code((0x2C, 0x2C, 0x28)) + b"\0" * 0x100)
        game = replace(function(0x0), size=0x40, body_size=0x40)
        report = audit(IMAGE, (game,), (), (ref(0x24, None, tier="proven", kind="jump"),), img)
        self.assertEqual(report["issues"], [])
        self.assertEqual(report["switch_dispatch"], [{
            "owner": "function:00000000", "site": 0x24, "pattern": "switch-table",
            "table": 0x40, "entries": 3,
        }])

    def test_game_switch_table_leaving_the_function_stays_unresolved(self):
        img = RetailImage.synthetic(IMAGE, 0x0, self.switch_code((0x2C, 0x80, 0x28)) + b"\0" * 0x100)
        game = replace(function(0x0), size=0x40, body_size=0x40)
        report = audit(IMAGE, (game,), (), (ref(0x24, None, tier="proven", kind="jump"),), img)
        self.assertEqual(report["summary"]["issues"]["unresolved-indirect-control"], 1)
        self.assertEqual(report["switch_dispatch"], [])

    def switch_table(self, last_entry, *, vendor=False, unresolved=True):
        # sltiu $v0,$v1,2; beqz $v0,+7; lui $v0,0; addiu $v0,$v0,0x1100;
        # sll $v1,$v1,2; addu $v1,$v1,$v0; lw $v0,0($v1); nop; jr $v0; nop
        code = words(0x2C620002, 0x10400007, 0x3C020000, 0x24421100, 0x00031880,
                     0x00621821, 0x8C620000, 0, 0x00400008, 0)
        table = words(0x1028, last_entry)
        img = RetailImage.synthetic(IMAGE, 0x1000, code + b"\0" * (0x100 - len(code)) + table
                                    + b"\0" * 0x100)
        switch = replace(function(0x1000, vendor=vendor), size=0x30, body_size=0x30)
        functions = (switch, function(0x1400)) if vendor else (switch,)
        refs = (ref(0x1400, 0x1000, tier="proven", kind="call"),) if vendor else ()
        if unresolved:
            refs += (ref(0x1020, None, tier="proven", kind="indirect-jump"),)
        return audit(IMAGE, functions, (), refs, img)

    def test_bounded_switch_table_inside_the_function_is_proven(self):
        report = self.switch_table(0x102C)
        self.assertEqual(report["issues"], [])
        self.assertEqual(report["switch_dispatch"], [{
            "owner": "function:00001000", "site": 0x1020,
            "pattern": "switch-table", "table": 0x1100, "entries": 2,
        }])

    def test_switch_table_leaving_the_function_stays_unresolved(self):
        report = self.switch_table(0x1200)
        self.assertEqual(report["summary"]["issues"]["unresolved-indirect-control"], 1)
        self.assertEqual(report["switch_dispatch"], [])

    def test_switch_witness_needs_no_unresolved_reference(self):
        # The semantic evidence folds a proven game switch into the CFG, so
        # the audit sees no reference at the `jr`; the witness row remains.
        report = self.switch_table(0x102C, unresolved=False)
        self.assertEqual(report["issues"], [])
        self.assertEqual([row["site"] for row in report["switch_dispatch"]], [0x1020])

    def hoisted_switch(self, base_load=None):
        # lui t7,0; addiu t7,t7,0x80 (the hoisted base, or BASE_LOAD);
        # addiu v1,a0,-1; sltiu v0,v1,2; beqz v0,0x30; sll v0,v1,2;
        # addu v0,v0,t7; lw v0,0(v0); nop; jr v0; nop;
        # 0x2c: addiu a0,a0,1; 0x30: jr ra; nop
        head = (0x3C0F0000, 0x25EF0080) if base_load is None else (0, base_load)
        code = words(*head, 0x2483FFFF, 0x2C620002, 0x10400007, 0x00031080,
                     0x004F1021, 0x8C420000, 0, 0x00400008, 0, 0x24840001,
                     0x03E00008, 0)
        img = RetailImage.synthetic(IMAGE, 0x0, code + b"\0" * (0x80 - len(code))
                                    + words(0x2C, 0x30) + b"\0" * 0x100)
        game = replace(function(0x0), size=0x38, body_size=0x38)
        return audit(IMAGE, (game,), (), (ref(0x24, None, tier="proven", kind="jump"),), img)

    def test_hoisted_switch_table_base_is_proven_by_reaching_definitions(self):
        report = self.hoisted_switch()
        self.assertEqual(report["issues"], [])
        self.assertEqual(report["switch_dispatch"], [{
            "owner": "function:00000000", "site": 0x24,
            "pattern": "switch-table-hoisted", "table": 0x80, "entries": 2,
        }])

    def test_hoisted_switch_with_an_unknown_base_stays_unresolved(self):
        report = self.hoisted_switch(base_load=0x8CAF0000)  # lw t7,0(a1)
        self.assertEqual(report["summary"]["issues"]["unresolved-indirect-control"], 1)
        self.assertEqual(report["switch_dispatch"], [])

    def test_vendored_switch_table_is_exempt_but_not_switch_proved(self):
        report = self.switch_table(0x102C, vendor=True)
        self.assertEqual(report["issues"], [])
        self.assertEqual(report["switch_dispatch"], [])
        self.assertEqual([(r["site"], r["pattern"]) for r in report["vendored_dispatch"]],
                         [(0x1020, "vendored-indirect")])

    def audit(self, functions, data=(), refs=(), img=None):
        img = img or RetailImage.synthetic(IMAGE, 0x1000, b"\0" * 0x1000)
        return audit(IMAGE, tuple(functions), tuple(data), tuple(refs), img)

    def test_all_game_functions_are_roots_without_source_or_exact_scores(self):
        report = self.audit([function(), function(0x1100), function(0x1200, vendor=True)])
        self.assertEqual(report["roots"], ["function:00001000", "function:00001100"])
        self.assertEqual(report["summary"]["reached_vendor_functions"], 0)
        self.assertFalse(report["complete_reachable_bytes_proven"])

    def test_empty_root_set_is_not_a_vacuous_success(self):
        report = self.audit([function(vendor=True)])
        self.assertIn("no-game-roots", report["summary"]["issues"])

    def test_optional_vendor_entry_traverses_startup_without_changing_game_roots(self):
        img = RetailImage.synthetic(IMAGE, 0x1000, b"\0" * 0x1000, entry=0x1100)
        report = audit(IMAGE, (function(), function(0x1100, vendor=True),
                               function(0x1200, vendor=True)), (),
                       (ref(0x1100, 0x1200, tier="proven", kind="call"),), img,
                       include_entry=True)
        self.assertEqual(report["summary"]["game_roots"], 1)
        self.assertEqual(report["summary"]["reached_vendor_functions"], 2)
        self.assertEqual(report["entry_roots"], ["function:00001100"])

    def test_unmodelled_entry_is_an_explicit_gap(self):
        img = RetailImage.synthetic(IMAGE, 0x1000, b"\0" * 0x1000, entry=0x1800)
        report = audit(IMAGE, (function(),), (), (), img, include_entry=True)
        self.assertEqual(report["summary"]["issues"]["entry-owner-not-unique"], 1)

    def test_overlapping_entry_owners_stay_candidate(self):
        img = RetailImage.synthetic(IMAGE, 0x1000, b"\0" * 0x1000, entry=0x1010)
        report = audit(IMAGE, (function(0x1800), function(0x1000, vendor=True),
                               function(0x1010, vendor=True)), (), (), img, include_entry=True)
        self.assertEqual(report["summary"]["issues"]["entry-owner-not-unique"], 1)
        vendors = [r for r in report["ranges"] if r.get("provider")]
        self.assertEqual([r["reachability"] for r in vendors], ["candidate", "candidate"])

    def test_bss_is_accounted_without_inventing_initialized_bytes(self):
        report = self.audit([function()], [datum(0x9000, storage="bss")], [
            ref(0x1000, 0x9000),
        ])
        self.assertEqual(report["issues"], [])
        bss = next(r for r in report["ranges"] if r["kind"] == "data")
        self.assertEqual(bss["storage"], "bss")

    def test_cross_owner_hi_lo_pair_remains_a_diagnostic(self):
        reference = replace(ref(0x1000, 0x1200), paired_site=0x1100)
        report = self.audit([function()], [datum(0x1200)], [reference])
        self.assertIn("relocation-pair-crosses-owner", report["summary"]["issues"])

    def test_transitive_vendor_and_pointer_chain(self):
        report = self.audit(
            [function(), function(0x1100, vendor=True)],
            [datum(0x1200), datum(0x1300), datum(0x1400)],
            [ref(0x1000, 0x1100, tier="proven", kind="call"),
             ref(0x1100, 0x1200), ref(0x1204, 0x1300, kind="pointer")],
        )
        self.assertEqual(report["summary"]["reached_vendor_functions"], 1)
        self.assertEqual(report["summary"]["source_owned_data_ranges"], 2)
        self.assertEqual(report["issues"], [])
        end = next(r for r in report["ranges"] if r["va"] == 0x1300)
        self.assertIsNotNone(end["via_reference"])
        self.assertIn("00001200", end["via_owner"])

    def test_cycles_terminate_and_do_not_duplicate_allocations(self):
        report = self.audit([function()], [datum(0x1200), datum(0x1300)], [
            ref(0x1000, 0x1200), ref(0x1200, 0x1300, kind="pointer"),
            ref(0x1300, 0x1200, kind="pointer"),
        ])
        self.assertEqual(len(report["ranges"]), 3)
        self.assertEqual(len(report["references"]), 3)

    def test_candidate_path_never_promotes_downstream_validated_reference(self):
        report = self.audit([function()], [datum(0x1200), datum(0x1300)], [
            ref(0x1000, 0x1200, tier="candidate"), ref(0x1200, 0x1300, kind="pointer"),
        ])
        data = [r for r in report["ranges"] if r["kind"] == "data"]
        self.assertEqual([r["reachability"] for r in data], ["candidate", "candidate"])
        self.assertEqual(report["summary"]["issues"]["candidate-reference-path"], 2)

    def test_stronger_path_revisits_a_previously_candidate_subgraph(self):
        report = self.audit(
            [function(), function(0x1100, vendor=True)],
            [datum(0x1200), datum(0x1300)],
            [ref(0x1000, 0x1200, tier="candidate"),
             ref(0x1004, 0x1100, kind="call", tier="proven"),
             ref(0x1100, 0x1200), ref(0x1200, 0x1300, kind="pointer")],
        )
        end = next(r for r in report["ranges"] if r["va"] == 0x1300)
        self.assertEqual(end["reachability"], "validated")
        self.assertEqual(report["summary"]["issues"]["candidate-reference-path"], 1)

    def test_reviewed_rejection_is_retained_but_not_traversed(self):
        origin = Origin("rejected", "raw-word", "test", "rejected", "review")
        report = self.audit([function()], [datum(0x1200)], [
            ref(0x1000, 0x1200, tier="rejected", origins=(origin,)),
        ])
        self.assertEqual(report["summary"]["source_owned_data_ranges"], 0)
        self.assertEqual(report["issues"], [])
        self.assertEqual(report["references"][0]["origins"][0]["status"], "rejected")

    def test_invalid_live_row_is_not_silently_removed(self):
        origin = Origin("rejected", "reachable-code", "test", "candidate", "test",
                        "decoded-target-mismatch")
        report = self.audit([function()], refs=[
            ref(0x1000, 0x1200, tier="rejected", origins=(origin,)),
        ])
        self.assertIn("invalid-live-reference", report["summary"]["issues"])

    def test_same_address_in_other_image_cannot_supply_owner_or_edges(self):
        report = self.audit(
            [function(), function(0x1100, image="OPEN.EXE")],
            [datum(0x1200, image="OPEN.EXE")],
            [ref(0x1000, 0x1200), ref(0x1004, 0x1300, image="OPEN.EXE")],
        )
        self.assertEqual(len(report["references"]), 1)
        self.assertEqual(report["summary"]["issues"], {"unmodeled-target": 1})

    def test_overlay_reference_into_its_host_is_owned_there_not_traversed(self):
        overlay, host = "FDAT_002.BIN", "ST.EXE"
        img = RetailImage.synthetic(overlay, 0x801EB560, b"\0" * 0x2800)
        root = replace(function(0x801EB7AC, image=overlay))
        callee = function(0x800744C4, image=host, vendor=True)
        table = datum(0x801C0270, 4, source="data_identities.tsv", image=host, storage="bss")
        refs = (ref(0x801EB7AC, 0x800744C4, tier="proven", kind="call", image=overlay),
                ref(0x801EB7B0, 0x801C0270, image=overlay),
                ref(0x801EB7B4, 0x80070000, image=overlay))
        report = audit(overlay, (root,), (), refs, img,
                       host_functions=(callee,), host_extents=(table,))
        self.assertEqual(report["summary"]["host_references"], 3)
        self.assertEqual(report["summary"]["reached_vendor_functions"], 0)
        self.assertEqual(report["summary"]["issues"], {"unmodeled-target": 1})
        self.assertEqual(report["issues"][0]["image"], host)
        owners = {row["id"]: row["target_owners"] for row in report["references"]}
        self.assertEqual(owners[0], ["ST.EXE:function:800744c4"])
        self.assertEqual(owners[1], [f"ST.EXE:{table.key}"])
        self.assertEqual(owners[2], [])
        # Without the host inventories the same edges stay unmodeled.
        bare = audit(overlay, (root,), (), refs, img)
        self.assertEqual(bare["summary"]["issues"], {"unmodeled-target": 3})

    def test_link_label_target_is_a_boundary_not_an_unmodeled_owner(self):
        img = RetailImage.synthetic(IMAGE, 0x1000, b"\0" * 0x1000)
        refs = (ref(0x1000, 0x1300),)
        unlabelled = audit(IMAGE, (function(),), (), refs, img)
        self.assertEqual(unlabelled["summary"]["issues"], {"unmodeled-target": 1})
        labelled = audit(IMAGE, (function(),), (), refs, img, link_labels=frozenset({0x1300}))
        self.assertEqual(labelled["summary"]["issues"], {})
        # label + constant: the word's referent is the label, past any owner.
        offset = (replace(ref(0x1000, 0x1384), referent=DataReferent("_end", 0x1300)),)
        report = audit(IMAGE, (function(),), (), offset, img, link_labels=frozenset({0x1300}))
        self.assertEqual(report["summary"]["issues"], {})

    def test_partial_config_prefix_is_not_promoted_to_source_ownership(self):
        prefix = replace(datum(0x1200, 4, source="data_identities.tsv"),
                         note="referenced prefix; full array capacity unknown")
        report = self.audit([function()], [prefix], [ref(0x1000, 0x1200)])
        self.assertEqual(report["summary"]["config_only_data_ranges"], 1)
        self.assertIn("config-only-data-not-compared", report["summary"]["issues"])
        self.assertEqual(report["ranges"][0]["size"], 4)

    def test_library_bss_identity_is_compared_by_the_current_link_placement(self):
        flag = replace(datum(0x1800, 4, source="data_identities.tsv", storage="bss"),
                       name="StCdIntrFlag")
        img = RetailImage.synthetic(IMAGE, 0x1000, b"\0" * 0x1000)
        refs = (ref(0x1000, 0x1800),)

        def run(linked, game=frozenset()):
            return audit(IMAGE, (function(),), (flag,), refs, img,
                         linked_symbols=linked, game_symbols=game)

        placed = run({"StCdIntrFlag": 0x1800, "after": 0x1804})
        self.assertEqual(placed["summary"]["issues"], {})
        self.assertEqual(placed["summary"]["sdk_bss_link_placed"], 1)
        self.assertEqual([r["comparison"] for r in placed["ranges"] if r["kind"] == "data"],
                         ["sdk-bss-link-placed"])
        gap = {"config-only-data-not-compared": 1}
        # No current link, a link that puts it elsewhere, a game object that
        # defines the name, another symbol inside the extent, or unknown game
        # objects: the range stays an uncompared config-only gap.
        self.assertEqual(run(None)["summary"]["issues"], gap)
        self.assertEqual(run({"StCdIntrFlag": 0x1810})["summary"]["issues"], gap)
        self.assertEqual(run({"StCdIntrFlag": 0x1800}, frozenset({"StCdIntrFlag"}))
                         ["summary"]["issues"], gap)
        self.assertEqual(run({"StCdIntrFlag": 0x1800, "inside": 0x1802})["summary"]["issues"],
                         gap)
        self.assertEqual(run({"StCdIntrFlag": 0x1800}, None)["summary"]["issues"], gap)
        # Initialized config data never qualifies, however the link places it.
        table = replace(flag, storage="load")
        report = audit(IMAGE, (function(),), (table,), refs, img,
                       linked_symbols={"StCdIntrFlag": 0x1800}, game_symbols=frozenset())
        self.assertEqual(report["summary"]["issues"], gap)

    def test_game_object_symbols_need_every_unit_object(self):
        import tempfile
        from pathlib import Path

        from scripts.kf.data_reachability import game_object_symbols
        from scripts.kf.mips_elf import STT_OBJECT, DefinedSymbol, write_mips_elf

        manifest = SimpleNamespace(units=(
            SimpleNamespace(image=IMAGE, object_name="00001000_a.o"),
            SimpleNamespace(image="OTHER.EXE", object_name="00001000_b.o"),
        ))
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            self.assertIsNone(game_object_symbols(IMAGE, manifest, root))
            (root / "game" / "base").mkdir(parents=True)
            (root / "game" / "base" / "00001000_a.o").write_bytes(write_mips_elf(
                b"\0" * 8, "fn", 8, data=b"\0" * 4,
                data_symbols=(DefinedSymbol("table", 0, 4, STT_OBJECT),)))
            self.assertEqual(game_object_symbols(IMAGE, manifest, root), {"fn", "table"})

    def test_source_owner_supersedes_interior_tsv_rows_without_duplication(self):
        report = self.audit([function()], [
            datum(0x1200, 16, source="RODATA"),
            datum(0x1200, 4, source="data.tsv"),
            datum(0x1204, 4, source="data_identities.tsv"),
        ], [ref(0x1000, 0x1204)])
        data = [r for r in report["ranges"] if r["kind"] == "data"]
        self.assertEqual(len(data), 1)
        self.assertEqual(data[0]["size"], 16)
        self.assertEqual(len(data[0]["inventory"]), 2)

    def test_conflicting_source_owners_remain_ambiguous(self):
        report = self.audit([function()], [datum(0x1200, 16), datum(0x1204, 8)], [
            ref(0x1000, 0x1204),
        ])
        self.assertIn("ambiguous-target-owner", report["summary"]["issues"])
        self.assertEqual(report["summary"]["source_owned_data_ranges"], 2)
        self.assertEqual(report["references"][0]["path_tier"], "candidate")

    def test_code_data_overlap_is_reported_instead_of_choosing_by_name(self):
        report = self.audit([function()], [datum(0x1008, 4, source="data.tsv")], [
            ref(0x1000, 0x1008, tier="proven", kind="branch"),
        ])
        self.assertIn("ambiguous-target-owner", report["summary"]["issues"])

    def test_unreferenced_census_overlap_is_not_hidden_by_reference_traversal(self):
        scan = replace(datum(0x1002, 4, source="data.tsv"),
                       confidence="byte-pattern", classification="string", note="R& b")
        report = self.audit([function()], [scan])
        self.assertEqual(report["summary"]["issues"], {"code-data-owner-overlap": 1})
        overlap = report["issues"][0]["overlaps"][0]
        self.assertEqual((overlap["overlap_va"], overlap["overlap_size"]), (0x1002, 4))
        self.assertEqual((overlap["confidence"], overlap["note"]), ("byte-pattern", "R& b"))
        self.assertEqual(report["summary"]["config_only_data_ranges"], 0)
        self.assertEqual(report["references"], [])

    def test_overlaps_keep_all_claims_and_clip_at_function_boundaries(self):
        report = self.audit([function(), function(0x1020)], [
            datum(0x101E, 4, source="data.tsv"), datum(0x1008, source="DATA"),
            datum(0x1008, source="data_identities.tsv"),
        ])
        issues = report["issues"]
        self.assertEqual(len(issues), 2)
        self.assertEqual([len(i["overlaps"]) for i in issues], [3, 1])
        boundary = [d for i in issues for d in i["overlaps"] if d["va"] == 0x101E]
        self.assertEqual([(d["overlap_va"], d["overlap_size"]) for d in boundary],
                         [(0x101E, 2), (0x1020, 2)])

    def test_overlap_scope_excludes_other_images_unreached_vendors_and_body_tail(self):
        root = replace(function(), size=0x40)
        report = self.audit([root, function(0x1100, vendor=True)], [
            datum(0x1020), datum(0x0FFC, 4), datum(0x1108),
            datum(0x1004, image="OPEN.EXE"),
        ])
        self.assertEqual(report["issues"], [])
        reached = self.audit([root, function(0x1100, vendor=True)], [datum(0x1108)], [
            ref(0x1000, 0x1100, kind="call", tier="candidate"),
        ])
        self.assertEqual(reached["summary"]["issues"]["code-data-owner-overlap"], 1)
        vendor = next(r for r in reached["ranges"] if r["va"] == 0x1100)
        self.assertEqual(vendor["reachability"], "candidate")

    def test_fragmented_extent_does_not_invent_an_overlap_inside_unknown_holes(self):
        report = self.audit([function(fragments=2)], [datum(0x1008)])
        self.assertEqual(report["summary"]["issues"], {"fragmented-function": 1})

    def test_unresolved_indirect_call_blocks_closure(self):
        report = self.audit([function()], refs=[
            ref(0x1000, None, kind="indirect-call", tier="proven"),
        ])
        self.assertIn("unresolved-indirect-control", report["summary"]["issues"])

    def test_fragmented_function_does_not_claim_false_contiguous_coverage(self):
        report = self.audit([function(fragments=2)], [datum(0x1200)], [
            ref(0x1004, 0x1200),
        ])
        self.assertIn("fragmented-function", report["summary"]["issues"])
        self.assertEqual(report["summary"]["source_owned_data_ranges"], 0)

    def test_direct_word_access_cannot_fit_into_one_byte_identity(self):
        img = RetailImage.synthetic(IMAGE, 0x1000, (0x8C421200).to_bytes(4, "little"))
        reference = replace(ref(0x1000, 0x1200), paired_site=0x1000)
        report = self.audit([function()], [datum(0x1200, 1)], [reference], img)
        self.assertIn("access-crosses-owner", report["summary"]["issues"])

    def test_unaligned_load_checks_containing_word(self):
        img = RetailImage.synthetic(IMAGE, 0x1000, (0x88421203).to_bytes(4, "little"))
        reference = replace(ref(0x1000, 0x1203), paired_site=0x1000)
        self.assertEqual(access_span(reference, img), (0x1200, 4))

    def test_cli_exposes_reachability_without_consuming_its_flags(self):
        output = io.StringIO()
        with redirect_stdout(output), self.assertRaises(SystemExit) as caught:
            cli_main(["verify", "reachability", "--help"])
        self.assertEqual(caught.exception.code, 0)
        self.assertIn("--image", output.getvalue())
        self.assertIn("--output", output.getvalue())


class DataPointerEvidenceTest(unittest.TestCase):
    def test_reviewed_pointer_uses_delinker_validation_and_candidates_stay_weak(self):
        data = replace(function(0x1000), space="data", kind="pointer", size=4)
        img = RetailImage.synthetic(IMAGE, 0x1000, (0x1200).to_bytes(4, "little"))
        for status, tier in (("reviewed", "validated"), ("candidate", "candidate")):
            with self.subTest(status=status):
                row = {"image": IMAGE, "site_va": "0x1000", "paired_site_va": "",
                       "target_va": "0x1200", "kind": "mips32_candidate",
                       "status": status, "channel": "raw-word", "confidence": "test",
                       "provenance": "test"}
                evidence = object.__new__(Evidence)
                evidence.image, evidence.img, evidence.rows = IMAGE, img, [row]
                evidence.catalog = None
                evidence.idx = SimpleNamespace(function_owner=lambda _: None,
                                               data_owner=lambda _: data)
                result = evidence._relocation_references()[0]
                self.assertEqual(result.tier, tier)
                self.assertIsNone(result.referent)
                if status == "reviewed":
                    decoded = validate_relocation(img.require(0x1000, 4), data,
                                                  row, None, "safe")
                    self.assertEqual((result.site, result.target), (decoded.site, decoded.target))

    def test_reviewed_word_naming_a_link_label_has_the_label_as_referent(self):
        data = replace(function(0x1000), space="data", kind="pointer", size=4)
        img = RetailImage.synthetic(IMAGE, 0x1000, (0x1384).to_bytes(4, "little"))
        row = {"image": IMAGE, "site_va": "0x1000", "paired_site_va": "",
               "target_va": "0x1384", "target_name": "_end", "kind": "mips32_candidate",
               "status": "reviewed", "channel": "data-word", "confidence": "test",
               "provenance": "test"}
        evidence = object.__new__(Evidence)
        evidence.image, evidence.img, evidence.rows = IMAGE, img, [row]
        evidence.catalog = None
        evidence.labels = {"_end": 0x1300}
        evidence.idx = SimpleNamespace(function_owner=lambda _: None,
                                       data_owner=lambda _: data)
        result = evidence._relocation_references()[0]
        self.assertEqual((result.tier, result.target, result.destination),
                         ("validated", 0x1384, 0x1300))
        self.assertEqual(result.as_dict()["addend"], 0x84)


if __name__ == "__main__":
    unittest.main()

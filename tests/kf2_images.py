"""Run tooling-control tests against the King's Field II image table.

The linker, delinker and report controls use King's Field II's four programs
as fixtures. Decorate such a test with `@kf2_images`; the table swap is
process-wide only while that test runs. Tests that check this repository's
own config are never decorated.
"""

from __future__ import annotations

import sys
from pathlib import Path

from scripts.kf import retail

FIXTURE = Path(__file__).resolve().parent / "fixtures" / "kf2_images.toml"
_TABLES = (
    "IMAGE_LAYOUTS", "IMAGE_DISC_FILES", "IMAGE_ORDER", "IMAGE_NAMES",
    "RETAIL_ARCHIVES", "OVERLAY_SOURCES",
)
_saved: dict[str, object] = {}


def _rebind(old: object, new: object) -> None:
    """Point every loaded project module's copy of a tuple at its replacement."""
    for name, module in list(sys.modules.items()):
        if not name.startswith("scripts.") or module is None:
            continue
        for attribute, value in list(vars(module).items()):
            if value is old:
                setattr(module, attribute, new)


def _install(disc, layouts, disc_files, archives, overlays) -> None:
    order = {image: index for index, image in enumerate(layouts)}
    names = {retail.image_key(image): image for image in layouts}
    # Dicts are shared by every `from scripts.kf.retail import ...`; update in place.
    contents = (layouts, disc_files, order, names, archives, overlays)
    for table, content in zip(_TABLES, contents, strict=True):
        target = getattr(retail, table)
        target.clear()
        target.update(content)
    keys = tuple(names)
    _rebind(retail.IMAGE_KEYS, keys)
    _rebind(retail.RETAIL_DISC, disc)


def use_kf2_images() -> None:
    if _saved:
        return
    _saved.update({table: dict(getattr(retail, table)) for table in _TABLES})
    _saved["IMAGE_KEYS"] = retail.IMAGE_KEYS
    _saved["RETAIL_DISC"] = retail.RETAIL_DISC
    _install(*retail._load_images(FIXTURE), *retail._load_overlays(FIXTURE))


def restore_images() -> None:
    if not _saved:
        return
    current_keys, current_disc = retail.IMAGE_KEYS, retail.RETAIL_DISC
    for table in _TABLES:
        target = getattr(retail, table)
        target.clear()
        target.update(_saved[table])
    _rebind(current_keys, _saved["IMAGE_KEYS"])
    _rebind(current_disc, _saved["RETAIL_DISC"])
    _saved.clear()


def kf2_images(test):
    """Decorate one test method so it runs against the King's Field II table."""
    import functools

    @functools.wraps(test)
    def wrapper(*args, **kwargs):
        use_kf2_images()
        try:
            return test(*args, **kwargs)
        finally:
            restore_images()

    return wrapper

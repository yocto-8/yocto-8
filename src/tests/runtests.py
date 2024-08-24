import pytest
import os
from pathlib import Path
from glob import glob

from util import execute_test_rom_y8, execute_test_rom_pico8, compare_p8_y8_outputs

# the root directory for tests is the script path
TEST_ROOT = Path(os.path.dirname(os.path.realpath(__file__)))
TEST_ROMS = list(glob(f"{str(TEST_ROOT)}/roms/**/*.p8", recursive=True))

@pytest.mark.parametrize(
    "rom_fname",
    (TEST_ROOT / "roms" / rom_fname for rom_fname in TEST_ROMS)
)
def test_rom(rom_fname):
    rom_fname = str(rom_fname)
    ground_truth = execute_test_rom_pico8(rom_fname)
    exit_code, output = execute_test_rom_y8(rom_fname)
    assert exit_code == 0, f"ROM {rom_fname} failed with code {exit_code}, output:\n{output}"
    compare_p8_y8_outputs(ground_truth, output)

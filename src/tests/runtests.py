import pytest
import os
from pathlib import Path

from util import execute_test_rom_y8, execute_test_rom_pico8, compare_p8_y8_outputs

# the root directory for tests is the script path
TEST_ROOT = Path(os.path.dirname(os.path.realpath(__file__))) / "roms"
# build a list of all p8 files inside of `tests/roms`
TEST_ROMS = list(str(path.relative_to(TEST_ROOT)) for path in TEST_ROOT.glob("**/*.p8"))

# the ROM name becomes part of the parameterized test name;
# thus you can filter tests for a specific file or directory with pytest's `-k` option, e.g. `-k rng/`
@pytest.mark.parametrize("test_name", TEST_ROMS, ids=TEST_ROMS)
def test_rom(test_name):
    rom_path = TEST_ROOT / test_name
    ground_truth = execute_test_rom_pico8(rom_path)
    exit_code, output = execute_test_rom_y8(rom_path)
    assert exit_code == 0, f"ROM `{test_name}` failed with code {exit_code}, output:\n{output}"
    compare_p8_y8_outputs(ground_truth, output)

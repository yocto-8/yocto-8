import pytest
import os
from pathlib import Path

from util import check_sanitizer_issues, execute_test_rom_y8, execute_test_rom_pico8, compare_p8_y8_outputs

# the root directory for tests is the script path
TEST_ROOT = Path(os.path.dirname(os.path.realpath(__file__))) / "roms"
# build lists of all p8 files inside of `tests/roms`
COMPARE_TEST_ROMS = list(str(path.relative_to(TEST_ROOT)) for path in TEST_ROOT.glob("compare/**/*.p8"))
STANDALONE_TEST_ROMS = list(str(path.relative_to(TEST_ROOT)) for path in TEST_ROOT.glob("standalone/**/*.p8"))

TEST_TIMEOUT = 5

# the ROM name becomes part of the parameterized test name;
# thus you can filter tests for a specific file or directory with pytest's `-k` option, e.g. `-k rng/`
@pytest.mark.parametrize("test_name", COMPARE_TEST_ROMS, ids=COMPARE_TEST_ROMS)
@pytest.mark.timeout(TEST_TIMEOUT)
def test_y8_check_rom_output_against_p8(test_name):
    rom_path = TEST_ROOT / test_name
    ground_truth = execute_test_rom_pico8(rom_path)
    exit_code, output = execute_test_rom_y8(rom_path)
    assert exit_code == 0, f"ROM `{test_name}` failed with code {exit_code}, output:\n{output}"
    compare_p8_y8_outputs(ground_truth, output)
    check_sanitizer_issues(output)

@pytest.mark.parametrize("test_name", STANDALONE_TEST_ROMS, ids=STANDALONE_TEST_ROMS)
@pytest.mark.timeout(TEST_TIMEOUT)
def test_y8_check_rom_asserts(test_name):
    rom_path = TEST_ROOT / test_name
    exit_code, output = execute_test_rom_y8(rom_path)
    assert exit_code == 0, f"ROM `{test_name}` failed with code {exit_code}, output:\n{output}"
    check_sanitizer_issues(output)

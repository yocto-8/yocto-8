import unittest
import testtools
from parameterized import parameterized
import os
from pathlib import Path

from util import execute_test_rom, execute_test_rom_pico8, compare_p8_y8_outputs

# the root directory for tests is the script path
TEST_ROOT = Path(os.path.dirname(os.path.realpath(__file__)))

TEST_ROMS = [
    "basic-assert.p8",
    "lua-if.p8",
    "trig.p8"
]


class TestROMs(unittest.TestCase):
    @parameterized.expand([
        [TEST_ROOT / "roms" / rom_fname] for rom_fname in TEST_ROMS
    ])
    def test_rom(self, rom_fname):
        ground_truth = execute_test_rom_pico8(rom_fname)
        exit_code, output = execute_test_rom(rom_fname)
        assert exit_code == 0, f"ROM {rom_fname} failed with code {exit_code}, output:\n{output}"
        compare_p8_y8_outputs(ground_truth, output)

if __name__ == '__main__':
    suite = unittest.TestLoader().loadTestsFromTestCase(TestROMs)
    concurrent_suite = testtools.ConcurrentStreamTestSuite(lambda: ((case, None) for case in suite))
    concurrent_suite.run(testtools.StreamResult())
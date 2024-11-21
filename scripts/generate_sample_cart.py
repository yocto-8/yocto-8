#!/usr/bin/env python3

# generate a cart with random data in the cartridge. used for testing persing.

import random
import string

random.seed(42)

hex_digits = "0123456789abcdef"


def sample_hex():
    return random.choice(string.hexdigits[:16])


def random_gfx_block():
    ret = ""
    for _row in range(128):
        for _col in range(128):
            ret += sample_hex()
        ret += "\n"
    return ret


def random_gff_block():
    ret = ""
    for _row in range(2):
        for _col in range(256):
            ret += sample_hex()
        ret += "\n"
    return ret


def random_map_block():
    ret = ""
    for _row in range(32):
        for _col in range(256):
            ret += sample_hex()
        ret += "\n"
    return ret


def random_sfx_block():
    ret = ""
    for _row in range(64):
        for _col in range(168):
            ret += sample_hex()
        ret += "\n"
    return ret


def random_music_block():
    ret = ""
    for _row in range(64):
        ret += f"{sample_hex()}{sample_hex()} "
        for _col in range(8):
            ret += sample_hex()
        ret += "\n"
    return ret


print(
    f"""pico-8 cartridge // http://www.pico-8.com
version 32
__lua__
__gfx__
{random_gfx_block()}
__gff__
{random_gff_block()}
__map__
{random_map_block()}
__sfx__
{random_sfx_block()}
__music__
{random_music_block()}
""",
    end="",
)

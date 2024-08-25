pico-8 cartridge // http://www.pico-8.com
version 32
__lua__

printh("check modulo")
printh(10.10 % 2.5)

printh("check %0 edge case in modulo")
printh(69.69 % 0)
printh(-69.69 % 0)

printh("check negative lhs in modulo")
printh(-10.10 % 2.5)

printh("check negative rhs in modulo")
printh(10.10 % -2.5)

printh("check negative both in modulo")
printh(-10.10 % -2.5)
printh(-10.10 % -20.5)

printh("check /0 edge case in division")
printh(10 / 0)
printh(-10 / 0)

printh("check negative lhs in division")
printh(-10.10 / 2.5)

printh("check negative rhs in division")
printh(10.10 / -2.5)

printh("check negative both in division")
printh(-10.10 / -2.5)
printh(-10.10 / -20.5)


printh("====DONE====")
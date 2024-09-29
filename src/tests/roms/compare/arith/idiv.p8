pico-8 cartridge // http://www.pico-8.com
version 32
__lua__

printh("check \0 edge case in intdivision")
printh(10 \ 0)
printh(-10 \ 0)

printh("check negative lhs in intdivision")
printh(-10.10 \ 2.5)

printh("check negative rhs in intdivision")
printh(10.10 \ -2.5)

printh("check negative both in intdivision")
printh(-10.10 \ -2.5)
printh(-10.10 \ -20.5)


printh("====DONE====")
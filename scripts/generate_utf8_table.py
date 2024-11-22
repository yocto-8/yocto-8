#!/usr/bin/env python3

# convert utf-8 sequences into p8scii
# uses list from https://web.archive.org/web/20240217141002/https://gist.github.com/joelsgp/bf930961230731fe370e5c25ba05c5d3

pscii_tbl_enc = [
    "\\0",
    "\\*",
    "\\#",
    "\\-",
    "\\|",
    "\\+",
    "\\^",
    "\\a",
    "\\b",
    "\\t",
    "\\n",
    "\\v",
    "\\f",
    "\\r",
    "\\014",
    "\\015",
    "▮",
    "■",
    "□",
    "⁙",
    "⁘",
    "‖",
    "◀",
    "▶",
    "「",
    "」",
    "¥",
    "•",
    "、",
    "。",
    "゛",
    "゜",
    " ",
    "!",
    '"',
    "#",
    "$",
    "%",
    "&",
    "'",
    "(",
    ")",
    "*",
    "+",
    ",",
    "-",
    ".",
    "/",
    "0",
    "1",
    "2",
    "3",
    "4",
    "5",
    "6",
    "7",
    "8",
    "9",
    ":",
    ";",
    "<",
    "=",
    ">",
    "?",
    "@",
    "A",
    "B",
    "C",
    "D",
    "E",
    "F",
    "G",
    "H",
    "I",
    "J",
    "K",
    "L",
    "M",
    "N",
    "O",
    "P",
    "Q",
    "R",
    "S",
    "T",
    "U",
    "V",
    "W",
    "X",
    "Y",
    "Z",
    "[",
    "\\",
    "]",
    "^",
    "_",
    "`",
    "a",
    "b",
    "c",
    "d",
    "e",
    "f",
    "g",
    "h",
    "i",
    "j",
    "k",
    "l",
    "m",
    "n",
    "o",
    "p",
    "q",
    "r",
    "s",
    "t",
    "u",
    "v",
    "w",
    "x",
    "y",
    "z",
    "{",
    "|",
    "}",
    "~",
    "○",
    "█",
    "▒",
    "🐱",
    "⬇️",
    "░",
    "✽",
    "●",
    "♥",
    "☉",
    "웃",
    "⌂",
    "⬅️",
    "😐",
    "♪",
    "🅾️",
    "◆",
    "…",
    "➡️",
    "★",
    "⧗",
    "⬆️",
    "ˇ",
    "∧",
    "❎",
    "▤",
    "▥",
    "あ",
    "い",
    "う",
    "え",
    "お",
    "か",
    "き",
    "く",
    "け",
    "こ",
    "さ",
    "し",
    "す",
    "せ",
    "そ",
    "た",
    "ち",
    "つ",
    "て",
    "と",
    "な",
    "に",
    "ぬ",
    "ね",
    "の",
    "は",
    "ひ",
    "ふ",
    "へ",
    "ほ",
    "ま",
    "み",
    "む",
    "め",
    "も",
    "や",
    "ゆ",
    "よ",
    "ら",
    "り",
    "る",
    "れ",
    "ろ",
    "わ",
    "を",
    "ん",
    "っ",
    "ゃ",
    "ゅ",
    "ょ",
    "ア",
    "イ",
    "ウ",
    "エ",
    "オ",
    "カ",
    "キ",
    "ク",
    "ケ",
    "コ",
    "サ",
    "シ",
    "ス",
    "セ",
    "ソ",
    "タ",
    "チ",
    "ツ",
    "テ",
    "ト",
    "ナ",
    "ニ",
    "ヌ",
    "ネ",
    "ノ",
    "ハ",
    "ヒ",
    "フ",
    "ヘ",
    "ホ",
    "マ",
    "ミ",
    "ム",
    "メ",
    "モ",
    "ヤ",
    "ユ",
    "ヨ",
    "ラ",
    "リ",
    "ル",
    "レ",
    "ロ",
    "ワ",
    "ヲ",
    "ン",
    "ッ",
    "ャ",
    "ュ",
    "ョ",
    "◜",
    "◝",
]

print_chr_map = {x.encode(): i for i, x in enumerate(pscii_tbl_enc) if i >= 16}
print_chr_map |= {
    x.encode(): i
    for x, i in {
        "¹": 1,
        "²": 2,
        "³": 3,
        "⁴": 4,
        "⁵": 5,
        "⁶": 6,
        "⁷": 7,
        "⁸": 8,
        "ᵇ": 11,
        "ᶜ": 12,
        "ᵉ": 14,
        "ᶠ": 15,
    }.items()
}
print_chr_map[bytes()] = 0
print_chr_map["\t".encode("utf-8")] = 9
print_chr_map["\n".encode("utf-8")] = 10
print_chr_map["\r".encode("utf-8")] = 13

# same map, but also parse the escaped format \\0 etc.
parse_chr_map = print_chr_map
# ... NOT, we need manual handling of escape sequences at the end of the day
# | {
#     x.encode(): i for i, x in enumerate(pscii_tbl_enc) if i < 16
# }

# p8scii -> utf-8 table
# sort utf by lexicographical order so we can binary search thru it
# in this table, each entry looks like [p8scii_value, utf8_str, NUL]
# p8scii_value is omitted if equal to utf8_str, offsets are adjusted accordingly
# both offset tables point towards the utf8_str, not the p8scii_value

p8_to_utf_offsets = [None for _ in range(256)]
sorted_utf_offsets = [None for _ in range(len(parse_chr_map.keys()))]

raw_table = bytes()

for i, (utf8_bytes, p8scii_value) in enumerate(
    sorted(parse_chr_map.items(), key=lambda x: x[0])
):
    # if bytes([p8scii_value]) != utf8_bytes and len(utf8_bytes) > 0:
    raw_table += bytes([p8scii_value])

    sorted_utf_offsets[i] = len(raw_table)
    if utf8_bytes in print_chr_map.keys():
        p8_to_utf_offsets[p8scii_value] = len(raw_table)

    raw_table += utf8_bytes
    raw_table += bytes([0])

assert all(x is not None for x in p8_to_utf_offsets)
assert all(x is not None for x in sorted_utf_offsets)

print(f"// {len(raw_table)} bytes")

table_qualifier = "static constexpr"

print(
    f"{table_qualifier} auto p8scii_table = std::to_array<std::uint8_t>({{{', '.join(str(x) for x in raw_table)}}});"
)

print(
    f"{table_qualifier} auto p8scii_to_utf8_offsets = std::to_array<std::int16_t>({{{', '.join(str(x) for x in p8_to_utf_offsets)}}});"
)

print(
    f"{table_qualifier} auto p8scii_lex_sort_utf8_offsets = std::to_array<std::int16_t>({{{', '.join(str(x) for x in sorted_utf_offsets)}}});"
)

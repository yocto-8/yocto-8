#include "lua.hpp"
#include <array>

#include <emu/bindings/input.hpp>
#include <emu/bindings/math.hpp>
#include <emu/bindings/misc.hpp>
#include <emu/bindings/mmio.hpp>
#include <emu/bindings/rng.hpp>
#include <emu/bindings/table.hpp>
#include <emu/bindings/time.hpp>
#include <emu/bindings/video.hpp>

namespace emu {

using BindingCallback = int(lua_State *);

struct Binding {
	/// @brief Name to export the binding under.
	const char *name;

	/// @brief Reference to the callback. Always of this fixed signature.
	BindingCallback *callback;
};

static constexpr auto y8_std = std::to_array<Binding>({
	{"camera", bindings::y8_camera},
	{"color", bindings::y8_color},
	{"pset", bindings::y8_pset},
	{"pget", bindings::y8_pget},
	{"sset", bindings::y8_sset},
	{"sget", bindings::y8_sget},
	{"fget", bindings::y8_fget},
	{"cls", bindings::y8_cls},
	{"line", bindings::y8_line},
	{"circfill", bindings::y8_circfill},
	{"rectfill", bindings::y8_rectfill},
	{"spr", bindings::y8_spr},
	{"sspr", bindings::y8_sspr},
	{"pal", bindings::y8_pal},
	{"palt", bindings::y8_palt},
	{"fillp", bindings::y8_fillp},
	{"clip", bindings::y8_clip},
	{"mset", bindings::y8_mset},
	{"mget", bindings::y8_mget},
	{"map", bindings::y8_map},
	{"flip", bindings::y8_flip},
	{"print", bindings::y8_print},
	{"_rgbpal", bindings::y8_rgbpal},

	{"btn", bindings::y8_btn},
	{"btnp", bindings::y8_btnp},

	{"peek", bindings::y8_peek},
	{"peek2", bindings::y8_peek2},
	{"peek4", bindings::y8_peek4},
	{"poke", bindings::y8_poke},
	{"poke2", bindings::y8_poke2},
	{"poke4", bindings::y8_poke4},
	{"memcpy", bindings::y8_memcpy},
	{"memset", bindings::y8_memset},
	{"reload", bindings::y8_reload},
	{"load", bindings::y8_load},
	{"ls", bindings::y8_ls},
	{"dir", bindings::y8_ls},

	{"abs", bindings::y8_abs},
	{"flr", bindings::y8_flr},
	{"mid", bindings::y8_mid},
	{"min", bindings::y8_min},
	{"max", bindings::y8_max},
	{"sin", bindings::y8_sin},
	{"cos", bindings::y8_cos},
	{"atan2", bindings::y8_atan2},
	{"sqrt", bindings::y8_sqrt},

	{"shl", bindings::y8_shl},
	{"shr", bindings::y8_shr},
	{"lshr", bindings::y8_lshr},
	{"rotl", bindings::y8_rotl},
	{"rotr", bindings::y8_rotr},
	{"band", bindings::y8_band},
	{"bor", bindings::y8_bor},
	{"sgn", bindings::y8_sgn},

	{"cursor", bindings::y8_cursor},
	{"printh", bindings::y8_printh},
	{"tostr", bindings::y8_tostr},
	{"tonum", bindings::y8_tonum},
	{"stat", bindings::y8_stat},
	{"sub", bindings::y8_sub},
	{"ord", bindings::y8_ord},
	{"_exit", bindings::y8_exit},
	{"_gc", bindings::y8_gc},

	{"rnd", bindings::y8_rnd},
	{"srand", bindings::y8_srand},

	{"t", bindings::y8_time},
	{"time", bindings::y8_time},

	{"add", bindings::y8_add},
	{"del", bindings::y8_del},
	{"foreach", bindings::y8_foreach},
	{"split", bindings::y8_split},
	{"unpack", bindings::y8_unpack},
});

struct NumericGlobal {
	const char8_t *name;
	lua_Number value;
};

// Populated manually from the list of identifiers listed on the pico-8 wiki for
// the header page
// TODO: check if still accurate
static constexpr auto y8_numeric_globals = std::to_array<NumericGlobal>({
	// joystick
	{u8"⬅️", 0},
	{u8"➡️", 1},
	{u8"⬆️", 2},
	{u8"⬇️", 3},
	{u8"🅾️", 4},
	{u8"❎", 5},
	{u8"█", 0x0000'8000_raw_fix16},
	{u8"▒", 0x5A5A'8000_raw_fix16},
	{u8"░", 0x7D7D'8000_raw_fix16},
	{u8"🐱", 0x511F'8000_raw_fix16},
	{u8"▥", 0x5555'8000_raw_fix16},
	{u8"▤", 0x0F0F'8000_raw_fix16},
	{u8"✽", 0xB81D'8000_raw_fix16},
	{u8"●", 0xF99F'8000_raw_fix16},
	{u8"♥", 0x51BF'8000_raw_fix16},
	{u8"☉", 0xB5BF'8000_raw_fix16},
	{u8"웃", 0x999F'8000_raw_fix16},
	{u8"⌂", 0xB11F'8000_raw_fix16},
	{u8"😐", 0xA0E0'8000_raw_fix16},
	{u8"♪", 0x9B3F'8000_raw_fix16},
	{u8"◆", 0xB1BF'8000_raw_fix16},
	{u8"…", 0xF5FF'8000_raw_fix16},
	{u8"★", 0xB15F'8000_raw_fix16},
	{u8"⧗", 0x1B1F'8000_raw_fix16},
	{u8"ˇ", 0xF5BF'8000_raw_fix16},
	{u8"∧", 0x7ADF'8000_raw_fix16},
});

} // namespace emu
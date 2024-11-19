#pragma once

#include "coredefs.hpp"
#include <array>
#include <functional>
#include <stdexcept>
#include <string_view>

#include <hal/hal.hpp>

namespace p8 {

class LookaheadReader {
	public:
	LookaheadReader(hal::ReaderCallback &reader, void *ud)
		: _fs_reader(reader), _fs_ud(ud), _file_buffer{},
		  _file_buffer_offset(0), _backtrack_offset(_backtrack_buffer.size()) {}

	/// Consumes and returns the next character in the stream. '\0' signals EOF.
	char consume_char() {
		if (!_remaining_backtrack().empty()) {
			return _backtrack_buffer[_backtrack_offset++];
		}

		if (_try_ensure_file_buffer()) {
			return _file_buffer[_file_buffer_offset++];
		}

		return '\0';
	}

	/// Does NOT consume input. Looks ahead for one character and returns it.
	/// '\0' signals EOF.
	char peek_char() {
		if (!_remaining_backtrack().empty()) {
			return _backtrack_buffer[_backtrack_offset];
		}

		if (_try_ensure_file_buffer()) {
			return _file_buffer[_file_buffer_offset];
		}

		return '\0';
	}

	/// Consumes characters as long as `predicate(c)` is true.
	/// Character is written to `*output_char` even if `predicate` fails.
	/// The character that fails the `predicate` is NOT consumed.
	bool consume_if(std::invocable<char> auto &&consumer,
	                char *output_char = nullptr) {
		char c = peek_char();
		if (output_char != nullptr) {
			*output_char = c;
		}
		if (consumer(c)) {
			consume_char();
			return true;
		}
		return false;
	}

	/// Consumes characters into the destination span as long as `predicate(c)`
	/// is true. The returned span is the subspan of actually read data.
	std::span<char> consume_into(std::invocable<char> auto &&predicate,
	                             std::span<char> output_buffer) {
		std::size_t i;
		for (i = 0; i < output_buffer.size(); ++i) {
			if (!consume_if(predicate, output_buffer.data() + i)) {
				break;
			}
		}

		if (i == output_buffer.size() - 1 && predicate(peek_char())) {
			throw std::out_of_range(
				"Undersized output buffer; input too long?");
		}

		return output_buffer.subspan(0, i);
	}

	/// Returns whether the next char will be an EOF without consuming it.
	bool is_eof() { return peek_char() == '\0'; }

	/// Consumes input expecting the string to be matched. Returns `true` only
	/// on success. On failure, the first failing character will not be
	/// consumed.
	bool consume_matches(std::string_view to_match) {
		for (std::size_t i = 0; i < to_match.size(); ++i) {
			if (peek_char() != to_match[i]) {
				return false;
			}
			consume_char();
		}
		return true;
	}

	/// Callbacks `consumer(c)` repeatedly with the input sequence, without
	/// consuming it. The amount of consumed bytes should not be larger than the
	/// backtrack buffer.
	void peek_foreach_while(std::invocable<char> auto &&consumer) {
		for (char c : _remaining_backtrack()) {
			if (!consumer(c)) {
				return;
			}
		}

		if (!_try_ensure_file_buffer()) { // eof?
			return;
		}

		for (char c : _remaining_file_buffer()) {
			if (!consumer(c)) {
				return;
			}
		}

		// Filesystem buffer exhausted.
		// Migrate to the backtrack buffer if possible
		while (_remaining_file_buffer().size() < free_backtrack_space()) {
			_migrate_file_buffer_to_backtrack();

			if (!_read_next_fs_buffer()) { // eof?
				return;
			}

			for (char c : _remaining_file_buffer()) {
				if (!consumer(c)) {
					return;
				}
			}

			// Falling through here? Maybe we received a short buffer, and still
			// got space to migrate to the backtrack buffer.
		}

		release_abort("tried to peek beyond backtrack buffer");
	}

	/// Does NOT consume input. Looks ahead for a given match. The given match
	/// must be smaller than the backtrack buffer. Returns `true` only if a
	/// match was found.
	bool peek_matches(std::string_view to_match) {
		std::size_t i = 0;
		bool mismatched = false;
		peek_foreach_while([&](char c) {
			if (c != to_match[i]) {
				mismatched = true;
				return false; // stop iteration
			}

			++i;
			return i < to_match.size(); // stop iteration when all chars checked
		});

		return !mismatched;
	}

	/// Consumes input until either an end-of-line or end-of-file is reached.
	/// Consumes the end-of-line character as well.
	void consume_until_next_line() {
		char c;
		do {
			c = consume_char();
		} while (c != '\n' && c != '\0');
	}

	/// Consumes spaces. Then, if a newline is found, consumes it and returns
	/// `true`. If other non-newline characters are found, returns false.
	bool consume_empty_line() {
		while (consume_if([](char c) { return c == ' ' || c == '\t'; }))
			;

		char c = peek_char();
		// consumed all leading spaces, and encountered a newline?
		// then the line was truly space-only/empty
		if (c == '\r' || c == '\n') {
			consume_until_next_line();
			return true;
		}

		return false;
	}

	/// Consumes as many bytes from the current buffer (either fs buffer or
	/// backtrack buffer) to either exhaust it or make the predicate false. The
	/// number of bytes consumed will be written to `*size` and the pointer to
	/// the start of the consumed buffer will be returned.
	/// That returned pointer will be invalidated as soon as any other call
	/// potentially causing a read is made.
	///
	/// When EOF has been reached and there is no data left to return, this
	/// function will return `nullptr` and write `0` to `*size`.
	const char *
	consume_rest_of_current_buffer_while(std::invocable<char> auto &&predicate,
	                                     std::size_t *size) {
		if (!_remaining_backtrack().empty()) {
			return _consume_from_buffer(predicate, _backtrack_buffer,
			                            _backtrack_offset, size);
		}

		if (_try_ensure_file_buffer()) {
			return _consume_from_buffer(predicate, _file_buffer,
			                            _file_buffer_offset, size);
		}

		*size = 0;
		return nullptr;
	}

	private:
	bool _read_next_fs_buffer() {
		debug_assert(_file_buffer.empty() || _remaining_file_buffer().empty());

		std::size_t fs_buffer_size = 0;
		const char *fs_buffer_start = _fs_reader(_fs_ud, &fs_buffer_size);

		_file_buffer = {fs_buffer_start, fs_buffer_size};
		_file_buffer_offset = 0;

		return !_file_buffer.empty();
	}

	/// If the fs buffer has any data, return true.
	/// Otherwise, try reading the next fs buffer, then returns true if anything
	/// was read.
	bool _try_ensure_file_buffer() {
		if (!_remaining_file_buffer().empty()) {
			return true;
		}
		return _read_next_fs_buffer();
	}

	void _migrate_file_buffer_to_backtrack() {
		// Considering an input like 'abcdefghi'
		// Our backtrack buffer looks like:
		// --------abc
		//         ^ _backtrack_offset
		// If our fs buffer contains 'def'
		// We want to update the backtrack buffer to:
		// -----abcdef
		//         ^ size - _remaining_fs_size
		//      ^ new _backtrack_offset
		// So first move current contents back by the remaining fs buffer
		// size.
		// Then the next fs buffer read would contain 'ghi'
		// We now want to start iterating on the fs buffer.

		const auto to_copy = _remaining_file_buffer();

		// Move backward the current backtrack buffer, i.e. turn it into
		// -----abcabc
		// and shift the backtrack offset backwards
		std::memmove(_backtrack_buffer.data() + _backtrack_offset -
		                 to_copy.size(),
		             _backtrack_buffer.data() + _backtrack_offset,
		             _remaining_backtrack().size());
		_backtrack_offset -= to_copy.size();

		// Copy the existing fs buffer into the end of the backtrack
		// buffer
		std::memcpy(_backtrack_buffer.data() + _backtrack_buffer.size() -
		                to_copy.size(),
		            to_copy.data(), to_copy.size());

		_file_buffer = {};
	}

	const char *_consume_from_buffer(std::invocable<char> auto &&predicate,
	                                 std::span<const char> buf,
	                                 std::size_t &current_offset,
	                                 std::size_t *size) {
		std::size_t consumed = 0;
		for (char c : buf.subspan(current_offset)) {
			if (!predicate(c)) {
				break;
			}
			++consumed;
		}

		const char *ret = buf.data() + current_offset;
		*size = consumed;
		current_offset += consumed;

		return ret;
	}

	std::size_t free_backtrack_space() const { return _backtrack_offset; }

	std::span<const char> _remaining_backtrack() const {
		return std::span{_backtrack_buffer}.subspan(_backtrack_offset);
	}

	std::span<const char> _remaining_file_buffer() const {
		return _file_buffer.subspan(_file_buffer_offset);
	}

	hal::ReaderCallback *_fs_reader;
	void *_fs_ud;

	std::span<const char> _file_buffer;
	std::size_t _file_buffer_offset;

	std::array<char, 64> _backtrack_buffer;
	std::size_t _backtrack_offset;
};

} // namespace p8
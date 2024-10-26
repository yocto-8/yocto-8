#pragma once

#include "coredefs.hpp"
#include <array>
#include <string_view>

#include <hal/hal.hpp>

namespace p8 {

class LookaheadReader {
	public:
	LookaheadReader(hal::ReaderCallback &reader, void *ud)
		: _fs_reader(reader), _fs_ud(ud), _fs_buffer(nullptr),
		  _fs_buffer_offset(0), _fs_buffer_size(0),
		  _backtrack_offset(_backtrack_buffer.size()) {}

	/// Consumes and returns the next character in the stream. '\0' signals EOF.
	char consume_char() {
		// Do we have stuff to read from the backtrack buffer?
		if (_backtrack_offset < _backtrack_buffer.size()) {
			return _backtrack_buffer[_backtrack_offset++];
		}

		// Anything left to read from the fs buffer?
		if (_fs_buffer_offset < _fs_buffer_size) {
			return _fs_buffer[_fs_buffer_offset++];
		}

		// If we can't, trigger a read, and retry after
		_read_next_fs_buffer();

		// Anything left to read from the fs buffer?
		if (_fs_buffer_offset < _fs_buffer_size) {
			return _fs_buffer[_fs_buffer_offset++];
		}

		return '\0';
	}

	/// Does NOT consume input. Looks ahead for one character and returns it.
	/// '\0' signals EOF.
	char peek_char() {
		// Do we have stuff to read from the backtrack buffer?
		if (_backtrack_offset < _backtrack_buffer.size()) {
			return _backtrack_buffer[_backtrack_offset];
		}

		// Anything left to read from the fs buffer?
		if (_fs_buffer_offset < _fs_buffer_size) {
			return _fs_buffer[_fs_buffer_offset];
		}

		// If we can't, trigger a read, and retry after
		_read_next_fs_buffer();

		// Anything left to read from the fs buffer?
		if (_fs_buffer_offset < _fs_buffer_size) {
			return _fs_buffer[_fs_buffer_offset];
		}

		return '\0';
	}

	/// Consumes characters as long as `predicate(c)` is true.
	/// Character is written to `*output_char` even if `predicate` fails.
	/// The character that fails the `predicate` is NOT consumed.
	template <class Func>
	bool consume_if(Func &&predicate, char *output_char = nullptr) {
		char c = peek_char();
		if (output_char != nullptr) {
			*output_char = c;
		}
		if (predicate(c)) {
			consume_char();
			return true;
		}
		return false;
	}

	template <class Func>
	std::span<char> consume_into(Func &&predicate,
	                             std::span<char> output_buffer) {
		// FIXME: proper error handling when exceeding buffer size
		std::size_t i;
		for (i = 0; i < output_buffer.size(); ++i) {
			if (!consume_if(predicate, &output_buffer[i])) {
				break;
			}
		}

		return output_buffer.subspan(0, i);
	}

	/// Returns whether the next char will be an EOF without consuming it.
	bool is_eof() { return peek_char() == '\0'; }

	// Consumes input expecting the string to be matched. Returns `true` only on
	// success. On failure, the first failing character will be consumed.
	bool consume_matches(std::string_view to_match) {
		while (!to_match.empty()) {
			if (consume_char() != to_match.front()) {
				return false;
			}
			to_match = to_match.substr(1);
		}
		return true;
	}

	template <class Func> void peek_foreach_while(Func &&callback) {
		// Iterate over what can be from the current backtrack buffer
		for (std::size_t i = _backtrack_offset; i < _backtrack_buffer.size();
		     ++i) {
			if (!callback(_backtrack_buffer[i])) {
				return;
			}
		}

		if (_fs_buffer_offset == _fs_buffer_size) {
			// Filesystem buffer is exhausted? We can read it in any case.
			_read_next_fs_buffer();

			if (_remaining_fs_buffer_size() == 0) { // eof?
				return;
			}
		}

		// Iterate over what can be from the fs buffer
		for (std::size_t i = _fs_buffer_offset; i < _fs_buffer_size; ++i) {
			if (!callback(_fs_buffer[i])) {
				return;
			}
		}

		// Filesystem buffer exhausted.
		// Migrate to the backtrack buffer if possible
		while (_remaining_fs_buffer_size() < _remaining_backtrack_size()) {
			// Move backward the current backtrack buffer
			std::memmove(_backtrack_buffer.data() + _backtrack_offset -
			                 _remaining_fs_buffer_size(),
			             _backtrack_buffer.data() + _backtrack_offset,
			             _remaining_fs_buffer_size());
			_backtrack_offset -= _remaining_fs_buffer_size();

			// Copy the existing fs buffer into the end of the backtrack buffer
			std::memcpy(_backtrack_buffer.data() + _backtrack_buffer.size() -
			                _remaining_fs_buffer_size(),
			            _fs_buffer + _fs_buffer_offset,
			            _remaining_fs_buffer_size());

			// Now that the fs buffer is backed up, override it with a new chunk
			// (of an unknown, potentially different size, still)
			_read_next_fs_buffer();

			if (_remaining_fs_buffer_size() == 0) { // eof?
				return;
			}

			// Iterate over what can be from the fs buffer
			for (std::size_t i = _fs_buffer_offset; i < _fs_buffer_size; ++i) {
				if (!callback(_fs_buffer[i])) {
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
				return false;
			}

			++i;
			return i < to_match.size(); // iteration condition
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
	template <class Func>
	const char *consume_rest_of_current_buffer_while(Func &&predicate,
	                                                 std::size_t *size) {
		if (_backtrack_offset < _backtrack_buffer.size()) {
			// Consume from the backtrack buffer
			std::size_t i;
			for (i = _backtrack_offset; i < _backtrack_buffer.size(); ++i) {
				if (!predicate(_backtrack_buffer[i])) {
					break;
				}
			}

			const char *ret = _backtrack_buffer.data() + _backtrack_offset;
			*size = i - _backtrack_offset;
			_backtrack_offset = i; // consume these

			return ret;
		}

		if (_fs_buffer_offset == _fs_buffer_size) {
			// Empty filesystem buffer? Try reading a new buffer.
			_read_next_fs_buffer();
		}

		if (_fs_buffer_offset < _fs_buffer_size) {
			// Consume from fs buffer
			std::size_t i;
			for (i = _fs_buffer_offset; i < _fs_buffer_size; ++i) {
				if (!predicate(_fs_buffer[i])) {
					break;
				}
			}

			const char *ret = _fs_buffer + _fs_buffer_offset;
			*size = i - _fs_buffer_offset;
			_fs_buffer_offset = i; // consume these

			return ret;
		}

		*size = 0;
		return nullptr;
	}

	private:
	void _read_next_fs_buffer() {
		debug_assert(_fs_buffer_offset == fs_read_buffer);
		_fs_buffer = _fs_reader(_fs_ud, &_fs_buffer_size);
		_fs_buffer_offset = 0;
	}

	std::size_t _remaining_backtrack_size() { return _backtrack_offset; }

	std::size_t _remaining_fs_buffer_size() {
		return _fs_buffer_size - _fs_buffer_offset;
	}

	hal::ReaderCallback *_fs_reader;
	void *_fs_ud;

	const char *_fs_buffer;
	std::size_t _fs_buffer_offset;
	std::size_t _fs_buffer_size;

	std::array<char, 64> _backtrack_buffer;
	std::size_t _backtrack_offset;
};

} // namespace p8
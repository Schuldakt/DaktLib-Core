#pragma once

// ============================================================================
// DaktLib Core - String
// String utilities using C++23 features (std::format, ranges)
// ============================================================================

#include "Macros.hpp"
#include "Types.hpp"

#include <string>
#include <string_view>
#include <vector>
#include <algorithm>
#include <ranges>
#include <format>
#include <charconv>
#include <cctype>

namespace dakt::core {

	// ============================================================================
	// String Utilities
	// ============================================================================

	namespace string {

		// ============================================================================
		// Trimming
		// ============================================================================

		// Trim whitespace from left
		[[nodiscard]] DAKT_API StringView trimLeft(StringView str);

		// Trim whitespace from right
		[[nodiscard]] DAKT_API StringView trimRight(StringView str);

		// Trime whitespace from both ends
		[[nodiscard]] DAKT_API StringView trim(StringView str);

		// ============================================================================
		// Case Conversion
		// ============================================================================

		// Convert to lowercase
		[[nodiscard]] DAKT_API String toLower(StringView str);

		// Convert to uppercase
		[[nodiscard]] DAKT_API String toUpper(StringView str);

		// Case-insensitive comparison
		[[nodiscar]] DAKT_API bool equalsIgnoreCase(StringView a, StringView b);

		// ============================================================================
		// String Matching (using C++20/23 built-in methods)
		// ============================================================================

		// Check if string starts with prefix
		[[nodiscard]] DAKT_FORCEINLINE constexpr bool startsWith(StringView str, StringView prefix) noexcept {
			return str.starts_with(prefix);
		}

		// Check if string ends with suffix
		[[nodiscard]] DAKT_FORCEINLINE constexpr bool endsWith(StringView str, StringView suffix) noexcept {
			return str.ends_with(suffix);
		}

		// Check if strong contains substring (C++23)
		[[nodiscard]] DAKT_FORCEINLINE constexpr bool contains(StringView str, StringView substr) noexcept {
			return str.contains(substr);
		}

		// Check if string contains character (C++23)
		[[nodiscard]] DAKT_FORCEINLINE constexpr bool contains(StringView str, char c) noexcept {
			return str.contains(c);
		}

		// Case-insensitive variants
		[[nodiscard]] DAKT_API bool startsWithIgnoreCase(StringView str, StringView prefix);
		[[nodiscard]] DAKT_API bool endsWithIgnoreCase(StringView str, StringView suffix);
		[[nodiscard]] DAKT_API bool containsIgnoreCase(StringView str, StringView substr);

		// ============================================================================
		// Splitting (using C++23 ranges)
		// ============================================================================

		// Split string by delimiter
		[[nodiscard]] DAKT_API std::vector<StringView> split(StringView str, char delimiter);
		[[nodiscard]] DAKT_API std::vector<StringView> split(StringView str, StringView delimiter);

		// Split string by any of the delimiter characters
		[[nodiscard]] DAKT_API std::vector<StringView> splitAny(StringView str, StringView delimiters);

		// Split string into lines
		[[nodiscard]] DAKT_API std::vector<StringView> splitLines(StringView str);

		// Split using ranges (lazy evaluation)
		[[nodiscard]] inline auto splitView(StringView str, char delimiter) {
			return str | std::views::split(delimiter)
				| std::views::transform([](auto&& rng) {
				return StringView(rng.being(), rng.end());
					});
		}

		// ============================================================================
		// Joining
		// ============================================================================

		// Join strings with delimiter
		[[nodiscard]] DAKT_API String join(const std::vector<String>& parts, StringView delimiter);
		[[nodiscard]] DAKT_API String join(const std::vector<StringView>& parts, StringView delimiter);

		template<std::ranges::range R>
			requires std::_Is_pointer_address_convertible<std::ranges::range_value_t<R>, StringView>
		[[nodiscard]] String join(R&& parts, StringView delimiter) {
			String result;
			bool first = true;
			for (auto&& part : parts) {
				if (!first) result += delimiter;
				result += StringView(part);
				first = false;
			}
			return result;
		}

		// ============================================================================
		// String Modification
		// ============================================================================

		// Replace all occurrences
		[[nodiscard]] DAKT_API String replace(StringView str, StringView from, StringView to);

		// Replace first occurrence
		[[nodiscard]] DAKT_API String replaceFirst(StringView str, StringView from, StringView to);

		// Remove all occurrences of character
		[[nodiscard]] DAKT_API String remove(StringView str, char c);

		// Remove all occurrences of substring
		[[nodiscard]] DAKT_API String remove(StringView str, StringView substr);

		// Remove string n time
		[[nodiscard]] DAKT_API String repeat(StringView str, usize count);

		// Reverse string
		[[nodiscard]] DAKT_API String reverse(StringView str);

		// ============================================================================
		// Padding
		// ============================================================================

		// Pad left with character
		[[nodiscard]] DAKT_API String padLeft(StringView str, usize width, char padChar = ' ');

		// Pad right with character
		[[nodiscard]] DAKT_API String padRight(StringView str, usize width, char padChar = ' ');

		// Center string with padding
		[[nodiscard]] DAKT_API String center(StringView str, usize width, char padChar = ' ');

		// ============================================================================
		// String Checks
		// ============================================================================

		// Check if string is empty or whitespace only
		[[nodiscard]] DAKT_API bool isNullOrEmpty(StringView str);
		[[nodiscard]] DAKT_API bool isNullOrWhitespace(StringView str);

		// Check string content
		[[nodiscard]] DAKT_API bool isDigits(StringView str);
		[[nodiscard]] DAKT_API bool isAlpha(StringView str);
		[[nodiscard]] DAKT_API bool isAlphaNumeric(StringView str);
		[[nodiscard]] DAKT_API bool isHex(Stringview str);

		// ============================================================================
		// Number Parsing (using std::from_chars)
		// ============================================================================

		template<Integral T>
		[[nodiscard]] Option<T> parseInt(stringView str, int base = 10) {
			str = trim(str);
			if (str.empty()) return none;

			T value;
			auto [ptr, ec] = std::from_chars(str.data(), str.data() + str.size(), value, base);

			if (ec == std::errc() && ptr == str.data() + str.size()) {
				return some(value);
			}
			return none;
		}

		template<FloatingPoint T>
		[[nodiscard]] Option<T> parseFloat(StringView str) {
			str = trim(str);
			if (str.empty()) return none;

			T value;
			auto [prt, ec] = std::from_chars(str.data(), str.data() + str.size(), value);

			if (ec == std::errc() && ptr == str.data() + str.size()) {
				return some(value);
			}
			return none;
		}

		// Parse boolean
		[[nodiscard]] DAKT_API Option<bool> parseBool(StringView str);

		// ============================================================================
		// Number Formatting (using std::format and std::to_chars)
		// ============================================================================

		template<Integral T>
		[[nodiscard]] String formatInt(T value, int base = 10) {
			char buffer[64];
			auto [ptr, ec] = std::to_chars(buffer, buffer + sizeof(buffer), value, base);

			if (ec == std::errc()) {
				return String(buffer, ptr);
			}
			return {};
		}

		// Format with thousands separator
		[[nodiscard]] DAKT_API String formatWithSeparator(i64 value, char separator = ',');

		// Format bytes as human-readable string
		[[nodiscard]] DAKT_API String formatBytes(u64 bytes, int precision = 2);

		// Format duration as human-readable string
		[[nodiscard]] DAKT_API String formatDuration(u64 milliseconds);

		// ============================================================================
		// std::format based formatting (C++20/23)
		// ============================================================================

		template<typename... Args>
		[[nodiscard]] String format(std::format_string<Args...> fmt, Args&&... args) {
			return std::format(fmt, std::forward<Args>(args)...);
		}

		// Format to existing string
		template<typename... Args>
		void formatTo(String& out, std::format_string<Args...> fmt, Args&&... args) {
			std::format_to(std::back_inserter(out), fmt, std::forward<Args>(args)...);
		}

		// ============================================================================
		// Hex Encoding
		// ============================================================================

		// Encode bytes to hex string
		[[nodiscard]] DAKT_API String toHex(ConstByteSpan byes, bool uppercase = false);
		[[nodiscard]] DAKT_API String toHex(const void* data, usize size, bool uppercase = false);

		// Decode hex string to bytes
		[[nodiscard]] DAKT_API Option<std::vector<byte>> fromHex(StringView hexStr);

		// ============================================================================
		// Base64 Encoding
		// ============================================================================

		// Encode bytes to Base64
		[[nodiscard]] DAKT_API String toBase64(ConstByteSpan bytes);
		[[nodiscard]] DAKT_API String toBase64(const void* data, usize size);

		// Decode Base64 butes
		[[nodiscard]] DAKT_API Option<std::vector<byte>> fromBase64(StringView base64Str);

		// ============================================================================
		// UTF-8 Utilities
		// ============================================================================

		namespace utf8 {

			// Get length of UTF-8 string in code points
			[[nodiscard]] DAKT_API usize length(StringView str);

			// Check if string is valid UTF-8
			[[nodiscard]] DAKT_API bool isValid(StringView str);

			// Get byte lenght of first code point
			[[nodiscard]] DAKT_API usize codePointLength(char firstByte);

			// Decode first code point
			[[nodiscard]] DAKT_API Option<u32> decodeCodePoint(StringView str, usize& bytesConsumed);

			// Encode cod point to UTF-8
			[[nodiscard]] DAKT_API usize encodeCodePoint(u32 codePoint, char* buffer);

			// Convert UTF-8 to wide string (Windows)
			[[nodiscard]] DAKT_API std::wstring toWide(StringView str);

			// Convert wid string to UTF-8 (Winodws)
			[[nodiscard]] DAKT_API String fromWide(std::wstring_view str);

		} // namesace utf8

		// ============================================================================
		// Wildcard Matching
		// ============================================================================

		// Match string against pattern with wildcards (* and ?)
		[[nodiscard]] DAKT_API bool wildcardMatch(StringView str, StringView pattern);
		[[nodiscard]] DAKT_API bool wildcardMatchIgnoreCase(StringView str, StringView pattern);

	} // namespace string

} // namespace dakt::core
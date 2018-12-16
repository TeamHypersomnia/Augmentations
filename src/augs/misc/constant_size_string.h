#pragma once
#include <array>
#include <string>

namespace augs {
	template <unsigned const_count>
	class constant_size_string {
		using array_type = std::array<char, const_count>;

		unsigned len = 0;
		array_type arr = {};

	public:
		constant_size_string() = default;

		constant_size_string(const std::string& ss) {
			len = std::min(std::size_t(const_count - 1), ss.size());

			std::memcpy(arr.data(), ss.data(), len);
			arr[len] = 0;
		}

		constant_size_string(const char* s) {
			auto it = arr.begin();

			while (it != arr.end() - 1 && *s) {
				*it++ = *s++;
			}

			*it = 0;
			len = it - arr.begin();
		}

		operator std::string() const {
			return arr.data();
		}

		template <unsigned B>
		bool operator==(const constant_size_string<B>& b) const {
			return len == b.len && !std::memcmp(data(), b.data(), len);
		}

		template <unsigned B>
		bool operator!=(const constant_size_string<B>& b) const {
			return !operator==(b);
		}

		const auto* c_str() const {
			return arr.data();
		}

		auto* data() {
			return arr.data();
		}

		auto* data() const {
			return arr.data();
		}

		auto size() const {
			return len;
		}

		auto begin() {
			return arr.begin();
		}

		auto end() {
			return arr.begin() + size();
		}

		auto begin() const {
			return arr.begin();
		}

		auto end() const {
			return arr.begin() + size();
		}
	};
}
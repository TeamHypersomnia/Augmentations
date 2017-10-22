#pragma once
#include <experimental/filesystem>

namespace augs {
	using path_type = std::experimental::filesystem::path;

#if READWRITE_TRAITS_INCLUDED
#error "I/O traits were included BEFORE I/O overloads, which may cause them to be omitted under some compilers."
#endif

	template <class Archive>
	void read_object(
		Archive& ar,
		path_type& storage
	) {
		std::string str;
		read(ar, str);
		storage = str;
	}

	template <class Archive>
	void write_object(
		Archive& ar,
		const path_type& storage
	) {
		write(ar, storage.string());
	}

	inline auto to_display_path(path_type target_path) {
		auto display_path = target_path.filename();
		display_path += " (";
		display_path += target_path.replace_filename("");
		display_path += ")";
		return display_path;
	}
}

namespace std {
	template <>
	struct hash<augs::path_type> {
		size_t operator()(const augs::path_type& k) const {
			return std::experimental::filesystem::hash_value(k);
		}
	};
}
#if BUILD_UNIT_TESTS
#include <catch.hpp>

#include "augs/filesystem/file.h"
#include "augs/readwrite/memory_stream.h"
#include "augs/readwrite/byte_readwrite.h"
#include "augs/math/vec2.h"
#include "augs/math/transform.h"
#include "augs/math/camera_cone.h"

static const auto path = GENERATED_FILES_DIR "test_byte_readwrite.bin";

template <class T>
static void report(const T& v, const T& reloaded) {
	if constexpr(can_stream_left_v<std::ostringstream, T>) {
		LOG("Original %x\nReloaded: %x", v, reloaded);
	}
}

template <class T>
static bool try_to_reload_with_file(const T& v) {
	augs::save_as_bytes(v, path);
	T test;
	augs::load_from_bytes(test, path);

	augs::remove_file(path);

	const auto success = test == v;

	if (!success) {
		report(v, test);
	}

	return success;
}

template <class T>
static bool try_to_reload_with_bytes(const T& v) {
	{
		std::vector<std::byte> bytes;
		bytes = augs::to_bytes(v);

		augs::save_as_bytes(bytes, path);
	}

	T test;
	augs::load_from_bytes(test, path);

	augs::remove_file(path);
	
	const auto success = test == v;

	if (!success) {
		report(v, test);
	}

	return success;
}

template <class T>
static bool try_to_reload_with_memory_stream(const T& v) {
	{
		std::vector<std::byte> bytes;
		bytes = augs::to_bytes(v);

		augs::save_as_bytes(bytes, path);
	}

	T test;
	auto bytes = augs::file_to_bytes(path);
	augs::memory_stream ss = std::move(bytes);
	augs::read_bytes(ss, test);

	augs::remove_file(path);

	const auto success = test == v;

	if (!success) {
		report(v, test);
	}

	return success;
}

#define test_cycle(variable) \
REQUIRE(try_to_reload_with_file(variable)); \
REQUIRE(try_to_reload_with_bytes(variable)); \
REQUIRE(try_to_reload_with_memory_stream(variable));

TEST_CASE("Byte readwrite Sanity check") {
	using T = std::variant<double, std::string, std::unordered_map<int, double>, std::optional<std::string>>;

	T a, b;
	REQUIRE(a == b);

	a = 2.0;
	b = 2.0;

	REQUIRE(a == b);
	
	REQUIRE(std::optional<std::string>() == std::optional<std::string>());

	a = std::optional<std::string>();
	b = std::optional<std::string>();

	REQUIRE(a == b);
	
	REQUIRE((std::unordered_map<int, double>() == std::unordered_map<int, double>()));

	std::unordered_map<int, double> ma;
	ma[2] = 4587.0;
	ma[3] = 458247.0;
	ma[22] = 45187.0;
	std::unordered_map<int, double> mb;
	mb[2] = 4587.0;
	mb[3] = 458247.0;
	mb[22] = 45187.0;

	REQUIRE(ma == mb);
}

TEST_CASE("Byte readwrite Trivial types") {
	int a = 2;
	double b = 512.0;
	float C = 432.f;
	
	test_cycle(a);
	test_cycle(b);
	test_cycle(C);
}

TEST_CASE("Byte readwrite Classes") {
	vec2 ab;
	transform tr;
	std::vector<transform> v;
	v.resize(3);

	test_cycle(ab);
	test_cycle(tr);
	test_cycle(v);
}

TEST_CASE("Byte readwrite Arrays") {
	std::array<float, 48> some;
	std::array<std::array<float, 12>, 12> some_more;
	std::array<std::array<std::array<std::array<std::array<float, 2>, 2>, 2>, 2>, 2> even_more;

	test_cycle(some);
	test_cycle(some_more);
	test_cycle(even_more);
}

TEST_CASE("Byte readwrite Containers") {
	std::vector<float> abc;
	std::vector<int> abcd;
	std::vector<std::vector<int>> abcde;
	std::vector<std::unordered_map<int, std::vector<int>>> abcdef;
	abc.resize(2);
	abcd.resize(2);
	abcde.resize(2);
	abcdef.resize(2);
	abcdef[0][412] = { 2, 3, 4 };
	abcdef[1][420] = { 997, 1 };

	test_cycle(abc);
	test_cycle(abcd);
	test_cycle(abcde);
	test_cycle(abcdef);
}

TEST_CASE("Byte readwrite Optionals") {
	std::optional<std::vector<float>> abc{ {} };
	std::optional<std::vector<int>> abcd{ {} };
	std::optional<std::vector<std::vector<int>>> abcde{ {} };
	std::vector<std::unordered_map<int, std::optional<std::vector<int>>>> abcdef;
	abc->resize(2);
	abcd->resize(2);
	abcde->resize(2);
	abcdef.resize(2);
	abcdef[0][412] = { { 2, 3, 4 } };
	abcdef[1][420] = { { 997, 1 } };

	test_cycle(abc);
	test_cycle(abcd);
	test_cycle(abcde);
	test_cycle(abcdef);
}

TEST_CASE("Byte readwrite Variants and optionals") {
	using T = std::variant<double, std::string, std::unordered_map<int, double>, std::optional<std::string>>;

	T v;

	{
		v = 2.0;
		test_cycle(v);
	}

	{
		std::unordered_map<int, double> mm;
		mm[4] = 2.0;
		mm[4287] = 455.2;
		mm[16445] = 4.0;
		v = mm;
		test_cycle(v);
	}


	{
		std::optional<std::string> mm = "Hello world!";
		v = mm;
		test_cycle(v);
	}

	{
		std::string mm = "Hello world!";
		v = mm;
		test_cycle(v);
	}
}
#endif
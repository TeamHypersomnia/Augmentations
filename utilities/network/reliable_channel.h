#pragma once
#include <vector>
#include <unordered_map>
#include <memory>

namespace augs {
	namespace network {
		struct bitstream;

		struct reliable_sender {
			struct message {
				luabind::object script;
				bool flag_for_deletion = false;

				bitstream* output_bitstream = nullptr;
			};

			std::vector<message> reliable_buf;
			bitstream* unreliable_buf = nullptr;

			std::unordered_map<unsigned, unsigned> sequence_to_reliable_range;

			unsigned short unreliable_only_sequence = 0u;
			unsigned short sequence = 0u;
			unsigned short ack_sequence = 0u;

			void post_message(message&);
			bool write_data(bitstream& output);
			void read_ack(bitstream& input);
		};

		struct reliable_receiver {
			unsigned short last_sequence = 0u;
			unsigned short last_unreliable_only_sequence = 0u;

			enum result {
				RELIABLE_RECEIVED,
				ONLY_UNRELIABLE_RECEIVED,
				NOTHING_RECEIVED
			};

			/* returns result enum */
			int read_sequence(bitstream& input);
			void write_ack(bitstream& input);
		};
	}
}
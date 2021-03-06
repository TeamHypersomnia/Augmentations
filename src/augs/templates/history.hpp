#pragma once
#include "augs/templates/history.h"
#include "augs/templates/container_templates.h"

namespace augs {
	template <class D, class... C>
	void history<D, C...>::discard_later_revisions() {
		erase_from_to(commands, current_revision + 1);

		derived_set_modified_flag();

		auto& self = *static_cast<D*>(this);
		self.invalidate_revisions_from(current_revision + 1);
	}

	template <class D, class... C>
	template <class T, class... RedoArgs>
	const T& history<D, C...>::execute_new(T&& command, RedoArgs&&... redo_args) {
		/* 
			Remove all redoable entries past the current revision.
			Later we might support branches. 
		*/

		discard_later_revisions();

		commands.emplace_back(
			std::in_place_type_t<remove_cref<T>>(),
			std::forward<T>(command)
		); 

		redo(std::forward<RedoArgs>(redo_args)...);

		set_last_op(history_op_type::EXECUTE_NEW);

		return std::get<remove_cref<T>>(last_command());
	}

	template <class D, class... C>
	template <class... Args>
	bool history<D, C...>::redo(Args&&... args) {
		if (is_revision_newest()) {
			return false;
		}

		derived_set_modified_flag();

		std::visit(
			[&](auto& command) {
				command.redo(std::forward<Args>(args)...); 
			},
			next_command()
		);

		++current_revision;
		set_last_op(history_op_type::REDO);

		return true;
	}

	template <class D, class... C>
	template <class... Args>
	bool history<D, C...>::undo(Args&&... args) {
		if (is_revision_oldest()) {
			return false;
		}

		derived_set_modified_flag();

		std::visit(
			[&](auto& command) {
				command.undo(std::forward<Args>(args)...); 
			},
			last_command()
		);

		--current_revision;
		set_last_op(history_op_type::UNDO);

		return true;
	}

	template <class D, class... C>
	template <class... Args>
	void history<D, C...>::seek_to_revision(const index_type n, Args&&... args) {
		if (n != current_revision) {
			set_last_op(history_op_type::SEEK);
		}

		while (current_revision < n) {
			redo(std::forward<Args>(args)...);
		}

		while (current_revision > n) {
			undo(std::forward<Args>(args)...);
		}
	}
}

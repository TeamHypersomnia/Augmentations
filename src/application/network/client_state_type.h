#pragma once

enum class client_state_type {
	INITIATING_CONNECTION,

	PENDING_WELCOME,
	WELCOME_ARRIVED,
	RECEIVING_INITIAL_STATE,
	RECEIVING_INITIAL_STATE_CORRECTION,
	IN_GAME
};


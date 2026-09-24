/**
 * @file edax_wrapper.c
 *
 * Python bindings wrapper for Edax engine.
 * Provides a simple C API for calling Edax from Python via ctypes.
 *
 * @date 2025
 * @version 1.0
 */

#include "edax_wrapper.h"
#include "board.h"
#include "play.h"
#include "search.h"
#include "book.h"
#include "eval.h"
#include "stats.h"
#include "const.h"
#include "options.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>

/**
 * Global initialization flag to ensure one-time setup
 */
static bool g_edax_initialized = false;

/**
 * Stub functions for symbols needed by options.c and ui.c
 * These are not used in the library mode but are referenced
 */
void version(void) {
	// Stub - not used in library mode
}

void usage(void) {
	// Stub - not used in library mode
}

/**
 * Engine structure for Python bindings
 */
struct EdaxEngine {
	Play play;
	Book book;
	int level;
};

/**
 * Perform one-time global initialization of Edax subsystems.
 * This must be called before creating any engine instances.
 */
static void edax_global_init(void) {
	// Only do truly global init once
	static bool tables_initialized = false;
	if (!tables_initialized) {
		edge_stability_init();
		statistics_init();
		tables_initialized = true;
	}

	// Load eval weights only once
	static bool eval_loaded = false;
	if (!eval_loaded) {
		const char *eval_paths[] = {
			"data/eval.dat",                      // Running from bin/
			"../data/eval.dat",                   // Running from bin/ to data/
			"../edax-reversi/bin/data/eval.dat",  // From ReversiSB3/
			"../bin/data/eval.dat",               // From src/
			"/Users/bill/src/edax-reversi/bin/data/eval.dat",  // Absolute fallback
			NULL
		};

		for (int i = 0; eval_paths[i] != NULL; i++) {
			FILE *f = fopen(eval_paths[i], "rb");
			if (f) {
				fclose(f);
				eval_open(eval_paths[i]);
				eval_loaded = true;
				break;
			}
		}

		if (!eval_loaded) {
			fprintf(stderr, "edax_wrapper: Warning - eval.dat not found, evaluation may be limited\n");
		}
	}

	// Call search_global_init every time - might be needed per engine
	// NOTE: This might be the issue - testing if it helps
	search_global_init();
}

/**
 * Initialize Edax engine with given search depth.
 */
EdaxEngine* edax_create(int level) {
	EdaxEngine *engine;

	// Ensure global init
	edax_global_init();

	// Validate level
	if (level < 1 || level > 60) {
		fprintf(stderr, "edax_create: invalid level %d (must be 1-60)\n", level);
		return NULL;
	}

	// Allocate engine
	engine = (EdaxEngine*)malloc(sizeof(EdaxEngine));
	if (!engine) {
		fprintf(stderr, "edax_create: malloc failed\n");
		return NULL;
	}

	// Initialize book (can be empty/unused)
	book_init(&engine->book);

	// Initialize play structure
	play_init(&engine->play, &engine->book);

	// Set search level
	engine->level = level;
	engine->play.level = level;
	search_set_level(&engine->play.search, level, 0);

	// Completely disable all output
	engine->play.search.options.verbosity = 0;  // No verbose output
	engine->play.search.observer = NULL;        // No observer callback

	// Set global options to be completely quiet
	extern Options options;
	options.verbosity = 0;   // No general output
	options.noise = 100;     // Never print search info (requires depth >= 100)

	return engine;
}

/**
 * Get best move for given board position.
 */
int edax_get_move(EdaxEngine *engine, uint64_t player_bits, uint64_t opponent_bits) {
	if (!engine) {
		return -1;
	}

	// Set board position
	engine->play.board.player = player_bits;
	engine->play.board.opponent = opponent_bits;

	// Set search board
	search_set_board(&engine->play.search, &engine->play.board, BLACK);

	// Check if there are legal moves
	if (!can_move(player_bits, opponent_bits)) {
		return -1;
	}

	// CRITICAL FIX: Don't use play_go() because it calls search_set_level()
	// which uses the LEVEL table to map (level, n_empties) -> (depth, selectivity)
	// Instead, manually set up the search and run it directly

	Search *search = &engine->play.search;

	// Set the exact depth we want (bypass LEVEL table completely)
	search->depth = engine->level;
	search->selectivity = 0;  // No selectivity (full search)

	// Also set in options struct (some search functions check this)
	search->options.depth = engine->level;
	search->options.selectivity = 0;

	// Disable all output
	search->options.verbosity = 0;
	search->observer = NULL;

	// Run the search directly
	search_run(search);

	// Copy result from search to play
	engine->play.result = *search->result;

	// Return best move (0-63, or PASS/NOMOVE if no move)
	if (engine->play.result.move == PASS || engine->play.result.move == NOMOVE) {
		return -1;
	}

	return engine->play.result.move;
}

/**
 * Get evaluation score for last position searched.
 */
int edax_get_score(EdaxEngine *engine) {
	if (!engine) {
		return 0;
	}
	return engine->play.result.score;
}

/**
 * Get number of nodes searched in last search.
 */
uint64_t edax_get_nodes(EdaxEngine *engine) {
	if (!engine) {
		return 0;
	}
	return engine->play.result.n_nodes;
}

/**
 * Set search depth/level for the engine.
 *
 * Note: This only updates the engine->level field. The actual search_set_level()
 * call happens in edax_get_move() with the correct n_empties from the board position.
 *
 * IMPORTANT: When level changes, we must reinitialize the play structure to clear
 * state from previous searches. This prevents crashes when reusing the engine.
 */
void edax_set_level(EdaxEngine *engine, int level) {
	if (!engine) {
		return;
	}
	if (level < 1 || level > 60) {
		fprintf(stderr, "edax_set_level: invalid level %d (must be 1-60)\n", level);
		return;
	}

	// If level is changing, reinitialize the play structure to clear state
	if (engine->level != level) {
		play_free(&engine->play);
		play_init(&engine->play, &engine->book);
	}

	engine->level = level;
	engine->play.level = level;
}

/**
 * Destroy engine and free resources.
 */
void edax_destroy(EdaxEngine *engine) {
	if (engine) {
		play_free(&engine->play);
		book_free(&engine->book);
		free(engine);
	}
}

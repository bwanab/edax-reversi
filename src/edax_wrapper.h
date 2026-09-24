/**
 * @file edax_wrapper.h
 *
 * Python bindings wrapper for Edax engine.
 * Provides a simple C API for calling Edax from Python via ctypes.
 *
 * @date 2025
 * @version 1.0
 */

#ifndef EDAX_WRAPPER_H
#define EDAX_WRAPPER_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Opaque engine handle for Python.
 */
typedef struct EdaxEngine EdaxEngine;

/**
 * Initialize Edax engine with given search depth.
 *
 * @param level Search depth/level (1-60, typical range 4-20)
 * @return Engine handle (NULL on failure)
 */
EdaxEngine* edax_create(int level);

/**
 * Get best move for given board position.
 *
 * Board is represented as two bitboards (64-bit integers):
 * - player_bits: Bitboard of current player's pieces (1 << position)
 * - opponent_bits: Bitboard of opponent's pieces
 *
 * Position mapping: row * 8 + col (0-63)
 * Example: a1=0, h1=7, a8=56, h8=63
 *
 * @param engine Engine handle
 * @param player_bits Bitboard of current player's pieces
 * @param opponent_bits Bitboard of opponent's pieces
 * @return Best move (0-63), or -1 if no legal moves
 */
int edax_get_move(EdaxEngine *engine, uint64_t player_bits, uint64_t opponent_bits);

/**
 * Get evaluation score for last position searched.
 *
 * @param engine Engine handle
 * @return Evaluation score (in centidisks, positive = favorable for current player)
 */
int edax_get_score(EdaxEngine *engine);

/**
 * Get number of nodes searched in last search.
 *
 * @param engine Engine handle
 * @return Node count
 */
uint64_t edax_get_nodes(EdaxEngine *engine);

/**
 * Set search depth/level for the engine.
 *
 * @param engine Engine handle
 * @param level Search depth/level (1-60)
 */
void edax_set_level(EdaxEngine *engine, int level);

/**
 * Destroy engine and free resources.
 *
 * @param engine Engine handle
 */
void edax_destroy(EdaxEngine *engine);

#ifdef __cplusplus
}
#endif

#endif // EDAX_WRAPPER_H

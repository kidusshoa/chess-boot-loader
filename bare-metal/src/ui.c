#include "ui.h"

#include "assets.h"
#include "chess.h"
#include "font.h"
#include "gfx.h"
#include "input.h"

#define COLOR_BG 0xFF1A1A2E
#define COLOR_LIGHT_SQ 0xFFEEEED2
#define COLOR_DARK_SQ 0xFF769656
#define COLOR_SELECT 0xFFBACA44
#define COLOR_MOVE 0xFF829769
#define COLOR_CURSOR 0xFFFFD166
#define COLOR_WHITE_RING 0xFFF5F5F5
#define COLOR_BLACK_RING 0xFF222222
#define COLOR_STATUS 0xFFFFFFFF
#define COLOR_STATUS_DIM 0xFFCCCCCC

#define MAX_LEGAL_MOVES 64

typedef struct {
    int board_x;
    int board_y;
    int square_size;
    int cursor_file;
    int cursor_rank;
    int selected_file;
    int selected_rank;
    bool has_selection;
    color_t turn;
    chess_board_t board;
    square_t legal_moves[MAX_LEGAL_MOVES];
    int legal_move_count;
    bool game_over;
    const char* status_message;
} ui_state_t;

static bool square_has_legal_move(const ui_state_t* state, int file, int rank) {
    for (int i = 0; i < state->legal_move_count; ++i) {
        if (state->legal_moves[i].file == file && state->legal_moves[i].rank == rank) {
            return true;
        }
    }
    return false;
}

static void ui_update_status(ui_state_t* state) {
    if (chess_is_checkmate(&state->board, state->turn)) {
        state->game_over = true;
        state->status_message = "Checkmate! Press R to restart.";
        return;
    }

    if (chess_is_stalemate(&state->board, state->turn)) {
        state->game_over = true;
        state->status_message = "Stalemate! Press R to restart.";
        return;
    }

    state->game_over = false;

    if (chess_is_in_check(&state->board, state->turn)) {
        state->status_message = (state->turn == COLOR_WHITE) ? "White to move - CHECK!" : "Black to move - CHECK!";
        return;
    }

    state->status_message = (state->turn == COLOR_WHITE) ? "White to move" : "Black to move";
}

static void ui_refresh_legal_moves(ui_state_t* state) {
    state->legal_move_count = 0;

    if (!state->has_selection) {
        return;
    }

    state->legal_move_count = chess_legal_moves_from(
        &state->board,
        state->selected_file,
        state->selected_rank,
        state->legal_moves,
        MAX_LEGAL_MOVES
    );
}

static void ui_clear_selection(ui_state_t* state) {
    state->has_selection = false;
    state->selected_file = -1;
    state->selected_rank = -1;
    state->legal_move_count = 0;
}

static void ui_reset(ui_state_t* state) {
    chess_board_reset(&state->board);
    state->turn = COLOR_WHITE;
    state->cursor_file = 4;
    state->cursor_rank = 1;
    ui_clear_selection(state);
    ui_update_status(state);
}

static int ui_square_screen_x(const ui_state_t* state, int file) {
    return state->board_x + file * state->square_size;
}

static int ui_square_screen_y(const ui_state_t* state, int rank) {
    return state->board_y + (7 - rank) * state->square_size;
}

static void ui_draw_piece(const ui_state_t* state, int file, int rank, const piece_t* piece) {
    const int square_x = ui_square_screen_x(state, file);
    const int square_y = ui_square_screen_y(state, rank);
    const int center_x = square_x + state->square_size / 2;
    const int center_y = square_y + state->square_size / 2;
    const int radius = state->square_size / 2 - 8;

    const uint32_t ring_color = piece->color == COLOR_WHITE ? COLOR_WHITE_RING : COLOR_BLACK_RING;
    gfx_fill_circle(center_x, center_y, radius, ring_color);
    gfx_draw_rect(square_x + 4, square_y + 4, state->square_size - 8, state->square_size - 8, 0xFF000000);

    const bitmap_t* sprite = assets_piece_bitmap(piece->type);
    if (sprite && sprite->pixels) {
        const int sprite_x = center_x - (int)sprite->width / 2;
        const int sprite_y = center_y - (int)sprite->height / 2;
        gfx_blit_rgba(sprite_x, sprite_y, sprite);
        return;
    }

    const char* label = chess_piece_label(piece->type);
    int label_len = 0;
    while (label[label_len] != '\0') {
        ++label_len;
    }

    const int text_x = center_x - (label_len * font_char_width()) / 2;
    const int text_y = center_y - font_char_height() / 2;
    font_draw_string(text_x, text_y, label, 0xFF111111);
}

static void ui_draw(ui_state_t* state, struct framebuffer* fb) {
    (void)fb;

    gfx_clear(COLOR_BG);

    for (int rank = 0; rank < 8; ++rank) {
        for (int file = 0; file < 8; ++file) {
            const int x = ui_square_screen_x(state, file);
            const int y = ui_square_screen_y(state, rank);
            const bool light = (file + rank) % 2 == 0;
            uint32_t color = light ? COLOR_LIGHT_SQ : COLOR_DARK_SQ;

            if (state->has_selection && state->selected_file == file && state->selected_rank == rank) {
                color = COLOR_SELECT;
            } else if (square_has_legal_move(state, file, rank)) {
                color = COLOR_MOVE;
            }

            gfx_fill_rect(x, y, state->square_size, state->square_size, color);
        }
    }

    const int cursor_x = ui_square_screen_x(state, state->cursor_file);
    const int cursor_y = ui_square_screen_y(state, state->cursor_rank);
    gfx_draw_rect(cursor_x + 2, cursor_y + 2, state->square_size - 4, state->square_size - 4, COLOR_CURSOR);

    for (int rank = 0; rank < 8; ++rank) {
        for (int file = 0; file < 8; ++file) {
            const piece_t* piece = chess_at(&state->board, file, rank);
            if (piece->type != PIECE_NONE) {
                ui_draw_piece(state, file, rank, piece);
            }
        }
    }

    const int status_y = state->board_y + state->square_size * 8 + 24;
    font_draw_string(state->board_x, status_y, state->status_message, COLOR_STATUS);
    font_draw_string(
        state->board_x,
        status_y + 16,
        "Arrows: move  Space/Enter: select  R: restart",
        COLOR_STATUS_DIM
    );
}

static void ui_try_select_or_move(ui_state_t* state) {
    if (state->game_over) {
        return;
    }

    const piece_t* cursor_piece = chess_at(&state->board, state->cursor_file, state->cursor_rank);

    if (!state->has_selection) {
        if (cursor_piece->type != PIECE_NONE && cursor_piece->color == state->turn) {
            state->has_selection = true;
            state->selected_file = state->cursor_file;
            state->selected_rank = state->cursor_rank;
            ui_refresh_legal_moves(state);
        }
        return;
    }

    if (state->cursor_file == state->selected_file && state->cursor_rank == state->selected_rank) {
        ui_clear_selection(state);
        return;
    }

    if (chess_is_legal_move(
            &state->board,
            state->selected_file,
            state->selected_rank,
            state->cursor_file,
            state->cursor_rank)) {
        chess_move_piece(
            &state->board,
            state->selected_file,
            state->selected_rank,
            state->cursor_file,
            state->cursor_rank
        );
        state->turn = state->turn == COLOR_WHITE ? COLOR_BLACK : COLOR_WHITE;
        ui_clear_selection(state);
        ui_update_status(state);
        return;
    }

    if (cursor_piece->type != PIECE_NONE && cursor_piece->color == state->turn) {
        state->selected_file = state->cursor_file;
        state->selected_rank = state->cursor_rank;
        ui_refresh_legal_moves(state);
        return;
    }

    ui_clear_selection(state);
}

static void ui_handle_key(ui_state_t* state, key_t key) {
    if (key == KEY_RESTART) {
        ui_reset(state);
        return;
    }

    if (state->game_over) {
        return;
    }

    switch (key) {
        case KEY_UP:
            if (state->cursor_rank < 7) {
                ++state->cursor_rank;
            }
            break;
        case KEY_DOWN:
            if (state->cursor_rank > 0) {
                --state->cursor_rank;
            }
            break;
        case KEY_LEFT:
            if (state->cursor_file > 0) {
                --state->cursor_file;
            }
            break;
        case KEY_RIGHT:
            if (state->cursor_file < 7) {
                ++state->cursor_file;
            }
            break;
        case KEY_SELECT:
            ui_try_select_or_move(state);
            break;
        default:
            break;
    }
}

void ui_run(struct framebuffer* fb) {
    ui_state_t state;

    int board_pixels = 640;
    if ((int)fb->width - 40 < board_pixels) {
        board_pixels = (int)fb->width - 40;
    }
    if ((int)fb->height - 80 < board_pixels) {
        board_pixels = (int)fb->height - 80;
    }
    if (board_pixels < 320) {
        board_pixels = 320;
    }
    board_pixels = (board_pixels / 8) * 8;

    const int margin_x = ((int)fb->width - board_pixels) / 2;
    const int margin_y = ((int)fb->height - board_pixels - 48) / 2;

    state.board_x = margin_x > 0 ? margin_x : 0;
    state.board_y = margin_y > 0 ? margin_y : 0;
    state.square_size = board_pixels / 8;
    state.cursor_file = 4;
    state.cursor_rank = 1;
    state.has_selection = false;
    state.selected_file = -1;
    state.selected_rank = -1;
    state.turn = COLOR_WHITE;
    state.legal_move_count = 0;
    state.game_over = false;
    state.status_message = "White to move";

    gfx_bind(fb);
    chess_board_reset(&state.board);
    ui_update_status(&state);
    ui_draw(&state, fb);
    keyboard_init();

    for (;;) {
        const key_t key = keyboard_poll();
        if (key != KEY_NONE) {
            ui_handle_key(&state, key);
            ui_draw(&state, fb);
        }

        __asm__ volatile("pause");
    }
}

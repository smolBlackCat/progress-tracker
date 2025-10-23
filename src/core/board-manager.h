#pragma once

#include <sigc++/signal.h>

#include <string>
#include <vector>

#include "board.h"

/**
 * @brief Progress board in the user's filesystem
 */
struct LocalBoard {
    std::string filename;
    std::shared_ptr<Board> board;
    bool is_open;
};

/**
 * @brief Helper responsible for loading and saving different Boards across the
 * user's system
 */
class BoardManager {
public:
    BoardManager();
    BoardManager(const std::string& board_dir);

    /**
     * @brief Opens local Progress board
     */
    std::shared_ptr<Board> local_open(const std::string& filename);

    /**
     * @brief Returns a list of all local boards
     */
    const std::vector<LocalBoard>& local_boards() const;

    /**
     * @brief Creates a new local Progress board file.
     *
     * @param name Board's title
     * @param background Board's background
     *
     * @return std::string The filename of the newly created board
     */
    std::string local_add(const std::string& name,
                          const std::string& background);

    /**
     * @brief Removes board from local database
     */
    void local_remove(const std::shared_ptr<Board>& board);

    /**
     * @brief Saves and board into local database
     */
    void local_save(const std::shared_ptr<Board>& board);

    /**
     * @brief Closes local board
     */
    void local_close(const std::shared_ptr<Board>& board);

    /**
     * @brief Returns whether the boards can be safely modified
     */
    bool loaded() const;

    sigc::signal<void(LocalBoard)>& signal_add_board();
    sigc::signal<void(LocalBoard)>& signal_remove_board();
    sigc::signal<void(LocalBoard)>& signal_save_board();

protected:
    const std::string BOARD_DIR;
    std::vector<LocalBoard> m_local_boards;

    sigc::signal<void(LocalBoard)> add_board_signal;
    sigc::signal<void(LocalBoard)> remove_board_signal;
    sigc::signal<void(LocalBoard)> save_board_signal;

private:
    mutable std::mutex valid_mutex;
    volatile bool m_loaded = false;
    void __local_save(const LocalBoard& local);
};


#pragma once

#include <tinyxml2.h>

#include <vector>

#include "cardlist.h"
#include "item.h"

enum class BackgroundType { COLOR, IMAGE, INVALID };

/**
 * @class Board
 *
 * @brief A class representing the kanban-style board of the application,
 */
class Board : public Item {
public:
    Board();

    /**
     * @brief Board constructor.
     *
     * @param name The board's name.
     * @param background The board's background. It can be either a path to an
     *                   image or a solid colour RGBA representation.
     */
    Board(const std::string& name, const std::string& background);

    /**
     * @brief Constructs a Board object from a filepath
     *
     * @param board_file_path valid path name pointing to a valid board xml file
     *
     * @details It's completely necessary for the filepath given to be valid,
     * that is, the file exists and the syntax of Progress Boards checks out.
     *
     * @throws std::domain_error if the board_file_path is not valid at all
     */
    Board(const std::string& board_file_path);

    /**
     * @brief Changes the background information of the board. If the given
     *        background is BackgroundType::INVALID, then the background will
     *        fall back for a default defined in Board::BACKGROUND_DEFAULT.
     *
     * @param other The new background.
     *
     * @returns a BackgroundType for later processing. It may be ignored
     */
    BackgroundType set_background(const std::string& other);

    /**
     * @brief Returns the current background value
     *
     * @returns The background value
     */
    std::string get_background() const;

    /**
     * @brief Sets a file path to where the board will be saved.
     *
     * @param filepath absolute path where the board will be saved
     *
     * @param create_dirs Flag indicating whether filepath's parent directory
     * can be created if it does not exist
     *
     * @returns true if the path was set successfully, otherwise false.
     */
    bool set_filepath(const std::string& file_path, bool create_dirs = true);

    /**
     * @brief Adds a CardList object to the board by moving the contents to a
     * dynamic allocated space.
     *
     * @param cardlist CardList object
     *
     * @returns a CardList pointer to the newly allocated object.
     */
    std::shared_ptr<CardList> add_cardlist(const CardList& cardlist);

    /**
     * @brief Removes a CardList object from the board and free the allocated
     * space linked to the cardlist object.
     *
     * @returns True if the CardList object is removed from the board.
     *          False is returned if the CardList object requested to be
     *          removed isn't in the board.
     */
    bool remove_cardlist(const CardList& cardlist);

    /**
     * @brief Reorders the next card after sibling
     */
    void reorder_cardlist(std::shared_ptr<CardList> next,
                          std::shared_ptr<CardList> sibling);

    /**
     * @brief Saves the board information as a file.
     *
     * @param create_dirs Flag indicating whether filepath's parent directory
     * can be created if it does not exist
     *
     * @details This method will create a new file based on the board's name. It
     *          will start numbering the files in case there are boards with the
     *          same name.
     *
     * @returns True if the file was created sucessfully. False may be returned
     * when the board's filepath is invalid (e.g parent directory does not
     * exist and create_dirs is false or an OS error).
     */
    bool save_as_xml(bool create_dirs = true);

    std::string get_filepath() const;

    const std::vector<std::shared_ptr<CardList>>& get_cardlist_vector();

    /**
     * @brief Returns true if the board was modified in some way, otherwise
     * False.
     *
     * @returns Boolean indicating if the board was modified.
     */
    bool get_modified() override;

    /**
     * @brief Returns the type of a given background
     *
     * @returns a BackgroundType enum informing the background's type
     */
    static BackgroundType get_background_type(const std::string& background);

    static const std::string BACKGROUND_DEFAULT;

private:
    std::string background, file_path;
    std::vector<std::shared_ptr<CardList>> cardlist_vector;
};
#pragma once

#include <tinyxml2.h>

#include <vector>

#include "cardlist.h"
#include "item.h"

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
    Board(std::string name, std::string background);

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
    Board(std::string board_file_path);

    /**
     * @brief Changes the background information of the board.
     *
     * @param other The new background.
     *
     * @returns True if the background member was successfully set, otherwise
     * False.
     */
    bool set_background(std::string other);

    /**
     * @brief Returns the current background value
     *
     * @returns The background value
     */
    std::string get_background() const;

    /**
     * @brief Sets a file path to where the board will be saved.
     *
     * @returns true if the path was set successfully, otherwise false.
     */
    bool set_filepath(const std::string& file_path);

    /**
     * @brief Returns the xml structure of this board object.
     *
     * @returns A string of xml structure of this object.
     */
    std::string xml_structure();

    /**
     * @brief Adds a CardList object to the board by moving the contents to a
     * dynamic allocated space.
     *
     * @param cardlist CardList rvalue
     *
     * @returns a CardList pointer to the newly allocated object.
     */
    std::shared_ptr<CardList> add_cardlist(CardList& cardlist);

    /**
     * @brief Adds a CardList object to the board by moving the contents to a
     * dynamic allocated space.
     *
     * @param cardlist CardList rvalue
     *
     * @returns a CardList pointer to the newly allocated object.
     */
    std::shared_ptr<CardList> add_cardlist(CardList&& cardlist);

    /**
     * @brief Removes a CardList object from the board and free the allocated
     * space linked to the cardlist object.
     *
     * @returns True if the CardList object is removed from the board.
     *          False is returned if the CardList object requested to be
     *          removed isn't in the board.
     */
    bool remove_cardlist(CardList& cardlist);

    /**
     * @brief Reorders the next card after sibling
     */
    void reorder_cardlist(std::shared_ptr<CardList> next,
                          std::shared_ptr<CardList> sibling);

    /**
     * @brief Saves the board information as a file.
     *
     * @details This method will create a new file based on the board's name. It
     *          will start numbering the files in case there are boards with the
     *          same name.
     *
     * @returns True if the file is created sucessfully. False is returned when
     *          the files already exists or there was a OS error.
     */
    bool save_as_xml();

    static std::string get_background_type(std::string background);

    const std::string get_filepath() const;

    /**
     * TODO: Remove this method. Don't give access to the underlying vector
     * holding the cardlists. Implement a way of iterating within the class
    */
    std::vector<std::shared_ptr<CardList>>& get_cardlists();

    /**
     * @brief Returns true if the board was modified in some way, otherwise
     * False.
     * 
     * @returns a boolean indicating if the board was modified.
    */
    bool is_modified() override;

    /**
     * @brief Generates a filename given a base. What it actually does is to
     * lowercase the given base and give an id if needed.
    */
    static const std::string new_filename(std::string base);

private:
    std::string background, file_path;

    // All CardLists pointers points to a dynamic object.
    std::vector<std::shared_ptr<CardList>> cardlist_vector;

    /**
     * @brief Checks whether a given string is of background type.
     *
     * @details The way this method checks if background is of background type
     *          is by checking whether the string is a file path or colour code
     *          (RGBA).
     *
     * @returns True if the background is of background type, otherwise False.
     */
    bool is_background(std::string& background) const;

    bool cardlists_modified();

    tinyxml2::XMLDocument* xml_doc();
};
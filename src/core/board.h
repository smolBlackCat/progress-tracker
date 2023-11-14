#pragma once

#include <vector>

#include "cardlist.h"
#include "item.h"
#include <tinyxml2.h>

/**
 * @class Board
 *
 * @brief A class representing the kanban-style board of the application,
 */
class Board : public Item {
public:
    /**
     * @brief Board constructor.
     *
     * @param name The board's name.
     * @param background The board's background. It can be either a path to an
     *                   image or a solid colour RGBA representation.
     */
    Board(std::string name, std::string background);

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
     * @brief Returns the xml structure of this board object.
     *
     * @returns A string of xml structure of this object.
     */
    std::string xml_structure() const;

    /**
     * @brief Adds a \ref CardList object to the board
     *
     * @returns True if the \ref CardList object was added successfully,
     * otherwise False.
     */
    bool add_cardlist(CardList& cardlist);

    /**
     * @brief Removes a \ref CardList object from the board.
     *
     * @returns True if the \ref CardList object is removed from the board.
     *          False is returned if the \ref CardList object requested to be
     *          removed isn't in the board.
     */
    bool remove_cardlist(CardList& cardlist);

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
    bool save_as_xml() const;

private:
    std::string background;
    const std::string boards_dir;
    std::vector<CardList> cardlist_vector;

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

    tinyxml2::XMLDocument* xml_doc() const;
};

/**
 * @brief Creates a pointer to a new Board object created from a xml file.
 *
 * @param xml_filename The file name. It can be relative or absolute.
 *
 * @returns Board pointer to the new Board object. The caller should deallocate
 *          the object after its use to avoid memory leaks. Note that a nullptr
 *          is returned if the object creation was not successful, either
 *          because the filename doesn't exist or the filename wasn't a proper
 *          Progress XML.
 */
Board* board_from_xml(std::string xml_filename);
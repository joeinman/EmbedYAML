/*
 * Copyright (c) 2025, Joe Inman
 *
 * Licensed under the MIT License.
 * You may obtain a copy of the License at:
 *     https://opensource.org/licenses/MIT
 *
 * This file is part of the EmbedYAML Library.
 */

#pragma once

#include <expected>
#include <string>

#include "EmbedYAML/Error.hpp"
#include "EmbedYAML/Node.hpp"

extern "C" {
#include <yaml.h>
}

namespace EmbedYAML
{

/**
 * @brief The EmbedYAML class provides functionality for parsing and emitting YAML data.
 */
class EmbedYAML
{
public:
    /**
     * @brief Parse a YAML input string into a Node.
     *
     * This function takes a YAML formatted string and parses it into an internal Node representation.
     *
     * @param input The YAML formatted string to be parsed.
     * @return An std::expected containing a Node if parsing is successful, or an EmbedYAMLError if an error occurs.
     */
    std::expected<Node, EmbedYAMLError> parse(const std::string& input) const noexcept;

    /**
     * @brief Emit a YAML document from a Node.
     *
     * Converts a Node representing a YAML structure into a YAML formatted string.
     *
     * @param node The Node representing the YAML data.
     * @return An std::expected containing a YAML string if successful, or an EmbedYAMLError if an error occurs.
     */
    std::expected<std::string, EmbedYAMLError> emit(const Node& node) const noexcept;

private:
    /**
     * @brief Parse a YAML node.
     *
     * Parses a YAML node from the provided parser and stores the result in the given Node.
     *
     * @param parser The YAML parser to use.
     * @param node The Node to store the parsed YAML data.
     * @return true if parsing was successful, false otherwise.
     */
    bool parseNode(yaml_parser_t& parser, Node& node) const noexcept;

    /**
     * @brief Parse a YAML node from a YAML event.
     *
     * Processes a YAML event to parse a node and stores the result in the given Node.
     *
     * @param parser The YAML parser in use.
     * @param event The current YAML event.
     * @param node The Node to store the parsed YAML data.
     * @return true if the node was parsed successfully, false otherwise.
     */
    bool parseNodeFromEvent(yaml_parser_t& parser, yaml_event_t& event, Node& node) const noexcept;

    /**
     * @brief Parse a scalar event into a Node.
     *
     * Converts a YAML scalar event into a Node containing the scalar value.
     *
     * @param event The YAML event representing a scalar.
     * @param node The Node to store the scalar value.
     * @return true if the scalar was parsed successfully, false otherwise.
     */
    static bool parseScalarEvent(yaml_event_t& event, Node& node) noexcept;

    /**
     * @brief Parse a sequence event into a Node.
     *
     * Processes a YAML sequence event and populates the given Node with sequence data.
     *
     * @param parser The YAML parser in use.
     * @param event The YAML event representing the start of a sequence.
     * @param node The Node to store the sequence data.
     * @param self Pointer to the EmbedYAML instance (for context).
     * @return true if the sequence was parsed successfully, false otherwise.
     */
    static bool parseSequenceEvent(yaml_parser_t&   parser,
                                   yaml_event_t&    event,
                                   Node&            node,
                                   const EmbedYAML* self) noexcept;

    /**
     * @brief Parse a mapping event into a Node.
     *
     * Processes a YAML mapping event and populates the given Node with mapping data.
     *
     * @param parser The YAML parser in use.
     * @param event The YAML event representing the start of a mapping.
     * @param node The Node to store the mapping data.
     * @param self Pointer to the EmbedYAML instance (for context).
     * @return true if the mapping was parsed successfully, false otherwise.
     */
    static bool parseMappingEvent(yaml_parser_t&   parser,
                                  yaml_event_t&    event,
                                  Node&            node,
                                  const EmbedYAML* self) noexcept;

    /**
     * @brief Emit a YAML node as a string.
     *
     * Converts the given Node into a YAML formatted string, using the specified indent level.
     *
     * @param node The Node representing the YAML data.
     * @param indentLevel The indentation level for the output string.
     * @return An std::expected containing the YAML string if successful, or an EmbedYAMLError if an error occurs.
     */
    std::expected<std::string, EmbedYAMLError> emitNode(const Node& node, int indentLevel) const noexcept;

    /**
     * @brief Emit a YAML scalar value as a string.
     *
     * Converts the scalar value contained in a Node to a YAML formatted string.
     *
     * @param node The Node containing the scalar value.
     * @return An std::expected containing the YAML string if successful, or an EmbedYAMLError if an error occurs.
     */
    static std::expected<std::string, EmbedYAMLError> emitScalar(const Node& node) noexcept;

    /**
     * @brief Emit a YAML sequence as a string.
     *
     * Converts the sequence contained in a Node to a YAML formatted string, applying the specified indentation.
     *
     * @param node The Node containing the sequence.
     * @param indentLevel The current indentation level for formatting.
     * @param self Pointer to the EmbedYAML instance (for context).
     * @return An std::expected containing the YAML string if successful, or an EmbedYAMLError if an error occurs.
     */
    static std::expected<std::string, EmbedYAMLError> emitSequence(const Node&      node,
                                                                   int              indentLevel,
                                                                   const EmbedYAML* self) noexcept;

    /**
     * @brief Emit a YAML mapping as a string.
     *
     * Converts the mapping contained in a Node to a YAML formatted string, applying the specified indentation.
     *
     * @param node The Node containing the mapping.
     * @param indentLevel The current indentation level for formatting.
     * @param self Pointer to the EmbedYAML instance (for context).
     * @return An std::expected containing the YAML string if successful, or an EmbedYAMLError if an error occurs.
     */
    static std::expected<std::string, EmbedYAMLError> emitMapping(const Node&      node,
                                                                  int              indentLevel,
                                                                  const EmbedYAML* self) noexcept;

    /**
     * @brief Generate an indentation string.
     *
     * Creates a string consisting of spaces for indentation, based on the given indentation level.
     *
     * @param indentLevel The number of indentation levels (each level corresponds to 2 spaces).
     * @return A string with the appropriate number of spaces for indentation.
     */
    std::string indentString(int indentLevel) const noexcept { return std::string(indentLevel * 2, ' '); }
};

}  // namespace EmbedYAML

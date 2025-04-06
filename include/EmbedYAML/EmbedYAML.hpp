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

class EmbedYAML
{
public:
    // Parse a YAML input string into a Node.
    std::expected<Node, EmbedYAMLError> parse(const std::string& input) const noexcept;

    // Emit a YAML document from a Node.
    std::expected<std::string, EmbedYAMLError> emit(const Node& node) const noexcept;

private:
    // Parsing functions.
    bool        parseNode(yaml_parser_t& parser, Node& node) const noexcept;
    bool        parseNodeFromEvent(yaml_parser_t& parser, yaml_event_t& event, Node& node) const noexcept;
    static bool parseScalarEvent(yaml_event_t& event, Node& node) noexcept;
    static bool parseSequenceEvent(yaml_parser_t&   parser,
                                   yaml_event_t&    event,
                                   Node&            node,
                                   const EmbedYAML* self) noexcept;
    static bool parseMappingEvent(yaml_parser_t&   parser,
                                  yaml_event_t&    event,
                                  Node&            node,
                                  const EmbedYAML* self) noexcept;

    // Emitting function.
    std::expected<std::string, EmbedYAMLError>        emitNode(const Node& node, int indentLevel) const noexcept;
    static std::expected<std::string, EmbedYAMLError> emitScalar(const Node& node) noexcept;
    static std::expected<std::string, EmbedYAMLError> emitSequence(const Node&      node,
                                                                   int              indentLevel,
                                                                   const EmbedYAML* self) noexcept;
    static std::expected<std::string, EmbedYAMLError> emitMapping(const Node&      node,
                                                                  int              indentLevel,
                                                                  const EmbedYAML* self) noexcept;

    // Generate an indent string.
    std::string indentString(int indentLevel) const noexcept { return std::string(indentLevel * 2, ' '); }
};

}  // namespace EmbedYAML

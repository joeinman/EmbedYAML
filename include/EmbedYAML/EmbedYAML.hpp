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

#include <yaml.h>

#include "Node.hpp"

namespace EmbedYAML
{

class EmbedYAML
{
public:
    std::optional<Node>        parse(const std::string& input) const noexcept;
    std::optional<std::string> emit(const Node& node) const noexcept;

private:
    bool parseNode(yaml_parser_t& parser, Node& node) const noexcept;
    bool parseNodeFromEvent(yaml_parser_t& parser, yaml_event_t& event, Node& node) const noexcept;

    std::optional<std::string> emitNode(const Node& node, int indentLevel) const noexcept;
    std::string indentString(int indentLevel) const noexcept { return std::string(indentLevel * 2, ' '); };
};

}  // namespace EmbedYAML

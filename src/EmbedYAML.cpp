/*
 * Copyright (c) 2025, Joe Inman
 *
 * Licensed under the MIT License.
 * You may obtain a copy of the License at:
 *     https://opensource.org/licenses/MIT
 *
 * This file is part of the EmbedYAML Library.
 */

#include "EmbedYAML/EmbedYAML.hpp"

namespace EmbedYAML
{

std::optional<Node> EmbedYAML::parse(const std::string& input) const noexcept
{
    yaml_parser_t parser;
    if (!yaml_parser_initialize(&parser))
    {
        return std::nullopt;
    }

    yaml_parser_set_input_string(&parser, reinterpret_cast<const unsigned char*>(input.c_str()), input.size());

    yaml_event_t event;
    // Stream start.
    if (!yaml_parser_parse(&parser, &event))
    {
        yaml_parser_delete(&parser);
        return std::nullopt;
    }
    if (event.type != YAML_STREAM_START_EVENT)
    {
        yaml_event_delete(&event);
        yaml_parser_delete(&parser);
        return std::nullopt;
    }
    yaml_event_delete(&event);

    // Document start.
    if (!yaml_parser_parse(&parser, &event))
    {
        yaml_parser_delete(&parser);
        return std::nullopt;
    }
    if (event.type != YAML_DOCUMENT_START_EVENT)
    {
        yaml_event_delete(&event);
        yaml_parser_delete(&parser);
        return std::nullopt;
    }
    yaml_event_delete(&event);

    Node root;
    if (!parseNode(parser, root))
    {
        yaml_parser_delete(&parser);
        return std::nullopt;
    }

    // Document end.
    if (!yaml_parser_parse(&parser, &event))
    {
        yaml_parser_delete(&parser);
        return std::nullopt;
    }
    if (event.type != YAML_DOCUMENT_END_EVENT)
    {
        yaml_event_delete(&event);
        yaml_parser_delete(&parser);
        return std::nullopt;
    }
    yaml_event_delete(&event);

    // Stream end.
    if (!yaml_parser_parse(&parser, &event))
    {
        yaml_parser_delete(&parser);
        return std::nullopt;
    }
    if (event.type != YAML_STREAM_END_EVENT)
    {
        yaml_event_delete(&event);
        yaml_parser_delete(&parser);
        return std::nullopt;
    }
    yaml_event_delete(&event);

    yaml_parser_delete(&parser);
    return root;
}

bool EmbedYAML::parseNode(yaml_parser_t& parser, Node& node) const noexcept
{
    yaml_event_t event;
    if (!yaml_parser_parse(&parser, &event))
    {
        return false;
    }
    return parseNodeFromEvent(parser, event, node);
}

bool EmbedYAML::parseNodeFromEvent(yaml_parser_t& parser, yaml_event_t& event, Node& node) const noexcept
{
    switch (event.type)
    {
    case YAML_SCALAR_EVENT:
    {
        node.value = ScalarType(reinterpret_cast<const char*>(event.data.scalar.value), event.data.scalar.length);
        node.type  = NodeType::Scalar;
        yaml_event_delete(&event);
        return true;
    }
    case YAML_SEQUENCE_START_EVENT:
    {
        // For sequences, initialize an empty vector.
        node.value = SequenceType{};
        node.type  = NodeType::Sequence;
        yaml_event_delete(&event);
        while (true)
        {
            yaml_event_t childEvent;
            if (!yaml_parser_parse(&parser, &childEvent))
            {
                return false;
            }
            if (childEvent.type == YAML_SEQUENCE_END_EVENT)
            {
                yaml_event_delete(&childEvent);
                break;
            }
            Node child;
            if (!parseNodeFromEvent(parser, childEvent, child))
                return false;
            std::get<SequenceType>(node.value).push_back(child);
        }
        return true;
    }
    case YAML_MAPPING_START_EVENT:
    {
        node.value = MapType{};
        node.type  = NodeType::Map;
        yaml_event_delete(&event);

        while (true)
        {
            yaml_event_t keyEvent;
            if (!yaml_parser_parse(&parser, &keyEvent))
                return false;
            if (keyEvent.type == YAML_MAPPING_END_EVENT)
            {
                yaml_event_delete(&keyEvent);
                break;
            }

            Node keyNode;
            if (!parseNodeFromEvent(parser, keyEvent, keyNode))
                return false;
            if (!std::holds_alternative<std::string>(keyNode.value))
                return false;
            std::string key = std::get<std::string>(keyNode.value);

            Node valueNode;
            if (!parseNode(parser, valueNode))
                return false;

            std::get<MapType>(node.value).emplace_back(key, valueNode);
        }

        return true;
    }
    default:
        yaml_event_delete(&event);
        return false;
    }
}

// Recursively emits YAML text for a given node with the specified indent level.
std::optional<std::string> EmbedYAML::emitNode(const Node& node, int indentLevel) const noexcept
{
    std::string output;
    std::string indent = indentString(indentLevel);

    switch (node.type)
    {
    case NodeType::Scalar:
    {
        // Ensure the variant holds a ScalarType.
        if (!std::holds_alternative<ScalarType>(node.value))
            return std::nullopt;
        output += std::get<ScalarType>(node.value);
        break;
    }
    case NodeType::Sequence:
    {
        if (!std::holds_alternative<SequenceType>(node.value))
            return std::nullopt;
        const auto& seq = std::get<SequenceType>(node.value);
        for (const auto& element : seq)
        {
            // Emit the element. For scalars, keep the same indent (0) for the element content.
            auto res = element.isScalar() ? emitNode(element, 0) : emitNode(element, indentLevel + 1);
            if (!res.has_value())
                return std::nullopt;
            if (element.isScalar())
                output += indent + "- " + res.value() + "\n";
            else
                output += indent + "-\n" + res.value();
        }
        break;
    }
    case NodeType::Map:
    {
        if (!std::holds_alternative<MapType>(node.value))
            return std::nullopt;
        const auto& map = std::get<MapType>(node.value);
        for (const auto& [key, value] : map)
        {
            auto res = value.isScalar() ? emitNode(value, 0) : emitNode(value, indentLevel + 1);
            if (!res.has_value())
                return std::nullopt;
            if (value.isScalar())
                output += indent + key + ": " + res.value() + "\n";
            else
                output += indent + key + ":\n" + res.value();
        }
        break;
    }
    case NodeType::Null:
    default:
    {
        output += "null";
        break;
    }
    }
    return output;
}

std::optional<std::string> EmbedYAML::emit(const Node& node) const noexcept
{
    return emitNode(node, 0);
}

}  // namespace EmbedYAML

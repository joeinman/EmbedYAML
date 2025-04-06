/*
 * Copyright (c) 2025, Joe Inman
 *
 * Licensed under the MIT License.
 * You may obtain a copy of the License at:
 *     https://opensource.org/licenses/MIT
 *
 * This file is part of the EmbedYAML Library.
 */

#include <memory>
#include <utility>
#include "EmbedYAML/EmbedYAML.hpp"

namespace EmbedYAML
{

//============================================================================
// Public API Functions
//============================================================================

std::expected<Node, EmbedYAMLError> EmbedYAML::parse(const std::string& input) const noexcept
{
    yaml_parser_t parser;
    if (!yaml_parser_initialize(&parser))
    {
        return std::unexpected(EmbedYAMLError{EmbedYAMLErrorType::ParseError, "Failed to initialize YAML parser"});
    }
    yaml_parser_set_input_string(&parser, reinterpret_cast<const unsigned char*>(input.c_str()), input.size());

    yaml_event_t event;
    // Stream start.
    if (!yaml_parser_parse(&parser, &event))
    {
        yaml_parser_delete(&parser);
        return std::unexpected(EmbedYAMLError{EmbedYAMLErrorType::ParseError, "Failed to parse stream start"});
    }
    if (event.type != YAML_STREAM_START_EVENT)
    {
        yaml_event_delete(&event);
        yaml_parser_delete(&parser);
        return std::unexpected(EmbedYAMLError{EmbedYAMLErrorType::ParseError, "Expected stream start event"});
    }
    yaml_event_delete(&event);

    // Document start.
    if (!yaml_parser_parse(&parser, &event))
    {
        yaml_parser_delete(&parser);
        return std::unexpected(EmbedYAMLError{EmbedYAMLErrorType::ParseError, "Failed to parse document start"});
    }
    if (event.type != YAML_DOCUMENT_START_EVENT)
    {
        yaml_event_delete(&event);
        yaml_parser_delete(&parser);
        return std::unexpected(EmbedYAMLError{EmbedYAMLErrorType::ParseError, "Expected document start event"});
    }
    yaml_event_delete(&event);

    // For configuration, assume a single document (a Map).
    Node root(NodeType::Map);
    if (!parseNode(parser, root))
    {
        yaml_parser_delete(&parser);
        return std::unexpected(EmbedYAMLError{EmbedYAMLErrorType::ParseError, "Failed to parse YAML node"});
    }

    // Document end.
    if (!yaml_parser_parse(&parser, &event))
    {
        yaml_parser_delete(&parser);
        return std::unexpected(EmbedYAMLError{EmbedYAMLErrorType::ParseError, "Failed to parse document end"});
    }
    if (event.type != YAML_DOCUMENT_END_EVENT)
    {
        yaml_event_delete(&event);
        yaml_parser_delete(&parser);
        return std::unexpected(EmbedYAMLError{EmbedYAMLErrorType::ParseError, "Expected document end event"});
    }
    yaml_event_delete(&event);

    // Stream end.
    if (!yaml_parser_parse(&parser, &event))
    {
        yaml_parser_delete(&parser);
        return std::unexpected(EmbedYAMLError{EmbedYAMLErrorType::ParseError, "Failed to parse stream end"});
    }
    if (event.type != YAML_STREAM_END_EVENT)
    {
        yaml_event_delete(&event);
        yaml_parser_delete(&parser);
        return std::unexpected(EmbedYAMLError{EmbedYAMLErrorType::ParseError, "Expected stream end event"});
    }
    yaml_event_delete(&event);

    yaml_parser_delete(&parser);
    return root;
}

std::expected<std::string, EmbedYAMLError> EmbedYAML::emit(const Node& node) const noexcept
{
    return emitNode(node, 0);
}

//============================================================================
// Parsing Functions
//============================================================================

bool EmbedYAML::parseNode(yaml_parser_t& parser, Node& node) const noexcept
{
    yaml_event_t event;
    if (!yaml_parser_parse(&parser, &event))
        return false;
    return parseNodeFromEvent(parser, event, node);
}

bool EmbedYAML::parseNodeFromEvent(yaml_parser_t& parser, yaml_event_t& event, Node& node) const noexcept
{
    switch (event.type)
    {
    case YAML_SCALAR_EVENT:
        return parseScalarEvent(event, node);
    case YAML_SEQUENCE_START_EVENT:
        return parseSequenceEvent(parser, event, node, this);
    case YAML_MAPPING_START_EVENT:
        return parseMappingEvent(parser, event, node, this);
    default:
        yaml_event_delete(&event);
        return false;
    }
}

bool EmbedYAML::parseScalarEvent(yaml_event_t& event, Node& node) noexcept
{
    std::string scalar(reinterpret_cast<const char*>(event.data.scalar.value), event.data.scalar.length);
    node = scalar;
    // Ensure node is set to Scalar.
    node = Node(NodeType::Scalar);
    node = scalar;
    yaml_event_delete(&event);
    return true;
}

bool EmbedYAML::parseSequenceEvent(yaml_parser_t&   parser,
                                   yaml_event_t&    event,
                                   Node&            node,
                                   const EmbedYAML* self) noexcept
{
    node = Node(NodeType::Sequence);
    yaml_event_delete(&event);
    while (true)
    {
        yaml_event_t childEvent;
        if (!yaml_parser_parse(&parser, &childEvent))
            return false;
        if (childEvent.type == YAML_SEQUENCE_END_EVENT)
        {
            yaml_event_delete(&childEvent);
            break;
        }
        Node child(NodeType::Null);
        if (!self->parseNodeFromEvent(parser, childEvent, child))
            return false;
        node.emplace_back(child);
    }
    return true;
}

bool EmbedYAML::parseMappingEvent(yaml_parser_t&   parser,
                                  yaml_event_t&    event,
                                  Node&            node,
                                  const EmbedYAML* self) noexcept
{
    node = Node(NodeType::Map);
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
        Node keyNode(NodeType::Scalar);
        if (!self->parseNodeFromEvent(parser, keyEvent, keyNode))
            return false;
        if (!keyNode.isScalar())
            return false;  // Only scalar keys supported.
        auto optKey = keyNode.asString();
        if (!optKey.has_value())
            return false;
        std::string key = optKey.value();
        Node        valueNode(NodeType::Null);
        if (!self->parseNode(parser, valueNode))
            return false;
        auto* m = node.asMap();
        if (m)
        {
            m->push_back(MapEntry{key, std::make_unique<Node>(std::move(valueNode))});
        }
    }
    return true;
}

//============================================================================
// Emitting Functions
//============================================================================

std::expected<std::string, EmbedYAMLError> EmbedYAML::emitNode(const Node& node, int indentLevel) const noexcept
{
    switch (node.getType())
    {
    case NodeType::Scalar:
        return emitScalar(node);
    case NodeType::Sequence:
        return emitSequence(node, indentLevel, this);
    case NodeType::Map:
        return emitMapping(node, indentLevel, this);
    case NodeType::Null:
    default:
        return std::unexpected(EmbedYAMLError{EmbedYAMLErrorType::EmissionError, "Invalid node type for emission"});
    }
}

std::expected<std::string, EmbedYAMLError> EmbedYAML::emitScalar(const Node& node) noexcept
{
    auto optScalar = node.asString();
    if (!optScalar.has_value())
        return std::unexpected(EmbedYAMLError{EmbedYAMLErrorType::EmissionError, "Invalid scalar node"});
    return optScalar.value();
}

std::expected<std::string, EmbedYAMLError> EmbedYAML::emitSequence(const Node&      node,
                                                                   int              indentLevel,
                                                                   const EmbedYAML* self) noexcept
{
    std::string output;
    std::string indent = self->indentString(indentLevel);
    auto*       seq    = node.asSequence();
    if (!seq)
        return std::unexpected(EmbedYAMLError{EmbedYAMLErrorType::EmissionError, "Invalid sequence node"});
    for (const auto& elementPtr : *seq)
    {
        auto res =
            elementPtr->isScalar() ? self->emitNode(*elementPtr, 0) : self->emitNode(*elementPtr, indentLevel + 1);
        if (!res.has_value())
            return std::unexpected(
                EmbedYAMLError{EmbedYAMLErrorType::EmissionError, "Failed to emit sequence element"});
        if (elementPtr->isScalar())
            output += indent + "- " + res.value() + "\n";
        else
            output += indent + "-\n" + res.value();
    }
    return output;
}

std::expected<std::string, EmbedYAMLError> EmbedYAML::emitMapping(const Node&      node,
                                                                  int              indentLevel,
                                                                  const EmbedYAML* self) noexcept
{
    std::string output;
    std::string indent = self->indentString(indentLevel);
    auto*       map    = node.asMap();
    if (!map)
        return std::unexpected(EmbedYAMLError{EmbedYAMLErrorType::EmissionError, "Invalid mapping node"});
    for (const auto& entry : *map)
    {
        auto res =
            entry.value->isScalar() ? self->emitNode(*entry.value, 0) : self->emitNode(*entry.value, indentLevel + 1);
        if (!res.has_value())
            return std::unexpected(EmbedYAMLError{EmbedYAMLErrorType::EmissionError, "Failed to emit mapping element"});
        if (entry.value->isScalar())
            output += indent + entry.key + ": " + res.value() + "\n";
        else
            output += indent + entry.key + ":\n" + res.value();
    }
    return output;
}

}  // namespace EmbedYAML

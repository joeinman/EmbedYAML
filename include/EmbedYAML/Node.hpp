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

#include <charconv>
#include <optional>
#include <expected>
#include <sstream>
#include <string>
#include <vector>
#include <variant>
#include <memory>
#include <cassert>

#include "EmbedYAML/Error.hpp"

namespace EmbedYAML
{

class Node;

using NullType     = std::monostate;
using ScalarType   = std::string;
using SequenceType = std::vector<std::unique_ptr<Node>>;

struct MapEntry
{
    std::string           key;
    std::unique_ptr<Node> value;
};

using MapType = std::vector<MapEntry>;

using NodeVariant = std::variant<NullType, ScalarType, SequenceType, MapType>;

enum class NodeType
{
    Null,
    Scalar,
    Sequence,
    Map
};

class Node
{
public:
    Node() : type(NodeType::Null), value(NullType{}) {}
    explicit Node(NodeType t) : type(t)
    {
        switch (t)
        {
        case NodeType::Null:
            value = NullType{};
            break;
        case NodeType::Scalar:
            value = ScalarType{};
            break;
        case NodeType::Sequence:
            value = SequenceType{};
            break;
        case NodeType::Map:
            value = MapType{};
            break;
        }
    }

    // Disable copy because of unique_ptr members.
    Node(const Node&)                = delete;
    Node& operator=(const Node&)     = delete;
    Node(Node&&) noexcept            = default;
    Node& operator=(Node&&) noexcept = default;

    bool     isNull() const { return type == NodeType::Null; }
    bool     isScalar() const { return type == NodeType::Scalar; }
    bool     isSequence() const { return type == NodeType::Sequence; }
    bool     isMap() const { return type == NodeType::Map; }
    NodeType getType() const { return type; }

    template <typename T>
    std::expected<T, EmbedYAMLError> as() const
    {
        if (!isScalar())
            return std::unexpected(EmbedYAMLError{EmbedYAMLErrorType::TypeError, "Node is not a Scalar"});
        const auto& s = std::get<ScalarType>(value);
        if constexpr (std::is_same_v<T, std::string>)
            return s;
        else if constexpr (std::is_integral_v<T>)
        {
            T result{};
            auto [ptr, ec] = std::from_chars(s.data(), s.data() + s.size(), result);
            if (ec != std::errc())
                return std::unexpected(EmbedYAMLError{EmbedYAMLErrorType::ScalarConversionError, "Conversion failed"});
            return result;
        }
        else if constexpr (std::is_floating_point_v<T>)
        {
            T result{};
            auto [ptr, ec] = std::from_chars(s.data(), s.data() + s.size(), result);
            if (ec != std::errc())
                return std::unexpected(EmbedYAMLError{EmbedYAMLErrorType::ScalarConversionError, "Conversion failed"});
            return result;
        }
        return std::unexpected(
            EmbedYAMLError{EmbedYAMLErrorType::ScalarConversionError, "Unsupported type conversion"});
    }

    // Map access operator.
    Node& operator[](const std::string& key)
    {
        assert(isMap() && "Node is not a Map");
        auto& map = std::get<MapType>(value);
        for (auto& entry : map)
        {
            if (entry.key == key)
                return *entry.value;
        }
        // Not found: create a new null node.
        map.push_back({key, std::make_unique<Node>(NodeType::Null)});
        return *map.back().value;
    }

    // Sequence access operator.
    Node& operator[](size_t index)
    {
        assert(isSequence() && "Node is not a Sequence");
        auto& seq = std::get<SequenceType>(value);
        assert(index < seq.size() && "Index out of range");
        return *seq[index];
    }

    // Append a new element to a sequence.
    template <typename T>
    Node& emplace_back(const T& v)
    {
        assert(isSequence() && "Node is not a Sequence");
        auto& seq = std::get<SequenceType>(value);
        if constexpr (std::is_same_v<T, Node>)
        {
            seq.push_back(std::make_unique<Node>(std::move(const_cast<Node&>(v))));
        }
        else
        {
            Node temp;
            temp = v;
            seq.push_back(std::make_unique<Node>(std::move(temp)));
        }
        return *this;
    }

    // Assignment operator for arithmetic and convertible types.
    template <typename T>
    Node& operator=(const T& v)
    {
        if constexpr (std::is_arithmetic_v<T>)
        {
            type  = NodeType::Scalar;
            value = std::to_string(v);
        }
        else if constexpr (std::is_convertible_v<T, std::string>)
        {
            type  = NodeType::Scalar;
            value = std::string(v);
        }
        else
        {
            type = NodeType::Scalar;
            std::ostringstream oss;
            oss << v;
            value = oss.str();
        }
        return *this;
    }

private:
    NodeType    type;
    NodeVariant value;
    friend class EmbedYAML;

    // For scalars.
    std::expected<std::string, EmbedYAMLError> asString() const
    {
        if (!isScalar())
            return std::unexpected(EmbedYAMLError{EmbedYAMLErrorType::TypeError, "Node is not a Scalar"});
        return std::get<ScalarType>(value);
    }

    // Return pointer to underlying Sequence (or nullptr if not a sequence).
    SequenceType* asSequence()
    {
        if (isSequence())
            return &std::get<SequenceType>(value);
        return nullptr;
    }
    const SequenceType* asSequence() const
    {
        if (isSequence())
            return &std::get<SequenceType>(value);
        return nullptr;
    }

    // Return pointer to underlying Map (or nullptr if not a map).
    MapType* asMap()
    {
        if (isMap())
            return &std::get<MapType>(value);
        return nullptr;
    }
    const MapType* asMap() const
    {
        if (isMap())
            return &std::get<MapType>(value);
        return nullptr;
    }
};

}  // namespace EmbedYAML

/**
 * @file Node.hpp
 * @brief Contains the definition of the Node class used to represent YAML data structures.
 *
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

/**
 * @brief Represents a map entry in a YAML mapping.
 *
 * A map entry consists of a key and a corresponding value stored as a Node.
 */
struct MapEntry
{
    std::string           key;    ///< The key for the mapping entry.
    std::unique_ptr<Node> value;  ///< The value associated with the key.
};

/**
 * @brief Container type for a YAML mapping.
 *
 * A vector of map entries representing key-value pairs.
 */
using MapType = std::vector<MapEntry>;

/**
 * @brief Represents the various types of data that a Node can hold.
 *
 * A Node can be one of the following types:
 * - Null: Represents an empty or undefined value.
 * - Scalar: Represents a single value stored as a string.
 * - Sequence: Represents a list of Nodes.
 * - Map: Represents a key-value mapping.
 */
enum class NodeType
{
    Null,      ///< Represents a null value.
    Scalar,    ///< Represents a scalar value (stored as std::string).
    Sequence,  ///< Represents a sequence (list) of Nodes.
    Map        ///< Represents a mapping of key-value pairs.
};

/**
 * @brief Variant types used to store the underlying value of a Node.
 */
using NullType     = std::monostate;
using ScalarType   = std::string;
using SequenceType = std::vector<std::unique_ptr<Node>>;
using NodeVariant  = std::variant<NullType, ScalarType, SequenceType, MapType>;

/**
 * @class Node
 * @brief Represents a YAML node which can be a scalar, sequence, or map.
 *
 * The Node class is used to represent YAML data in a structured manner. It can store
 * different types of values including Null, Scalar, Sequence, and Map. The class provides
 * methods to access and manipulate these values as well as conversion utilities.
 */
class Node
{
public:
    /**
     * @brief Default constructor.
     *
     * Initializes the Node as a Null type.
     */
    Node() : type(NodeType::Null), value(NullType{}) {}

    /**
     * @brief Constructs a Node with the specified type.
     *
     * @param t The type of the Node (Null, Scalar, Sequence, or Map).
     */
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

    /**
     * @brief Checks if the Node is of Null type.
     * @return true if the Node is Null, false otherwise.
     */
    bool isNull() const { return type == NodeType::Null; }

    /**
     * @brief Checks if the Node is a Scalar.
     * @return true if the Node is a Scalar, false otherwise.
     */
    bool isScalar() const { return type == NodeType::Scalar; }

    /**
     * @brief Checks if the Node is a Sequence.
     * @return true if the Node is a Sequence, false otherwise.
     */
    bool isSequence() const { return type == NodeType::Sequence; }

    /**
     * @brief Checks if the Node is a Map.
     * @return true if the Node is a Map, false otherwise.
     */
    bool isMap() const { return type == NodeType::Map; }

    /**
     * @brief Retrieves the type of the Node.
     * @return The NodeType representing the type of the Node.
     */
    NodeType getType() const { return type; }

    /**
     * @brief Converts the Node's scalar value to the specified type.
     *
     * This template method attempts to convert the Node's scalar value to the desired type T.
     * It supports conversion to std::string, integral types, and floating-point types.
     *
     * @tparam T The type to which the scalar value should be converted.
     * @return An std::expected containing the converted value if successful, or an EmbedYAMLError on failure.
     */
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

    /**
     * @brief Access operator for map-type Nodes.
     *
     * Retrieves the Node associated with the given key in a map. If the key does not exist,
     * a new Node of type Null is created and inserted.
     *
     * @param key The key for which the Node is to be retrieved.
     * @return A reference to the Node corresponding to the given key.
     *
     * @pre The Node must be of Map type.
     */
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

    /**
     * @brief Access operator for sequence-type Nodes.
     *
     * Retrieves the Node at the specified index in a sequence.
     *
     * @param index The index of the Node to retrieve.
     * @return A reference to the Node at the given index.
     *
     * @pre The Node must be of Sequence type.
     */
    Node& operator[](size_t index)
    {
        assert(isSequence() && "Node is not a Sequence");
        auto& seq = std::get<SequenceType>(value);
        assert(index < seq.size() && "Index out of range");
        return *seq[index];
    }

    /**
     * @brief Appends a new element to a sequence-type Node.
     *
     * This template method adds a new element to the sequence. If the type T is Node,
     * the value is moved directly; otherwise, a temporary Node is created from the value.
     *
     * @tparam T The type of the value to append.
     * @param v The value to append to the sequence.
     * @return A reference to the Node after appending the new element.
     *
     * @pre The Node must be of Sequence type.
     */
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

    /**
     * @brief Assignment operator for arithmetic and convertible types.
     *
     * Assigns a new scalar value to the Node by converting the provided value to a string.
     *
     * @tparam T The type of the value being assigned.
     * @param v The value to assign to the Node.
     * @return A reference to the Node after assignment.
     */
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
    NodeType    type;   ///< The type of the Node.
    NodeVariant value;  ///< The underlying value stored in the Node.

    friend class EmbedYAML;

    /**
     * @brief Retrieves the scalar value of the Node as a string.
     *
     * @return An std::expected containing the scalar value as a string if the Node is a Scalar,
     *         otherwise an EmbedYAMLError.
     */
    std::expected<std::string, EmbedYAMLError> asString() const
    {
        if (!isScalar())
            return std::unexpected(EmbedYAMLError{EmbedYAMLErrorType::TypeError, "Node is not a Scalar"});
        return std::get<ScalarType>(value);
    }

    /**
     * @brief Retrieves a pointer to the underlying Sequence.
     *
     * @return A pointer to the Sequence if the Node is of Sequence type, or nullptr otherwise.
     */
    SequenceType* asSequence()
    {
        if (isSequence())
            return &std::get<SequenceType>(value);
        return nullptr;
    }

    /**
     * @brief Retrieves a const pointer to the underlying Sequence.
     *
     * @return A const pointer to the Sequence if the Node is of Sequence type, or nullptr otherwise.
     */
    const SequenceType* asSequence() const
    {
        if (isSequence())
            return &std::get<SequenceType>(value);
        return nullptr;
    }

    /**
     * @brief Retrieves a pointer to the underlying Map.
     *
     * @return A pointer to the Map if the Node is of Map type, or nullptr otherwise.
     */
    MapType* asMap()
    {
        if (isMap())
            return &std::get<MapType>(value);
        return nullptr;
    }

    /**
     * @brief Retrieves a const pointer to the underlying Map.
     *
     * @return A const pointer to the Map if the Node is of Map type, or nullptr otherwise.
     */
    const MapType* asMap() const
    {
        if (isMap())
            return &std::get<MapType>(value);
        return nullptr;
    }
};

}  // namespace EmbedYAML

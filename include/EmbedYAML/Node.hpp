#pragma once

#include <charconv>
#include <map>
#include <optional>
#include <sstream>
#include <string>
#include <variant>
#include <vector>
#include <type_traits>

namespace EmbedYAML
{

class Node;

enum class NodeType
{
    Null,
    Scalar,
    Sequence,
    Map
};
typedef std::monostate               NullType;
typedef std::string                  ScalarType;
typedef std::vector<Node>            SequenceType;
typedef std::pair<std::string, Node> MapEntry;
typedef std::vector<MapEntry>        MapType;
using NodeVariant = std::variant<NullType, ScalarType, SequenceType, MapType>;

class Node
{
public:
    Node(NodeType type = NodeType::Null) : type(type)
    {
        if (type == NodeType::Null)
            value = NullType{};
        else if (type == NodeType::Scalar)
            value = ScalarType{};
        else if (type == NodeType::Sequence)
            value = SequenceType{};
        else if (type == NodeType::Map)
            value = MapType{};
    }

    // Access a child node by key.
    Node& operator[](const std::string& key)
    {
        if (isMap())
        {
            auto& vec = std::get<MapType>(value);
            for (auto& [k, v] : vec)
            {
                if (k == key)
                    return v;
            }

            // If the key does not exist, create a new entry.
            vec.emplace_back(key, Node{});
            return vec.back().second;
        }
        return *this;
    }

    Node& operator[](const size_t& index)
    {
        if (isSequence())
        {
            auto& seq = std::get<SequenceType>(value);
            if (index < seq.size())
                return seq[index];
        }
        else if (isMap())
        {
            auto& vec = std::get<MapType>(value);
            if (index < vec.size())
                return vec[index].second;
        }
        return *this;
    }

    size_t size() const
    {
        if (isSequence())
        {
            return std::get<SequenceType>(value).size();
        }
        else if (isMap())
        {
            return std::get<MapType>(value).size();
        }
        return 1;
    }

    template <typename T>
    T as() const
    {
        if (!isScalar())
            return T{};
        const ScalarType& str = std::get<ScalarType>(value);

        if constexpr (std::is_same_v<T, std::string>)
        {
            return str;
        }
        else if constexpr (std::is_integral_v<T>)
        {
            T result{};
            auto [ptr, ec] = std::from_chars(str.data(), str.data() + str.size(), result);
            if (ec == std::errc())
            {
                return result;
            }
        }
        else if constexpr (std::is_floating_point_v<T>)
        {
            T result{};
            auto [ptr, ec] = std::from_chars(str.data(), str.data() + str.size(), result);
            if (ec == std::errc())
            {
                return result;
            }
        }

        return T{};
    }

    // Overload the assignment operator to set the value of a node.
    template <typename T>
    Node& operator=(const T& v)
    {
        if constexpr (std::is_same_v<T, Node>)
        {
            // Allow self-assignment or copying a Node
            if (this != &v)
            {
                type  = v.type;
                value = v.value;
            }
        }
        else if constexpr (std::is_same_v<T, std::vector<Node>>)
        {
            type  = NodeType::Sequence;
            value = v;
        }
        else if constexpr (std::is_same_v<T, std::vector<MapEntry>>)
        {
            type  = NodeType::Map;
            value = v;
        }
        else if constexpr (std::is_arithmetic_v<T>)
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
            // Fallback: use stringstream for any other type
            type = NodeType::Scalar;
            std::ostringstream oss;
            oss << v;
            value = oss.str();
        }
        return *this;
    }

    bool isScalar() const { return type == NodeType::Scalar; }
    bool isSequence() const { return type == NodeType::Sequence; }
    bool isMap() const { return type == NodeType::Map; }

private:
    NodeType    type  = NodeType::Null;
    NodeVariant value = std::monostate();
    friend class EmbedYAML;
};

}  // namespace EmbedYAML

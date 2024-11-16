#pragma once

#include <iostream>
#include <string>
#include <variant>
#include <vector>

class YAMLNode {
public:
    YAMLNode() = default;
    ~YAMLNode() = default;

    YAMLNode(const std::string &key)
        : m_key(key), m_data(std::vector<YAMLNode>()) {}

    YAMLNode(const std::string &key, const std::string &data)
        : m_key(key), m_data(data) {}

    YAMLNode(const std::string &key, const std::vector<YAMLNode> &data)
        : m_key(key), m_data(data) {}

    bool isScalar() const {
        return m_data.index() == 0;
    }

    bool isSequence() const {
        return m_data.index() == 1;
    }

    void addNode(const YAMLNode &node) {
        std::get<std::vector<YAMLNode>>(m_data).push_back(node);
    }

    void addScalar(const std::string &key, const std::string &data) {
        std::get<std::vector<YAMLNode>>(m_data).push_back(YAMLNode(key, data));
    }

    void addSequence(const std::string &key, const std::vector<YAMLNode> &data) {
        std::get<std::vector<YAMLNode>>(m_data).push_back(YAMLNode(key, data));
    }

        YAMLNode &operator[](size_t index) {
        return std::get<std::vector<YAMLNode>>(m_data)[index];
    }

    YAMLNode &operator[](const std::string &key) {
        for (auto &node : std::get<std::vector<YAMLNode>>(m_data)) {
            if (node.m_key == key) {
                return node;
            }
        }
        throw std::runtime_error("Key not found");
    }

    operator std::string() const {
        return std::get<std::string>(m_data);
    }

    std::string asScalar() const {
        return std::get<std::string>(m_data);
    }

private:
    std::string m_key;
    std::variant<std::string, std::vector<YAMLNode>> m_data;
};

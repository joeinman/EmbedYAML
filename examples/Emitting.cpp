/*
 * Copyright (c) 2025, Joe Inman
 *
 * Licensed under the MIT License.
 * You may obtain a copy of the License at:
 *     https://opensource.org/licenses/MIT
 *
 * This file is part of the EmbedYAML Library.
 */

#include <iostream>
#include <string>

#include <EmbedYAML/EmbedYAML.hpp>

using namespace EmbedYAML;

int main()
{
    EmbedYAML::EmbedYAML e;

    Node node(NodeType::Map);
    node["person"] = Node(NodeType::Sequence);
    node["person"].emplace_back("Name 1");
    node["person"].emplace_back("Name 2");
    node["other"]        = Node(NodeType::Map);
    node["other"]["key"] = 42;

    auto result = e.emit(node);
    if (!result.has_value())
    {
        std::cerr << "Error emitting YAML: " << result.error() << std::endl;
        return 1;
    }
    std::cout << result.value();
}

/*
 * Copyright (c) 2025, Joe Inman
 *
 * Licensed under the MIT License.
 * You may obtain a copy of the License at:
 *     https://opensource.org/licenses/MIT
 *
 * This file is part of the EmbedYAML Library.
 */

#include <EmbedYAML/EmbedYAML.hpp>

#include <iostream>
#include <string>

using namespace EmbedYAML;

int main()
{
    EmbedYAML::EmbedYAML e;

    auto node              = Node(NodeType::Map);
    node["person"]         = Node(NodeType::Map);
    node["person"]["name"] = "John Doe";
    node["person"]["age"]  = 30;

    auto result = e.emit(node);
    if (!result.has_value())
    {
        std::cerr << "Error emitting YAML" << std::endl;
        return 1;
    }
    std::cout << result.value();
}

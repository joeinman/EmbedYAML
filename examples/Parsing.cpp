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

int main()
{
    std::string yaml_string = "person:\n"
                              "  name: John Doe\n"
                              "  age: 30\n"
                              "  email: john.doe@example.com\n"
                              "  address:\n"
                              "    street: 123 Main St\n"
                              "    city: Springfield\n"
                              "    zip: 12345\n";

    EmbedYAML::EmbedYAML e;
    auto                 result = e.parse(yaml_string);
    if (!result.has_value())
    {
        std::cerr << "Error parsing YAML: " << result.error().message << std::endl;
        return 1;
    }

    auto& person = result.value()["person"];
    std::cout << "Name: " << person["name"].as<std::string>().value_or("N/A") << std::endl;
    std::cout << "Age: " << person["age"].as<int>().value_or(0) << std::endl;
    std::cout << "Email: " << person["email"].as<std::string>().value_or("N/A") << std::endl;
    std::cout << "Street: " << person["address"]["street"].as<std::string>().value_or("N/A") << std::endl;
    std::cout << "City: " << person["address"]["city"].as<std::string>().value_or("N/A") << std::endl;
    std::cout << "Zip: " << person["address"]["zip"].as<int>().value_or(0) << std::endl;
}

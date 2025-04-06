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

#include <iostream>
#include <string>
#include <array>

enum class EmbedYAMLErrorType
{
    ParseError,
    EmissionError
};

static std::array<std::string, 2> EmbedYAMLErrorTypeToString = {"Parse Error", "Emission Error"};

struct EmbedYAMLError
{
    EmbedYAMLErrorType error;
    std::string        message;

    operator std::string() const { return EmbedYAMLErrorTypeToString[static_cast<size_t>(error)] + ": " + message; }

    friend std::ostream& operator<<(std::ostream& os, const EmbedYAMLError& error)
    {
        os << static_cast<std::string>(error);
        return os;
    }
};

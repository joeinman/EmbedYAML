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

#include <string>

enum class EmbedYAMLErrorType
{
    ParseError,
    EmissionError
};

struct EmbedYAMLError
{
    EmbedYAMLErrorType error;
    std::string        message;
};
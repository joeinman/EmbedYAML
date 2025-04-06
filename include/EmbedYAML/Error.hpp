/**
 * @file EmbedYAMLError.hpp
 * @brief Defines error types and error handling for the EmbedYAML library.
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

#include <iostream>
#include <string>
#include <array>

/**
 * @enum EmbedYAMLErrorType
 * @brief Enumerates the different types of errors that can occur within EmbedYAML.
 */
enum class EmbedYAMLErrorType
{
    ParseError,            ///< An error occurred during YAML parsing.
    EmissionError,         ///< An error occurred during YAML emission.
    TypeError,             ///< A type mismatch error occurred.
    ScalarConversionError  ///< An error occurred during scalar value conversion.
};

/**
 * @brief Array mapping EmbedYAMLErrorType values to their string representations.
 */
static std::array<std::string, 4> EmbedYAMLErrorTypeToString = {"Parse Error",
                                                                "Emission Error",
                                                                "Type Error",
                                                                "Scalar Conversion Error"};

/**
 * @struct EmbedYAMLError
 * @brief Represents an error encountered in the EmbedYAML library.
 *
 * This structure encapsulates an error type and a corresponding descriptive message.
 */
struct EmbedYAMLError
{
    EmbedYAMLErrorType error;    ///< The type of error.
    std::string        message;  ///< A detailed error message.

    /**
     * @brief Conversion operator to std::string.
     *
     * Converts the EmbedYAMLError instance into a string representation that
     * combines the error type and the error message.
     *
     * @return A string formatted as "ErrorType: message".
     */
    operator std::string() const { return EmbedYAMLErrorTypeToString[static_cast<size_t>(error)] + ": " + message; }

    /**
     * @brief Stream insertion operator for EmbedYAMLError.
     *
     * Enables outputting the error details directly to an output stream.
     *
     * @param os The output stream.
     * @param error The EmbedYAMLError instance to output.
     * @return The output stream with the error details appended.
     */
    friend std::ostream& operator<<(std::ostream& os, const EmbedYAMLError& error)
    {
        os << static_cast<std::string>(error);
        return os;
    }
};

#include "EmbedYAML/EmbedYAML.hpp"

namespace EmbedYAML {

EmbedYAML::EmbedYAML(EYFileOpenFunction open, EYFileCloseFunction close, EYReadCharFunction read_char, void* user_context)
    : m_file_open_function(open), m_file_close_function(close), m_read_char_function(read_char), m_user_context(user_context)
{
}

EmbedYAML::~EmbedYAML()
{
}

std::vector<Token> EmbedYAML::parseFile(std::string filename)
{
    if (m_file_open_function(this, filename) < 0)
    {
        return std::vector<Token>{Token{TokenType::ERROR, "Failed to open file"}};
    }

    std::vector<Token> tokens;
    int current_indent_level = 0;
    bool is_new_line = true;

    while (true)
    {
        auto c = m_read_char_function(this);

        if (!c.has_value())
        {
            tokens.emplace_back(Token{TokenType::EOF_TOKEN, ""});
            break;
        }

        if (c.value() == '\n')
        {
            tokens.emplace_back(Token{TokenType::EOL, ""});
            is_new_line = true;
            continue;
        }

        if (is_new_line && (c.value() == ' ' || c.value() == '\t'))
        {
            int indent_count = 0;

            // Count indentation level at the beginning of a line
            while (c.has_value() && (c.value() == ' ' || c.value() == '\t'))
            {
                indent_count++;
                c = m_read_char_function(this);
            }

            // Register the indentation token based on the number of spaces
            if (indent_count > current_indent_level)
            {
                tokens.emplace_back(Token{TokenType::INDENT, ""});
                current_indent_level = indent_count;
            }

            // Continue processing after indentation
            if (!c.has_value())
            {
                tokens.emplace_back(Token{TokenType::EOF_TOKEN, ""});
                break;
            }
            is_new_line = false;
        }

        if (c.value() == ':')
        {
            tokens.emplace_back(Token{TokenType::COLON, ""});
            continue;
        }

        if (c.value() == '-')
        {
            tokens.emplace_back(Token{TokenType::HYPHEN, ""});
            continue;
        }

        // Handle strings and numbers
        if (std::isalnum(c.value()) || c.value() == '_')
        {
            std::string value;
            value.push_back(c.value());

            while (true)
            {
                c = m_read_char_function(this);
                if (!c.has_value() || c.value() == ' ' || c.value() == ':' || c.value() == '\n' || c.value() == '-')
                {
                    // Put back the last character if needed (not consumed yet)
                    if (c.has_value() && c.value() != '\n')
                    {
                        // You can cache the character for the next iteration
                    }

                    // Determine if it's a number or string
                    if (std::all_of(value.begin(), value.end(), ::isdigit))
                    {
                        tokens.emplace_back(Token{TokenType::NUMBER, value});
                    }
                    else
                    {
                        tokens.emplace_back(Token{TokenType::STRING, value});
                    }
                    break;
                }
                value.push_back(c.value());
            }
        }
    }

    if (m_file_close_function(this, filename) < 0)
    {
        tokens.emplace_back(Token{TokenType::ERROR, "Failed to close file"});
    }

    return tokens;
}


} // namespace EmbedYAML
#include "EmbedYAML/EmbedYAML.hpp"

namespace EmbedYAML {

EmbedYAML::EmbedYAML(EYFileOpenFunction open,
                     EYFileCloseFunction close,
                     EYReadCharFunction read_char,
                     void* user_context)
    : m_file_open_function(open),
      m_file_close_function(close),
      m_read_char_function(read_char),
      m_user_context(user_context)
{
}

EmbedYAML::~EmbedYAML()
{
}

bool EmbedYAML::parseFile(std::string filename)
{
    if (m_file_open_function(this, filename) < 0)
        return false;

    while (true)
    {
        auto c = m_read_char_function(this);
        if (c.has_value())
            printf("%c", c.value());
        else
            break;
    }
    printf("\n");

    if (m_file_close_function(this, filename) < 0)
        return false;

    return true;
}

} // namespace EmbedYAML

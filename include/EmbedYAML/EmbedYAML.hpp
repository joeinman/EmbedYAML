#pragma once

#include <EmbedYAML/YAMLNode.hpp>
#include <functional>
#include <optional>
#include <vector>
#include <string>
#include <yaml.h>
#include <stack>

namespace EmbedYAML {

class EmbedYAML;

using EYFileOpenFunction = std::function<int(EmbedYAML*, std::string)>;
using EYFileCloseFunction = std::function<int(EmbedYAML*, std::string)>;
using EYReadCharFunction = std::function<std::optional<char>(EmbedYAML*)>;

class EmbedYAML {
public:
    EmbedYAML(EYFileOpenFunction open, EYFileCloseFunction close, EYReadCharFunction read_char, void* user_context = nullptr);
    ~EmbedYAML();

    YAMLNode parseFile(std::string filename);

    void* getUserContext() const { return m_user_context; }
private:
    EYFileOpenFunction m_file_open_function;
    EYFileCloseFunction m_file_close_function;
    EYReadCharFunction m_read_char_function;

    // Allow user context variable
    void* m_user_context;
};

} // namespace EmbedYAML
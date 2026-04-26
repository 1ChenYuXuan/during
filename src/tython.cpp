// Copyright (c) 2026 Chen Yuxuan
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense,/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#include "./define.hpp"
#include "./ast.hpp"

#include <fstream>
#include <iostream>
#include <string>
#include <vector>
#include <cstddef>
#include <cctype>
#include <cstdlib>
#include <algorithm>

using Tokens = std::vector<Token>;

class _TokenError : public Exception {
public:
    _TokenError(const std::string& msg, size_t line) {
        this->name = "TokenError";
        this->msg = msg;
        this->line = line;
        this->throws();
    }
};

#define TokenError(string) _TokenError(string, __LINE__)

static inline auto printAllTokens(const Tokens& tokens) -> void {
    for (const Token& token : tokens) {
        std::cout << "Type: " << token.head.type << "\n";
        std::cout << "Str : " << token.str << "\n\n";
    }
    std::cout << std::flush;
}

static inline auto readAll(const std::string& filename) -> std::string {
    std::ifstream ifs(filename);
    if (!ifs) {
        std::cerr << "Failed to open file: " << filename << std::endl;
        exit(EXIT_FAILURE);
    }
    return std::string(std::istreambuf_iterator<char>(ifs),
                       std::istreambuf_iterator<char>());
}

static inline auto isDigits(const std::string& str) -> bool {
    if (str.empty()) return false;
    for (char c : str)
        if (!isdigit(static_cast<unsigned char>(c)))
            return false;
    return true;
}

static inline auto isIderCharExceptNumber(const char c) -> bool {
    return (('A' <= c && c <= 'Z') || ('a' <= c && c <= 'z') ||
            c == '_' || c == '<' || c == '>' || c == '=' || c == '$');
}

static inline auto isIderChar(const char c) -> bool {
    return isIderCharExceptNumber(c) || isdigit(static_cast<unsigned char>(c));
}

static inline auto isIdentifier(const std::string& str) -> bool {
    if (str.empty()) return false;
    if (!isIderCharExceptNumber(str[0]))
        return false;
    for (size_t i = 1; i < str.size(); ++i)
        if (!isIderChar(str[i]))
            return false;
    return true;
}

static inline auto isWhite(const char c) -> bool {
    return isspace(static_cast<unsigned char>(c));
}

static inline auto isWhites(const std::string& str) -> bool {
    for (char c : str)
        if (!isWhite(c))
            return false;
    return true;
}

static inline auto upper(std::string& str) -> void {
    std::transform(str.begin(), str.end(), str.begin(), toupper);
}

static inline auto typeToken(const std::string& str) -> Type {
    if (str.size() < 9) { // a small optimization for most tokens
        std::string newString = str;
        upper(newString);
        if (newString == "IF") return IF;
        if (newString == "ELSE") return ELSE;
        if (newString == "END") return END;
        if (newString == "BIGGER") return BIGGER;
        if (newString == "SMALLER") return SMALLER;
        if (newString == "EQUALS") return EQUALS;
        if (newString == "EQUAL") return EQUAL;
        if (newString == "AND") return AND;
        if (newString == "OR") return OR;
        if (newString == "NOT") return NOT;

        if (newString == "ADD") return ADD;
        if (newString == "SUB") return SUB;
        if (newString == "MUL") return MUL;
        if (newString == "DIV") return DIV;
        if (newString == "FLOORDIV") return FLOORDIV;
        if (newString == "MOD") return MOD;

        if (newString == "INT") return INT;
        if (newString == "CHAR") return CHAR;
        if (newString == "OVER") return OVER;
    }

    if (isDigits(str)) return DIGITS;
    if (isIdentifier(str)) return IDENTIFIER;

    return UNKNOWN;
}

static inline auto lexer(const std::string& str) -> Tokens {
    Tokens tokens;
    size_t line = 1;
    std::string buffer;

    for (char c : str) {
        if (c == '\n') {
            line++;
            continue;
        }
        if (isWhite(c)) {
            if (!buffer.empty()) {
                tokens.push_back({typeToken(buffer), buffer});
                buffer.clear();
            }
            continue;
        }
        buffer += c;
    }

    if (!buffer.empty())
        tokens.push_back({typeToken(buffer), buffer});

    return tokens;
}

static inline auto parse(const Tokens& tokens) -> AST {
    struct Parser {
        const Tokens& tokens;
        size_t pos = 0;

        auto match(Type type) -> bool {
            if (pos >= tokens.size())
                return false;
            if (tokens[pos].head.type == type) {
                pos++;
                return true;
            }
            return false;
        }

        auto skip(Type type) -> void {
            while (match(type));
        }

        auto term() -> AST {
            if (match(DIGITS)) {
                size_t num_pos = pos - 1;
                
                if (match(MUL)) {
                    return AST(HeadType{MUL}, "MUL", {
                        AST(HeadType{DIGITS}, tokens[num_pos].str, {}),
                        term()
                    });
                }
                if (match(DIV)) {
                    return AST(HeadType{DIV}, "DIV", {
                        AST(HeadType{DIGITS}, tokens[num_pos].str, {}),
                        term()
                    });
                }
                if (match(ADD)) {
                    return AST(HeadType{ADD}, "ADD", {
                        AST(HeadType{DIGITS}, tokens[num_pos].str, {}),
                        term()
                    });
                }
                if (match(SUB)) {
                    return AST(HeadType{SUB}, "SUB", {
                        AST(HeadType{DIGITS}, tokens[num_pos].str, {}),
                        term()
                    });
                }
                if (match(FLOORDIV)) {
                    return AST(HeadType{FLOORDIV}, "FLOORDIV", {
                        AST(HeadType{DIGITS}, tokens[num_pos].str, {}),
                        term()
                    });
                }
                if (match(MOD)) {
                    return AST(HeadType{MOD}, "MOD", {
                        AST(HeadType{DIGITS}, tokens[num_pos].str, {}),
                        term()
                    });
                }

                return AST(HeadType{DIGITS}, tokens[num_pos].str, {});
            }

            if (match(IDENTIFIER))
                return AST(HeadType{IDENTIFIER}, tokens[pos - 1].str, {});
            if (match(INT)) {
                if (!match(IDENTIFIER)) {
                    throw TokenError("Expected identifier after INT");
                }
                std::string var_name = tokens[pos - 1].str;

                // int a = 123
                if (match(EQUALS)) {
                    AST expr = term();
                    if (match(OVER)) {
                        return AST(HeadType{EQUALS}, "INT_ASSIGN", {
                            AST(HeadType{IDENTIFIER}, var_name, {}),
                            expr
                        });
                    }
                }

                // int a;
                if (match(OVER)) {
                    return AST(HeadType{EQUALS}, "INT_DECL", {
                        AST(HeadType{IDENTIFIER}, var_name, {})
                    });
                }

                throw TokenError("Expected end of statement after INT definition");
            }

            throw TokenError("Unexpected token: " + tokens[pos].str);
        }

        auto expr() -> AST {
            return term();
        }
    };

    Parser parser{tokens};
    return parser.expr();
}

auto compiler(const std::string& str) -> std::string {
    Tokens tokens = lexer(str);
    printAllTokens(tokens);
    AST ast = parse(tokens);
    std::cout << std::string(ast) << std::endl;
    std::cout << ast.cstring() << std::endl;
    return "Compile finished.\n";
}

auto usage(const std::string& path) -> void {
    std::cout << "Usage: " << path << " -c input.txt\n";
    exit(EXIT_FAILURE);
}

auto main(int argc, char** _argv) -> int {
    std::vector<std::string> argv;
    for (size_t index = 0; index < argc; ++index)
        argv.push_back(std::string(_argv[index]));

    if (argv.size() < 2)
        usage(argv[0]);

    bool isCompile = false;
    std::string inputFile;

    for (size_t index = 1; index < argv.size(); ++index) {
        auto& str = argv[index];
        if (str == "-c")
            isCompile = true;
        else
            inputFile = str;
    }

    if (inputFile.empty())
        usage(argv[0]);

    std::string str = readAll(inputFile);
    if (isCompile) {
        compiler(str);
    } else
        TODO("Else");

    return 0;
}
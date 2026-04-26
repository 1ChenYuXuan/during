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

#include <set>
#include <cstring>
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

class _FileError : public Exception {
public:
    _FileError(const std::string& msg, size_t line) {
        this->name = "FileError";
        this->msg = msg;
        this->line = line;
        this->throws();
    }
};

#define TokenError(string) _TokenError(string, __LINE__)

#define FileError(string) _FileError(string, __LINE__)

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

static inline auto saveAll(const std::string& filepath, const std::string& content) -> void {
    std::ofstream ofs(filepath);
    if (!ofs)
        throw FileError("Failed to open file: " + filepath);
    ofs << content;
}

static inline auto isDigits(const std::string& str) -> bool {
    if (str.empty()) return false;
    for (char c : str)
        if (!isdigit(static_cast<unsigned char>(c)))
            return false;
    return true;
}

static inline auto isIderCharExceptNumber(const char c) -> bool {
    return ('A' <= c && c <= 'Z') || ('a' <= c && c <= 'z') ||
            c == '_';
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
    // printf("%s\n", str.c_str());

    if (str.size() < 9) { // a small optimization for most tokens
        std::string newString = str;
        upper(newString);
        if (newString == "IF") return IF;
        if (newString == "ELSE") return ELSE;
        if (newString == "END") return END;
        if (newString == "BIGGER") return BIGGER;
        if (newString == "SMALLER") return SMALLER;
        if (newString == "EQUAL") return EQUAL;
        if (newString == "RETURN") return RETURN;

        if (newString == "(") return LEFT_PAREN;
        if (newString == ")") return RIGHT_PAREN;
        if (newString == "{") return LEFT_BRACE;
        if (newString == "}") return RIGHT_BRACE;
        if (newString == ";") return SEMICOLON;
        if (newString == ",") return COMMA;

        if (newString == "+") return ADD;
        if (newString == "-") return SUB;
        if (newString == "*") return MUL;
        if (newString == "/") return DIV;
        if (newString == "%") return MOD;
        if (newString == "@") return AT;
        if (newString == "==") return EQUAL;
        if (newString == "&&") return AND;
        if (newString == "||") return OR;
        if (newString == "!") return NOT;
        if (newString == "=") return EQUALS;
        if (newString == "!=") return NOT_EQUAL;
        if (newString == ">=") return BIGGER_EQUAL;
        if (newString == "<=") return SMALLER_EQUAL;

        if (newString == "|") return EACH_OR;
        if (newString == "~") return EACH_NOT;
        if (newString == "&") return EACH_AND;
        if (newString == "^") return EACH_XOR;

        if (newString == "FUNCTION") return FUNCTION;
    }

    if (isDigits(str)) return DIGITS;
    // printf("%s\n", str.c_str());
    if (isIdentifier(str)) return IDENTIFIER;
    // printf("%s\n", str.c_str());

    return UNKNOWN;
}

static inline auto lexer(const std::string& str) -> Tokens {
    Tokens tokens;
#ifdef NEED_LINE
    std::size_t line = 1;
#endif
    std::string buffer;
    buffer.reserve(64);
    tokens.reserve(str.size() / 10 + 10);

    auto it = str.cbegin();
    const auto end = str.cend();

    for (; it != end; ++it) {
        char c = *it;
        if (c == '\n') {
#ifdef NEED_LINE
            ++line;
#endif
            if (!buffer.empty()) {
                tokens.emplace_back(Token{typeToken(buffer), buffer});
                buffer.clear();
            }
            continue;
        }
        if (isWhite(c)) {
            if (!buffer.empty()) {
                tokens.emplace_back(Token{typeToken(buffer), buffer});
                buffer.clear();
            }
            continue;
        }

        if (it + 1 != end) {
            char nc = *(it + 1);
            switch (c) {
                case '=':
                    if (nc == '=') {
                        if (!buffer.empty()) { tokens.emplace_back(Token{typeToken(buffer), buffer}); buffer.clear(); }
                        tokens.emplace_back(Token{typeToken("=="), "=="});
                        ++it;
                        continue;
                    }
                    break;
                case '!':
                    if (nc == '=') {
                        if (!buffer.empty()) { tokens.emplace_back(Token{typeToken(buffer), buffer}); buffer.clear(); }
                        tokens.emplace_back(Token{typeToken("!="), "!="});
                        ++it;
                        continue;
                    }
                    break;
                case '&':
                    if (nc == '&') {
                        if (!buffer.empty()) { tokens.emplace_back(Token{typeToken(buffer), buffer}); buffer.clear(); }
                        tokens.emplace_back(Token{typeToken("&&"), "&&"});
                        ++it;
                        continue;
                    }
                    break;
                case '|':
                    if (nc == '|') {
                        if (!buffer.empty()) { tokens.emplace_back(Token{typeToken(buffer), buffer}); buffer.clear(); }
                        tokens.emplace_back(Token{typeToken("||"), "||"});
                        ++it;
                        continue;
                    }
                    break;
                case '+': case '-': case '*': case '/': case '%':
                    if (nc == '=') {
                        std::string op(1, c); op += '=';
                        if (!buffer.empty()) { tokens.emplace_back(Token{typeToken(buffer), buffer}); buffer.clear(); }
                        tokens.emplace_back(Token{typeToken(op), op});
                        ++it;
                        continue;
                    }
                    break;
                default: break;
            }
        }

        static const char* singleOps = "(){};,+-*/@=%&|!";
        if (strchr(singleOps, c)) {
            if (c == '/' && it + 1 != end && *(it + 1) == '/') {
                while (it != end && *it != '\n') ++it;
                continue;
            }
            if (!buffer.empty()) {
                tokens.emplace_back(Token{typeToken(buffer), buffer});
                buffer.clear();
            }
            tokens.emplace_back(Token{typeToken(std::string(1, c)), std::string(1, c)});
            continue;
        }
        buffer += c;
    }
    if (!buffer.empty())
        tokens.emplace_back(Token{typeToken(buffer), buffer});

    return tokens;
}

static inline auto parse(const Tokens& tokens) -> std::string {
    struct Parser {
        const Tokens& tokens;
        size_t pos = 0;
        std::set<std::string> functionNames;

        auto match(Type type) -> bool {
            if (pos >= tokens.size())
                return false;
            if (tokens[pos].head.type == type) {
                pos++;
                return true;
            }
            return false;
        }

        auto check(Type type) -> bool {
            if (pos >= tokens.size())
                return false;
            return tokens[pos].head.type == type;
        }

        auto getOp() -> Type {
            if (checkOp())
                return tokens[pos].head.type;
            throw TokenError("Except an op there");
        }

        auto checkOp() -> bool {
            return  check(ADD)          || check(SUB)           || check(MUL)           || check(DIV)       || check(MOD) ||
                    check(BIGGER)       || check(SMALLER)       || check(AND)           || check(OR)        || check(NOT) ||
                    check(EQUAL)        || check(BIGGER_EQUAL)  || check(SMALLER_EQUAL) || check(NOT_EQUAL) ||
                    check(EACH_NOT)     || check(EACH_AND)      || check(EACH_OR)       || check(EACH_XOR);
        }

        auto matchOp() -> bool {
            return  match(ADD)          || match(SUB)           || match(MUL)           || match(DIV)       || match(MOD) ||
                    match(BIGGER)       || match(SMALLER)       || match(AND)           || match(OR)        || match(NOT) ||
                    match(EQUAL)        || match(BIGGER_EQUAL)  || match(SMALLER_EQUAL) || match(NOT_EQUAL) ||
                    match(EACH_NOT)     || match(EACH_AND)      || match(EACH_OR)       || match(EACH_XOR);
        }

        auto term() -> AST {
            if (match(DIGITS)) {
                std::string num = tokens[pos - 1].str;
                AST num_node = AST(HeadType{DIGITS}, num, {});
                
                if (checkOp()) {
                    Type op = getOp();
                    pos++;
                    return AST(HeadType{op}, "", {
                        num_node,
                        term()
                    });
                }
                return num_node;
            }

            if (match(FUNCTION))
                return parseFunction();

            if (match(RETURN)) {
                auto ret = AST(HeadType{RETURN}, "", {term()});
                match(SEMICOLON);
                return ret;
            }

            if (match(IDENTIFIER)) {
                std::string id_str = tokens[pos - 1].str;
                AST id_node = AST(HeadType{IDENTIFIER}, id_str, {});

                if (checkOp()) {
                    Type op = getOp();
                    pos++;
                    return AST(HeadType{op}, "", {
                        id_node,
                        term()
                    });
                }

                if (match(LEFT_PAREN)) {
                    std::vector<AST> params;
                    if (!check(RIGHT_PAREN)) {
                        do {
                            params.push_back(term());
                        } while (match(COMMA));
                    }
                    if (!match(RIGHT_PAREN))
                        throw TokenError("Expected ')'");

                    match(SEMICOLON);

                    return AST(HeadType{FUNCTIONCALL}, id_str, {
                        AST(HeadType{IDENTIFIER}, id_str, {}),
                        AST(PARAMS, "", params)
                    });
                }

                if (check(IDENTIFIER)) {
                    std::string type = tokens[pos].str;
                    pos++;
                    std::string name = id_str;

                    if (match(EQUALS)) {
                        AST expr = term();
                        match(SEMICOLON);
                        return AST(HeadType{EQUALS}, type, {
                            AST(HeadType{IDENTIFIER}, name, {}),
                            expr
                        });
                    }

                    if (match(SEMICOLON)) {
                        return AST(HeadType{EQUALS}, type, {
                            AST(HeadType{IDENTIFIER}, name, {})
                        });
                    }

                    throw TokenError("Expected '=' or ';' in variable declaration");
                }

                return id_node;
            }

            if (match(LEFT_PAREN)) {
                AST node = AST(LEFT_PAREN, "", {term()});
                if (!match(RIGHT_PAREN))
                    throw TokenError("Expected ')'");

                if (checkOp()) {
                    Type op = getOp();
                    pos++;
                    return AST(HeadType{op}, "", { node, term() });
                }
                return node;
            }

            if (match(IF)) {
                if (!match(LEFT_PAREN))
                    throw TokenError("Expected '('");
                AST expr = term();
                if (!match(RIGHT_PAREN))
                    throw TokenError("Expected ')'");
                if (!match(LEFT_BRACE))
                    throw TokenError("Expected '{'");
                std::vector<AST> body;
                while (!check(RIGHT_BRACE) && pos < tokens.size())
                    body.push_back(term());
                if (!match(RIGHT_BRACE))
                    throw TokenError("Expected '}'");
                return AST(
                    IF, "", {
                        AST(EXPR, "", {expr}),
                        AST(BODY, "", body)
                    }
                );
            }

            if (match(ELSE)) {
                if (!match(RIGHT_PAREN))
                    throw TokenError("Expected ')'");
                if (!match(LEFT_BRACE))
                    throw TokenError("Expected '{'");
                std::vector<AST> body;
                while (!check(RIGHT_BRACE) && pos < tokens.size())
                    body.push_back(term());
                if (!match(RIGHT_BRACE))
                    throw TokenError("Expected '}'");
                return AST(
                    ELSE, "", {
                        AST(BODY, "", body)
                    }
                );
            }

            if (match(SEMICOLON))
                return AST(HeadType{IDENTIFIER}, "", {});

            throw TokenError("Unexpected token: " + tokens[pos].str);
        }

        auto parseFunction() -> AST {
            if (!match(IDENTIFIER))
                throw TokenError("Expected function name");
            std::string func_name = tokens[pos-1].str;
            std::string return_type = "void";
            if (functionNames.count(func_name))
                throw ASTError("Cannot define same functions.");
            functionNames.insert(func_name);

            if (!match(LEFT_PAREN))
                throw TokenError("Expected '('");

            std::vector<AST> params;
            std::vector<AST> types;
            std::vector<AST> body;

            if (!check(RIGHT_PAREN)) {
                do {
                    if (!match(IDENTIFIER))
                        throw TokenError("Expected parameter type");
                    std::string param_type = tokens[pos-1].str;
                    types.emplace_back(AST(HeadType{IDENTIFIER}, param_type, {}));

                    if (!match(IDENTIFIER))
                        throw TokenError("Expected parameter name");
                    params.emplace_back(HeadType{IDENTIFIER}, tokens[pos-1].str, ASTs{});

                } while (match(COMMA));
            }

            if (!match(RIGHT_PAREN))
                throw TokenError("Expected ')'");

            if (match(AT)) {
                if (!match(IDENTIFIER))
                    throw TokenError("Expected return type after @");
                return_type = tokens[pos-1].str;
            }

            if (!match(LEFT_BRACE))
                throw TokenError("Expected '{'");

            while (!check(RIGHT_BRACE) && pos < tokens.size())
                body.push_back(term());

            if (!match(RIGHT_BRACE))
                throw TokenError("Expected '}'");

            return AST(HeadType{FUNCTION}, func_name, {
                AST(HeadType{PARAMS}, "", params),
                AST(HeadType{BODY}, "", body),
                AST(HeadType{TYPES}, "", types),
                AST(HeadType{IDENTIFIER}, return_type, {})
            });
        }
    };

    std::string code;
    Parser parser{tokens};

    while (parser.pos < tokens.size()) {
        try {
            AST term = parser.term();
            code += term.cstring() + "\n";
        } catch (const _TokenError& e) {
            code += "/* Parse Error:\n" + std::string(e.what()) + "\n*/\n";
            break;
        }
    }

    return code;
}

auto compiler(const std::string& str) -> std::string {
    Tokens tokens = lexer(str);
    // printAllTokens(tokens);
    std::string astString = parse(tokens);
    std::cout << astString << std::endl;
    return astString;
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
    std::string outputFile = "a.out";

    for (size_t index = 1; index < argv.size(); ++index) {
        auto& str = argv[index];
        if (str == "-c")
            isCompile = true;
        if (str == "-o") {
            if (index + 1 >= argv.size()) {
                std::cerr << "Expected output file after -o\n";
                usage(argv[0]);
            }
            outputFile = argv[++index];
        }
        else
            inputFile = str;
    }

    if (inputFile.empty())
        usage(argv[0]);

    std::string str = readAll(inputFile);
    if (isCompile) {
        std::string string = compiler(str);
        saveAll(outputFile, string);
    } else
        TODO("Else");

    return 0;
}
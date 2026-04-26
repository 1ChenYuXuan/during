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
        if (newString == "=") return EQUALS;
        if (newString == "&") return AND;
        if (newString == "|") return OR;
        if (newString == "!") return NOT;
        if (newString == "=") return EQUALS;

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
    size_t line = 1;
#endif
    std::string buffer;
    buffer.reserve(64); // a small optimization for most tokens
    tokens.reserve(str.size() / 10 + 10); // a small optimization for most tokens
    
    for (char c : str) {
        if (c == '\n') {
#ifdef NEED_LINE
            line++;
#endif
            continue;
        }
        if (isWhite(c)) {
            if (!buffer.empty()) {
                tokens.push_back({typeToken(buffer), buffer});
                buffer.clear();
            }
            continue;
        } if (c == '(' || c == ')' || c == '{' || c == '}' || c == ';' || c == ',' || c == '+' || c == '-' || c == '*' || c == '/' || c == '@' || c == '=' || c == '%' || c == '&' || c == '|' || c == '!') {
            if (!buffer.empty()) {
                tokens.push_back({typeToken(buffer), buffer});
                buffer.clear();
            }
            tokens.push_back({typeToken(std::string(1, c)), std::string(1, c)});
            continue;
        }
        buffer += c;
    }

    if (!buffer.empty())
        tokens.push_back({typeToken(buffer), buffer});

    return tokens;
}

static inline auto parse(const Tokens& tokens) -> std::string {
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

        auto check(Type type) -> bool {
            if (pos >= tokens.size())
                return false;
            return tokens[pos].head.type == type;
        }

        auto term() -> AST {
            if (match(DIGITS)) {
                std::string num = tokens[pos - 1].str;
                AST num_node = AST(HeadType{DIGITS}, num, {});
                if (check(ADD) || check(SUB) || check(MUL) || check(DIV) || check(MOD) ||
                        check(BIGGER) || check(SMALLER) || check(AND) || check(OR) || check(NOT) || check(EQUAL)) {
                    Type op = tokens[pos].head.type;
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
            if (match(RETURN))
                return AST(HeadType{RETURN}, "", {term()});
            if (match(IDENTIFIER)) {
                std::string num = tokens[pos - 1].str;
                AST num_node = AST(HeadType{IDENTIFIER}, num, {});
                if (check(ADD) || check(SUB) || check(MUL) || check(DIV) || check(MOD) ||
                        check(BIGGER) || check(SMALLER) || check(AND) || check(OR) || check(NOT) || check(EQUAL)) {
                    Type op = tokens[pos].head.type;
                    pos++;
                    return AST(HeadType{op}, "", {
                        num_node,
                        term()
                    });
                }
                if (match(IDENTIFIER)) {
                    std::string type = tokens[pos - 1].str;
                    // pos++;
                    // printf("Type: %d\n", tokens[pos].head.type);
                    std::string name = tokens[pos - 2].str;
                    
                    if (match(SEMICOLON))
                        return AST(HeadType{EQUALS}, type, {
                            AST(HeadType{IDENTIFIER}, name, {})
                        });

                    if (!match(EQUALS))
                        throw TokenError("Expected '=' after type in variable declaration");

                    // printAllTokens(tokens);
                    AST expr = term();
                    return AST(HeadType{EQUALS}, type, {
                        AST(HeadType{IDENTIFIER}, name, {}),
                        expr
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
                    // pos++;
                    return AST(HeadType{FUNCTIONCALL}, tokens[pos - 2].str, {
                        AST(HeadType{IDENTIFIER}, num, {}),
                        AST(PARAMS, "", params)
                    });
                }
                return num_node;
            }
            if (match(LEFT_PAREN)) {
                AST node = term();
                if (!match(RIGHT_PAREN))
                    throw TokenError("Expected ')'");
                return node;
            }
            if (match(SEMICOLON))
                return AST(HeadType{SEMICOLON}, "", {});
            throw TokenError("Unexpected token: " + tokens[pos].str);
        }

        auto parseFunction() -> AST {
            if (!match(IDENTIFIER))
                throw TokenError("Expected function name");
            std::string func_name = tokens[pos-1].str;
            std::string type = "void";
            if (!match(LEFT_PAREN))
                throw TokenError("Expected '('");
            std::vector<AST> params, types, body;
            if (!match(RIGHT_PAREN)) {
                do {
                    match(IDENTIFIER);
                    std::string type = tokens[pos-1].str;
                    types.emplace_back(AST(HeadType{IDENTIFIER}, type, {}));
                    // std::printf("%d\n", tokens[pos].head.type);

                    if (!match(IDENTIFIER))
                        throw TokenError("Expected parameter name");
                    params.emplace_back(HeadType{IDENTIFIER}, tokens[pos-1].str, ASTs{});

                } while (match(COMMA));

                if (!match(RIGHT_PAREN))
                    throw TokenError("Expected ')'");
            }
            if (match(AT))
                type = match(IDENTIFIER) ? tokens[pos-1].str : "void";
            // printf("%d\n", tokens[pos-1].head.type);
            if (!match(LEFT_BRACE))
                throw TokenError("Expected '{'");
            while (!check(RIGHT_BRACE))
                body.push_back(term());
            match(RIGHT_BRACE);
            return AST(HeadType{FUNCTION}, func_name, {
                AST(HeadType{PARAMS}, "", params),
                AST(HeadType{BODY}, "", body),
                AST(HeadType{TYPES}, "", types),
                AST(HeadType{IDENTIFIER}, type, {})
            });
        }
    };
    std::string code;
    Parser parser{tokens};
    // It is that Python can teach me:
    // while (true) {
    //     try {
    //         code += parser.term().cstring() + "\n";
    //     } catch(...) {
    //         break;
    //     }
    // }
    // Who create for-loop to try-except in Python? It is too wried and slow.
    while (parser.pos <= (tokens.size() - 1)) {
        AST term = parser.term();
        // printf("Term: %s\n", term.operator std::string().c_str());
        code += term.cstring() + "\n";
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
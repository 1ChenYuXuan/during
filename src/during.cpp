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
        if (newString == "INCLUDE") return INCLUDE;
        if (newString == "WHILE") return WHILE;
        if (newString == "WHILE") return WHILE;
        if (newString == "IMPORT") return IMPORT;
        if (newString == "CLASS") return CLASS;

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
        if (newString == ">") return BIGGER;
        if (newString == "<") return SMALLER;

        if (newString == "|") return EACH_OR;
        if (newString == "~") return EACH_NOT;
        if (newString == "&") return EACH_AND;
        if (newString == "^") return EACH_XOR;

        if (newString == "+=") return ADDSELF;
        if (newString == "-=") return SUBSELF;
        if (newString == "*=") return MULSELF;
        if (newString == "/=") return DIVSELF;
        if (newString == "%=") return MODSELF;

        if (newString == "|=") return EACH_ORSELF;
        if (newString == "&=") return EACH_ANDSELF;
        if (newString == "^=") return EACH_XORSELF;

        if (newString == "FUNCTION") return FUNCTION;
    }

    if (isDigits(str)) return DIGITS;
    // printf("%s\n", str.c_str());
    if (isIdentifier(str)) return IDENTIFIER;
    // printf("%s\n", str.c_str());

    return UNKNOWN;
}

#define NEED_LINE
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

    bool isString = false;
    bool isChar = false;

    for (; it != end; ++it) {
        char c = *it;

        if (c == '\n') {
#ifdef NEED_LINE
            ++line;
#endif
            if (isString || isChar)
                throw TokenError("unclosed string/char");

            if (!buffer.empty()) {
                tokens.emplace_back(Token{typeToken(buffer), std::move(buffer), line});
                buffer.clear();
            }
            tokens.emplace_back(Token{LINESEP, "", line});
            continue;
        }

        if (isString) {
            if (c == '"') {
                tokens.emplace_back(Token{STRING, std::move(buffer), line});
                buffer.clear();
                isString = false;
                continue;
            }
            if (c == '\\') {
                if (std::next(it) == end)
                    throw TokenError("escape at end");
                char next_c = *std::next(it);
                if (next_c == '"') {
                    buffer += '"';
                    ++it;
                    continue;
                }
            }
            buffer += c;
            continue;
        }

        if (isChar) {
            if (c == '\'') {
                tokens.emplace_back(Token{CHAR, std::move(buffer), line});
                buffer.clear();
                isChar = false;
                continue;
            }
            if (c == '\\') {
                if (std::next(it) == end)
                    throw TokenError("escape at end");
                ++it;
            }
            buffer += c;
            continue;
        }

        if (isWhite(c)) {
            if (!buffer.empty()) {
                tokens.emplace_back(Token{typeToken(buffer), std::move(buffer), line});
                buffer.clear();
            }
            continue;
        }

        if (c == '"') {
            isString = true;
            continue;
        }
        if (c == '\'') {
            isChar = true;
            continue;
        }

        if (std::next(it) != end) {
            char nc = *std::next(it);
            bool match = false;
            std::string op;

            switch (c) {
                case '=': if (nc == '=') { op = "=="; match = true; } break;
                case '!': if (nc == '=') { op = "!="; match = true; } break;
                case '&': if (nc == '&') { op = "&&"; match = true; } break;
                case '|': if (nc == '|') { op = "||"; match = true; } else if (nc == '=') {op = "|="; match = true;} break;
                case '+': case '-': case '*': case '/': case '%': case '^':
                    if (nc == '=') { op = std::string(1, c) + '='; match = true; } break;
                default: break;
            }

            if (match) {
                if (!buffer.empty()) {
                    tokens.emplace_back(Token{typeToken(buffer), std::move(buffer), line});
                    buffer.clear();
                }
                tokens.emplace_back(Token{typeToken(op), std::move(op), line});
                ++it;
                continue;
            }
        }

        if (c == '/' && std::next(it) != end && *std::next(it) == '/') {
            while (it != end && *it != '\n') ++it;
            if (it != end) --it;
            continue;
        }

        static const char* singleOps = "(){};,+-*/@=%&|!\"'^|";
        if (std::strchr(singleOps, c)) {
            if (!buffer.empty()) {
                tokens.emplace_back(Token{typeToken(buffer), std::move(buffer), line});
                buffer.clear();
            }
            tokens.emplace_back(Token{typeToken(std::string(1, c)), std::string(1, c), line});
            continue;
        }

        buffer += c;
    }

    if (isString || isChar)
        throw TokenError("unclosed string/char at EOF");

    if (!buffer.empty())
        tokens.emplace_back(Token{typeToken(buffer), std::move(buffer), line});

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

        auto getOp() -> Type {
            if (checkOp())
                return tokens[pos].head.type;
            throw _TokenError("Except an op there", __LINE__);
        }

        auto checkOp() -> bool {
            return  check(ADD)          || check(SUB)           || check(MUL)           || check(DIV)       || check(MOD)     ||
                    check(BIGGER)       || check(SMALLER)       || check(AND)           || check(OR)        || check(NOT)     ||
                    check(EQUAL)        || check(BIGGER_EQUAL)  || check(SMALLER_EQUAL) || check(NOT_EQUAL) ||
                    check(ADDSELF)      || check(SUBSELF)       || check(MULSELF)       || check(DIVSELF)   || check(MODSELF) ||
                    check(EACH_ANDSELF) || check(EACH_ORSELF)   || check(EACH_XORSELF)  ||
                    check(EACH_NOT)     || check(EACH_AND)      || check(EACH_OR)       || check(EACH_XOR);
        }

        auto matchOp() -> bool {
            return  match(ADD)          || match(SUB)           || match(MUL)           || match(DIV)       || match(MOD)     ||
                    match(BIGGER)       || match(SMALLER)       || match(AND)           || match(OR)        || match(NOT)     ||
                    match(EQUAL)        || match(BIGGER_EQUAL)  || match(SMALLER_EQUAL) || match(NOT_EQUAL) ||
                    match(ADDSELF)      || match(SUBSELF)       || match(MULSELF)       || match(DIVSELF)   || match(MODSELF) ||
                    match(EACH_ANDSELF) || match(EACH_ORSELF)   || match(EACH_XORSELF)  ||
                    match(EACH_NOT)     || match(EACH_AND)      || match(EACH_OR)       || match(EACH_XOR);
        }

        auto parseExpr() -> AST {
            if (match(DIGITS)) {
                std::string num = tokens[pos - 1].str;
                AST num_node = AST(HeadType{DIGITS}, num, {});
                
                if (checkOp()) {
                    Type op = getOp();
                    pos++;
                    return AST(HeadType{op}, "", {
                        num_node,
                        parseExpr()
                    });
                }
                return num_node;
            }

            if (match(IDENTIFIER)) {
                std::string id_str = tokens[pos - 1].str;
                AST id_node = AST(HeadType{IDENTIFIER}, id_str, {});

                if (checkOp()) {
                    Type op = getOp();
                    pos++;
                    return AST(HeadType{op}, "", {
                        id_node,
                        parseExpr()
                    });
                }

                if (match(LEFT_PAREN)) {
                    std::vector<AST> params;
                    if (!check(RIGHT_PAREN))
                        do
                            params.push_back(parseExpr());
                        while (match(COMMA));
                    if (!match(RIGHT_PAREN))
                        throw TokenError("Expected ')'");

                    std::string str = match(SEMICOLON) ? ";" : "";

                    return AST(HeadType{FUNCTIONCALL}, id_str, {
                        AST(HeadType{IDENTIFIER}, id_str, {}),
                        AST(PARAMS, str, params)
                    });
                }

                return id_node;
            }

            if (match(LEFT_PAREN)) {
                AST node = AST(LEFT_PAREN, "", {parseExpr()});
                if (!match(RIGHT_PAREN))
                    throw TokenError("Expected ')'");

                if (checkOp()) {
                    Type op = getOp();
                    pos++;
                    return AST(HeadType{op}, "", { node, parseExpr() });
                }
                return node;
            }

            if (match(STRING))
                return AST(HeadType{STRING}, tokens[pos-1].str, {});
            if (match(CHAR))
                return AST(HeadType{STRING}, tokens[pos-1].str, {});

            throw _TokenError("Unexpected expr token: " + tokens[pos].str, __LINE__);
        }

        auto parseStmt() -> AST {
            if (match(CLASS)) {
                if (!match(IDENTIFIER))
                    throw _TokenError("Unexcept an identifier after 'class'", __LINE__);
                std::string identifier = tokens[pos - 1].str;
                if (!match(LEFT_BRACE))
                    throw _TokenError("Unexcept '{'", __LINE__);
                std::vector<AST> privates, publics, projects;
                while (!match(RIGHT_BRACE)) {
                    if ((pos + 1) >= tokens.size())
                        throw _TokenError("TODO", __LINE__);
                }
            }
            if (match(IMPORT))
                throw TODOError("TODO: import [@] xxx");
            if (match(LINESEP))
                return AST(HeadType{LINESEP}, "", {});

            if (match(INCLUDE)) {
                Type type = UNAT;
                AST ast;
                if (match(AT))
                    type = AT;
                if (match(STRING))
                    ast = AST(INCLUDE, tokens[pos - 1].str, {AST(HeadType{type}, "", {})});
                if (!match(SEMICOLON))
                    throw _TokenError("Unexcept ';' after include XXX", __LINE__);
                return ast;
            }

            if (match(FUNCTION))
                return parseFunction();

            if (match(RETURN)) {
                auto ret = AST(HeadType{RETURN}, "", {parseExpr()});
                match(SEMICOLON);
                return ret;
            }

            if (match(IDENTIFIER)) {
                std::string id_str = tokens[pos - 1].str;

                if (check(IDENTIFIER)) {
                    std::string type = tokens[pos].str;
                    pos++;
                    std::string name = id_str;

                    if (match(EQUALS)) {
                        AST expr = parseExpr();
                        if (!match(SEMICOLON))
                            throw _TokenError("Expected ';' in variable declaration", __LINE__);;
                        return AST(HeadType{EQUALS}, type, {
                            AST(HeadType{IDENTIFIER}, name, {}),
                            expr
                        });
                    }

                    if (match(SEMICOLON))
                        return AST(HeadType{EQUALS}, type, {
                            AST(HeadType{IDENTIFIER}, name, {})
                        });

                    throw _TokenError("Expected '=' or ';' in variable declaration", __LINE__);
                }

                pos--;
                return parseExpr();
            }

            if (match(IF)) {
                if (!match(LEFT_PAREN))
                    throw _TokenError("Expected '('", __LINE__);
                AST expr = parseExpr();
                if (!match(RIGHT_PAREN))
                    throw _TokenError("Expected ')'", __LINE__);
                if (!match(LEFT_BRACE))
                    throw _TokenError("Expected '{'", __LINE__);
                std::vector<AST> body;
                while (!check(RIGHT_BRACE) && pos < tokens.size())
                    body.push_back(parseStmt());
                if (!match(RIGHT_BRACE))
                    throw _TokenError("Expected '}'", __LINE__);
                return AST(
                    IF, "", {
                        AST(EXPR, "", {expr}),
                        AST(BODY, "", body)
                    }
                );
            }

            if (match(WHILE)) {
                if (!match(LEFT_PAREN))
                    throw _TokenError("Expected '('", __LINE__);
                AST expr = parseExpr();
                // printf("%d\n", tokens[pos].head.type);
                if (!match(RIGHT_PAREN))
                    throw _TokenError("Expected ')'", __LINE__);
                if (!match(LEFT_BRACE))
                    throw _TokenError("Expected '{'", __LINE__);
                std::vector<AST> body;
                while (!check(RIGHT_BRACE) && pos < tokens.size())
                    body.push_back(parseStmt());
                if (!match(RIGHT_BRACE))
                    throw _TokenError("Expected '}'", __LINE__);
                return AST(
                    WHILE, "", {
                        AST(EXPR, "", {expr}),
                        AST(BODY, "", body)
                    }
                );
            }

            if (match(ELSE)) {
                if (!match(LEFT_BRACE))
                    throw _TokenError("Expected '{'", __LINE__);
                std::vector<AST> body;
                while (!check(RIGHT_BRACE) && pos < tokens.size())
                    body.push_back(parseStmt());
                if (!match(RIGHT_BRACE))
                    throw _TokenError("Expected '}'", __LINE__);
                return AST(
                    ELSE, "", {
                        AST(BODY, "", body)
                    }
                );
            }

            if (match(ELIF)) {
                if (!match(LEFT_PAREN))
                    throw _TokenError("Expected '('", __LINE__);
                AST expr = parseExpr();
                if (!match(RIGHT_PAREN))
                    throw _TokenError("Expected ')'", __LINE__);
                if (!match(LEFT_BRACE))
                    throw _TokenError("Expected '{'", __LINE__);
                std::vector<AST> body;
                while (!check(RIGHT_BRACE) && pos < tokens.size())
                    body.push_back(parseStmt());
                if (!match(RIGHT_BRACE))
                    throw _TokenError("Expected '}'", __LINE__);
                return AST(
                    ELIF, "", {
                        AST(EXPR, "", {expr}),
                        AST(BODY, "", body)
                    }
                );
            }

            if (match(SEMICOLON))
                return AST(HeadType{IDENTIFIER}, "", {});

            return parseExpr();
        }

        auto parseFunction() -> AST {
            if (!match(IDENTIFIER))
                throw _TokenError("Expected function name", __LINE__);
            std::string func_name = tokens[pos-1].str;
            std::string return_type = "void";

            if (!match(LEFT_PAREN))
                throw TokenError("Expected '('");

            std::vector<AST> params;
            std::vector<AST> types;
            std::vector<AST> body;

            if (!check(RIGHT_PAREN)) {
                do {
                    if (!match(IDENTIFIER))
                        throw _TokenError("Expected parameter type", __LINE__);
                    std::string param_type = tokens[pos-1].str;
                    types.emplace_back(AST(HeadType{IDENTIFIER}, param_type, {}));

                    if (!match(IDENTIFIER))
                        throw _TokenError("Expected parameter name", __LINE__);
                    params.emplace_back(HeadType{IDENTIFIER}, tokens[pos-1].str, ASTs{});

                } while (match(COMMA));
            }

            if (!match(RIGHT_PAREN))
                throw _TokenError("Expected ')'", __LINE__);

            if (match(AT)) {
                if (!match(IDENTIFIER))
                    throw _TokenError("Expected return type after @", __LINE__);
                return_type = tokens[pos-1].str;
            }

            if (!match(LEFT_BRACE))
                throw _TokenError("Expected '{'", __LINE__);

            while (!check(RIGHT_BRACE) && pos < tokens.size())
                body.push_back(parseStmt());

            if (!match(RIGHT_BRACE))
                throw _TokenError("Expected '}'", __LINE__);

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
            AST term = parser.parseStmt();
            code += term.cstring();
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
    std::cout << "Usage: " << path << " -c input.txt" << std::endl;
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
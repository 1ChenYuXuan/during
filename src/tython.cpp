
// Copyright (c) 2026 Chen Yuxuan
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
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

using Tokens = std::vector<Token>;

auto printAllTokens(const Tokens& tokens) -> void {
    for (Token token : tokens) {
        std::cout << "Type: " << token.head.type << "\n";
        std::cout << "Str : " << token.str << "\n";
    }
    std::cout << std::flush;
}

auto readAll(const std::string& filename) -> std::string {
    std::ifstream ifs(filename);
    return std::string(std::istreambuf_iterator<char>(ifs),
                       std::istreambuf_iterator<char>());
}

auto isDigits(const std::string& str) -> bool {
    if (str.empty()) return false;
    for (char c : str)
        if (!isdigit(static_cast<unsigned char>(c)))
            return false;
    return true;
}

auto isLetterORUnderscore(const char c) -> bool {
    return ('A' <= c && c <= 'Z') || ('a' <= c && c <= 'z') || c == '_';
}

auto isIdentifier(const std::string& str) -> bool {
    for (char c : str)
        if (!isLetterORUnderscore(c))
            return false;
    return true;
}

auto isWhite(const char c) -> bool {
    return  c == ' '  ||
            c == '\n' ||
            c == '\t' ||
            c == '\r';
}

auto isWhites(const std::string& str) -> bool {
    for (char c : str)
        if (!isWhite(c))
            return false;
    return true;
}

auto typeToken(const std::string& str) -> Type {
    if (str == "IF") return IF;
    if (str == "ELSE") return ELSE;
    if (str == "END") return END;
    if (isDigits(str)) return DIGITS;
    if (isIdentifier(str)) return IDENTIFIER;
    return UNKNOWN;
}

auto lexer(const std::string& str) -> Tokens {
    Tokens tokens = Tokens();
    size_t line = 1;
    char c;
    std::string buffer;

    for (auto begin = str.begin(), end = str.end(); begin < end;) {
        c = *begin;

        if (c == '\n') {
            line++;
            begin++;
            continue;
        }

        if (isWhite(c)) {
            if (!buffer.empty()) {
                tokens.push_back({{typeToken(buffer)}, buffer});
                buffer.clear();
            }
            begin++;
            continue;
        }

        buffer.push_back(c);
        begin++;
    }

    if (!buffer.empty())
        tokens.push_back({{typeToken(buffer)}, buffer});

    return tokens;
}

auto parse(const Tokens& tokens) -> AST {
    return AST();
}

auto compiler(const std::string& str) -> std::string {
    Tokens tokens = lexer(str);
    AST ast = parse(tokens);
    return "";
}

auto usage(const std::string& path) -> void {
    std::cout << "Usage: " << path << " -c input.txt\n";
    exit(1);
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
        Tokens tokens = lexer(str);
        printAllTokens(tokens);
    } else
        TODO("Else");

    return 0;
}

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

#pragma once
#include "./define.hpp"

#include <string>
#include <vector>

static const auto magical = 4;

enum Type: uint16_t {
    IF, ELSE, EXPR, WHILE, ELIF,
    DIGITS,
    IDENTIFIER,
    END,
    BIGGER, SMALLER, EQUALS, EQUAL, AND, OR, NOT,
    BIGGER_EQUAL, SMALLER_EQUAL, NOT_EQUAL,
    EACH_AND, EACH_OR, EACH_NOT, EACH_XOR,
    ADD, SUB, MUL, DIV, MOD,
    EACH_ANDSELF, EACH_ORSELF, EACH_XORSELF,
    ADDSELF, SUBSELF, MULSELF, DIVSELF, MODSELF,
    FUNCTION,
    LEFT_PAREN,
    RIGHT_PAREN,
    LEFT_BRACE,
    RIGHT_BRACE,
    SEMICOLON,
    COMMA, FUNCTIONCALL,
    PARAMS, BODY, TYPES,
    AT, RETURN, UNAT,
    INCLUDE, IMPORT, CLASS,
    // ONE, TWO,
    STRING, CHAR,
    LINESEP,
    UNKNOWN
};

struct HeadType {
    Type type;
    HeadType() = default;
    HeadType(Type type) {
        this->type = type;
    }
};

struct Token {
    HeadType head;
    std::string str;
    size_t line;
};

class _ASTError : public Exception {
public:
    _ASTError(const std::string& msg, size_t line);
};

#define ASTError(string) _ASTError(string, __LINE__)

class AST {
using ASTs = std::vector<AST>;
// public: // ALL PUBLIC
private:
    HeadType head;
    std::string str;
    ASTs asts;
public:
    AST() = default;
    AST(const HeadType&, const std::string&, const ASTs&); // not constant

    auto cstring(size_t indent=0) -> std::string const;

    operator std::string() const;
    auto operator[] (size_t index) -> AST&; // for ast[i]
    auto operator* () -> HeadType&; // for ast->type
};

using ASTs = std::vector<AST>;

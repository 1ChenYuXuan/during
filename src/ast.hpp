
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

enum Type: uint16_t {
    IF, ELSE,
    DIGITS,
    IDENTIFIER,
    END, // TODO: BIGGER, SMALLER, EQUALS, EQUAL, AND, OR, NOT, END
    UNKNOWN
};

struct HeadType {
    Type type;
};

struct Token {
    HeadType head;
    std::string str;
};

class AST {
using ASTs = std::vector<AST>;
// public: // ALL PUBLIC
private:
    HeadType head;
    ASTs asts;
public:
    AST() = default;
    AST(HeadType& head, ASTs&); // not constant

    auto operator[] (size_t index) -> AST&; // for ast[i]
    auto operator* () -> HeadType&; // for ast->type
};

using ASTs = std::vector<AST>;

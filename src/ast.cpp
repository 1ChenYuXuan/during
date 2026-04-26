
// Copyright (c) 2026 Chen Yuxuan

// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:

// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.

// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#include "./ast.hpp"

_ASTError::_ASTError(const std::string& msg, size_t line) {
    this->name = "ASTError";
    this->msg = msg;
    this->line = line;
    this->throws();
}

AST::AST(const HeadType& head, const std::string& str, const ASTs& asts) {
    this->head = head;
    this->str = str;
    this->asts = asts;
}

auto AST::cstring(size_t indent) -> std::string const {
    std::string indentStr;
    for (size_t i = 0; i < indent; ++i)
        indentStr += ' ';
    switch (this->head.type) {
    case EQUALS: {
        if (this->str == "INT_DECL")
            return indentStr + "int " + this->asts[0].str + ";\n" ;
        else if (this->str == "INT_ASSIGN")
            return indentStr + "int " + this->asts[0].str + " = " + this->asts[1].cstring() + ";\n";
        else
            throw ASTError("Unknown INT AST type: " + this->str);
        // break;
    } case IDENTIFIER:
        return this->str;
        // break;
    case DIGITS:
        return this->str;
        // break;
    case ADD: 
        return indentStr + this->asts[0].cstring() + " + " + this->asts[1].cstring();
        // break;
    case SUB: 
        return indentStr + this->asts[0].cstring() + " - " + this->asts[1].cstring();
        // break;
    case MUL: 
        return indentStr + this->asts[0].cstring() + " * " + this->asts[1].cstring();
        // break;
    case DIV: 
        return indentStr + this->asts[0].cstring() + " / " + this->asts[1].cstring();
        // break;
    case FUNCTION: {
        std::string paramsStr, bodyStr;
        for (size_t index = 0; index < this->asts[0].asts.size(); ++index) {
            AST& param = this->asts[0].asts[index];
            AST& type = this->asts[2].asts[index];
            paramsStr += typeToString(type.head.type) + ' ' + param.cstring() + ", ";
        }
        if (!paramsStr.empty())
            paramsStr.pop_back(), paramsStr.pop_back(); // remove last ", "
        for (AST& stmt : this->asts[1].asts)
            bodyStr += stmt.cstring(indent + magical);
        return indentStr + typeToString(this->asts[3].head.type) + " " + this->str + "(" + paramsStr + ") {\n" + bodyStr + indentStr + "}\n";
        // break;
    }
    default:
        throw ASTError("Unknown AST type: " + std::to_string(this->head.type));
        // break;
    }
}

AST::operator std::string() const {
    std::string result = "AST(" + std::to_string(this->head.type) + ", " + this->str + ", [";
    for (const AST& ast : this->asts)
        result += std::string(ast) + ", ";
    if (!this->asts.empty())
        result.pop_back(), result.pop_back(); // remove last ", "
    result += "])";
    return result;
}

auto AST::operator[] (size_t index) -> AST& {
    return this->asts[index];
}

auto AST::operator* () -> HeadType& {
    return this->head;
}

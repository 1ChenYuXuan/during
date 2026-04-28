
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
    case LINESEP:
        return "\n";
    case EQUALS: {
        std::string type = this->asts[0].cstring();
        if (this->asts.size() == 1) {
            if (type == "reset")
                return indentStr + this->str + ";";
            return indentStr + type + " " + this->str + ";";
        } else {
            if (type == "reset")
                return indentStr + this->str + " = " + this->asts[1].cstring() + ";";
            return indentStr + type + " " + this->str + " = " + this->asts[1].cstring() + ";";
        }
        break;
    }
    case INCLUDE:
        if (this->asts[0].head.type == AT)
            return "#include \"" + this->str + "\"";
        else
            return "#include <" + this->str + ">";
        break;
    case RETURN:
        return indentStr + "return " + this->asts[0].cstring() + ";";
        break;
    case IDENTIFIER:
    case DIGITS:
        return this->str;
        break;
    case STRING:
        return "\"" + this->str + "\"";
        break;
    case CHAR:
        return "'" + this->str + "'";
        break;
    case ADD:
        return this->asts[0].cstring() + " + " + this->asts[1].cstring() ;
        break;
    case SUB:
        return this->asts[0].cstring() + " - " + this->asts[1].cstring();
        break;
    case MUL:
        return this->asts[0].cstring() + " * " + this->asts[1].cstring();
        break;
    case DIV:
        return this->asts[0].cstring() + " / " + this->asts[1].cstring();
        break;
    case MOD:
        return this->asts[0].cstring() + " % " + this->asts[1].cstring();
        break;
    case BIGGER:
        return this->asts[0].cstring() + " > " + this->asts[1].cstring();
        break;
    case SMALLER:
        return this->asts[0].cstring() + " < " + this->asts[1].cstring();
        break;
    case AND:
        return this->asts[0].cstring() + " && " + this->asts[1].cstring();
        break;
    case OR:
        return this->asts[0].cstring() + " || " + this->asts[1].cstring();
        break;
    case NOT: 
        return "!" + this->asts[0].cstring();
        break;
    case EQUAL:
        return this->asts[0].cstring() + " == " + this->asts[1].cstring();
        break;
    case NOT_EQUAL:
        return this->asts[0].cstring() + " != " + this->asts[1].cstring();
        break;
    case BIGGER_EQUAL:
        return this->asts[0].cstring() + " >= " + this->asts[1].cstring();
        break;
    case SMALLER_EQUAL:
        return this->asts[0].cstring() + " <= " + this->asts[1].cstring();
        break;
    case EACH_OR:
        return this->asts[0].cstring() + " | " + this->asts[1].cstring();
        break;
    case EACH_AND:
        return this->asts[0].cstring() + " & " + this->asts[1].cstring();
        break;
    case EACH_NOT:
        return this->asts[0].cstring() + " ~ " + this->asts[1].cstring();
        break;
    case EACH_XOR:
        return this->asts[0].cstring() + " ^" + this->asts[1].cstring();
        break;

    case ADDSELF:
        return this->asts[0].cstring() + " += " + this->asts[1].cstring();
        break;
    case SUBSELF:
        return this->asts[0].cstring() + " -= " + this->asts[1].cstring();
        break;
    case MULSELF:
        return this->asts[0].cstring() + " *= " + this->asts[1].cstring();
        break;
    case DIVSELF:
        return this->asts[0].cstring() + " /= " + this->asts[1].cstring();
        break;
    case MODSELF:
        return this->asts[0].cstring() + " %= " + this->asts[1].cstring();
        break;
    case EACH_ORSELF:
        return this->asts[0].cstring() + " |= " + this->asts[1].cstring();
        break;
    case EACH_ANDSELF:
        return this->asts[0].cstring() + " &= " + this->asts[1].cstring();
        break;
    case EACH_XORSELF:
        return (this->str == ";" ? indentStr : "") + this->asts[0].cstring() + " ^=" + this->asts[1].cstring() + this->str;
        break;
    
    case LEFT_PAREN:
        return "(" + this->asts[0].cstring() + ")";
    case IF: {
        std::string bodyStr;
        for (AST& stmt : this->asts[1].asts)
            bodyStr += stmt.cstring(indent + magical);
        return indentStr + "if (" + this->asts[0].asts[0].cstring() + ") {" + bodyStr + indentStr + "}";
    }
    case WHILE: {
        std::string bodyStr;
        for (AST& stmt : this->asts[1].asts)
            bodyStr += stmt.cstring(indent + magical);
        return indentStr + "while (" + this->asts[0].asts[0].cstring() + ") {" + bodyStr + indentStr + "}";
    }
    case ELIF: {
        std::string bodyStr;
        for (AST& stmt : this->asts[1].asts)
            bodyStr += stmt.cstring(indent + magical);
        return indentStr + "else if (" + this->asts[0].asts[0].cstring() + ") {" + bodyStr + indentStr + "}";
    }
    case ELSE: {
        std::string bodyStr;
        for (AST& stmt : this->asts[0].asts)
            bodyStr += stmt.cstring(indent + magical);
        return indentStr + "else {" + bodyStr + indentStr + "}";
    }
    case FUNCTION: {
        std::string paramsStr, bodyStr;
        for (size_t index = 0; index < this->asts[0].asts.size(); ++index) {
            AST& param = this->asts[0].asts[index];
            AST& type = this->asts[2].asts[index];
            paramsStr += type.str + ' ' + param.cstring() + ", ";
        }
        if (!paramsStr.empty())
            paramsStr.pop_back(), paramsStr.pop_back(); // remove last ", "
        for (AST& stmt : this->asts[1].asts)
            bodyStr += stmt.cstring(indent + magical);
        return indentStr +this->asts[3].str + " " + this->str + "(" + paramsStr + ") {" + bodyStr + indentStr + "}";
        break;
    }
    case FUNCTIONCALL: {
        std::string paramsStr;
        for (AST& param : this->asts[1].asts)
            paramsStr += param.cstring() + ", ";
        if (!paramsStr.empty())
            paramsStr.pop_back(), paramsStr.pop_back(); // remove last ", "
        return indentStr + this->asts[0].cstring() + "(" + paramsStr + ")" + this->asts[1].str;
        break;
    }
    case SEMICOLON:
        return ";";
        break;
    default:
        throw ASTError("Unknown AST type: " + std::to_string(this->head.type));
        break;
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

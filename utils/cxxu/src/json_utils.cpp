#include "cu_utils/json_utils.h"
#include <sstream>
#include <iomanip>
#include <cmath>
#include <stdexcept>

namespace cu {

JsonNode::JsonNode() : type_(Null) {}

JsonNode::JsonNode(bool v) : type_(Bool), bool_val_(v) {}

JsonNode::JsonNode(double v) : type_(Number), num_val_(v) {}

JsonNode::JsonNode(const std::string& v) : type_(String), str_val_(v) {}

JsonNode::JsonNode(Type type) : type_(type) {}

JsonNode::Type JsonNode::type() const { return type_; }

bool JsonNode::asBool() const { return bool_val_; }

double JsonNode::asNumber() const { return num_val_; }

const std::string& JsonNode::asString() const { return str_val_; }

JsonArray& JsonNode::asArray() { return arr_val_; }

const JsonArray& JsonNode::asArray() const { return arr_val_; }

JsonObject& JsonNode::asObject() { return obj_val_; }

const JsonObject& JsonNode::asObject() const { return obj_val_; }

std::shared_ptr<JsonNode> JsonNode::get(const std::string& key) const {
    if (type_ != Object) return nullptr;
    auto it = obj_val_.find(key);
    if (it != obj_val_.end()) return it->second;
    return nullptr;
}

std::shared_ptr<JsonNode> JsonNode::get(size_t index) const {
    if (type_ != Array || index >= arr_val_.size()) return nullptr;
    return arr_val_[index];
}

static std::string makePad(int indent, int depth) {
    if (indent < 0) return "";
    return std::string(static_cast<size_t>(indent * depth), ' ');
}

static std::string stringifyImpl(const JsonNode& node, int indent, int depth) {
    std::ostringstream oss;
    std::string nl = indent >= 0 ? "\n" : "";
    std::string pad = makePad(indent, depth);
    std::string pad1 = makePad(indent, depth + 1);
    std::string sep = indent >= 0 ? " " : "";

    switch (node.type()) {
    case JsonNode::Null:
        oss << "null";
        break;
    case JsonNode::Bool:
        oss << (node.asBool() ? "true" : "false");
        break;
    case JsonNode::Number: {
        double v = node.asNumber();
        if (std::isfinite(v) && v == std::floor(v) && std::abs(v) < 1e15) {
            oss << static_cast<long long>(v);
        } else {
            oss << std::setprecision(15) << v;
        }
        break;
    }
    case JsonNode::String:
        oss << '"';
        for (char c : node.asString()) {
            switch (c) {
            case '"':  oss << "\\\""; break;
            case '\\': oss << "\\\\"; break;
            case '\b': oss << "\\b"; break;
            case '\f': oss << "\\f"; break;
            case '\n': oss << "\\n"; break;
            case '\r': oss << "\\r"; break;
            case '\t': oss << "\\t"; break;
            default:
                if (static_cast<unsigned char>(c) < 0x20) {
                    oss << "\\u" << std::hex << std::uppercase << std::setw(4) << std::setfill('0') << static_cast<int>(static_cast<unsigned char>(c));
                } else {
                    oss << c;
                }
                break;
            }
        }
        oss << '"';
        break;
    case JsonNode::Array: {
        const JsonArray& arr = node.asArray();
        if (arr.empty()) {
            oss << "[]";
            break;
        }
        oss << '[' << nl;
        for (size_t i = 0; i < arr.size(); ++i) {
            oss << pad1 << stringifyImpl(*arr[i], indent, depth + 1);
            if (i + 1 < arr.size()) oss << ',';
            oss << sep << nl;
        }
        oss << pad << ']';
        break;
    }
    case JsonNode::Object: {
        const JsonObject& obj = node.asObject();
        if (obj.empty()) {
            oss << "{}";
            break;
        }
        oss << '{' << nl;
        size_t i = 0;
        for (const auto& [key, val] : obj) {
            oss << pad1 << '"';
            for (char c : key) {
                switch (c) {
                case '"':  oss << "\\\""; break;
                case '\\': oss << "\\\\"; break;
                case '\b': oss << "\\b"; break;
                case '\f': oss << "\\f"; break;
                case '\n': oss << "\\n"; break;
                case '\r': oss << "\\r"; break;
                case '\t': oss << "\\t"; break;
                default:   oss << c; break;
                }
            }
            oss << '"' << ':' << sep << stringifyImpl(*val, indent, depth + 1);
            if (i + 1 < obj.size()) oss << ',';
            oss << sep << nl;
            ++i;
        }
        oss << pad << '}';
        break;
    }
    }
    return oss.str();
}

std::string JsonNode::stringify(int indent) const {
    return stringifyImpl(*this, indent, 0);
}

class Parser {
public:
    Parser(const std::string& input) : input_(input), pos_(0) {}

    std::shared_ptr<JsonNode> parse() {
        skipWhitespace();
        auto node = parseValue();
        skipWhitespace();
        return node;
    }

private:
    std::string input_;
    size_t pos_;

    char peek() {
        if (pos_ >= input_.size()) return '\0';
        return input_[pos_];
    }

    char advance() {
        return input_[pos_++];
    }

    void expect(char c) {
        if (peek() != c) {
            throw std::runtime_error(std::string("expected '") + c + "' at position " + std::to_string(pos_));
        }
        advance();
    }

    void skipWhitespace() {
        while (pos_ < input_.size() && (input_[pos_] == ' ' || input_[pos_] == '\t' || input_[pos_] == '\n' || input_[pos_] == '\r')) {
            pos_++;
        }
    }

    std::shared_ptr<JsonNode> parseValue() {
        skipWhitespace();
        char c = peek();
        if (c == '"') return parseString();
        if (c == '{') return parseObject();
        if (c == '[') return parseArray();
        if (c == 't') return parseLiteral("true", true);
        if (c == 'f') return parseLiteral("false", false);
        if (c == 'n') return parseLiteral("null", JsonNode::Null);
        if (c == '-' || (c >= '0' && c <= '9')) return parseNumber();
        throw std::runtime_error("unexpected character at position " + std::to_string(pos_));
    }

    std::shared_ptr<JsonNode> parseString() {
        expect('"');
        std::string result;
        while (peek() != '"') {
            if (peek() == '\0') throw std::runtime_error("unterminated string");
            char c = advance();
            if (c == '\\') {
                c = advance();
                switch (c) {
                case '"':  result += '"'; break;
                case '\\': result += '\\'; break;
                case '/':  result += '/'; break;
                case 'b':  result += '\b'; break;
                case 'f':  result += '\f'; break;
                case 'n':  result += '\n'; break;
                case 'r':  result += '\r'; break;
                case 't':  result += '\t'; break;
                case 'u': {
                    std::string hex;
                    for (int i = 0; i < 4; ++i) {
                        hex += advance();
                    }
                    unsigned int cp = static_cast<unsigned int>(std::stoul(hex, nullptr, 16));
                    if (cp >= 0xD800 && cp <= 0xDBFF) {
                        expect('\\');
                        expect('u');
                        std::string hex2;
                        for (int i = 0; i < 4; ++i) {
                            hex2 += advance();
                        }
                        unsigned int cp2 = static_cast<unsigned int>(std::stoul(hex2, nullptr, 16));
                        if (cp2 >= 0xDC00 && cp2 <= 0xDFFF) {
                            unsigned int full = 0x10000 + ((cp - 0xD800) << 10) + (cp2 - 0xDC00);
                            result += encodeUtf8(full);
                        } else {
                            result += encodeUtf8(cp);
                            result += encodeUtf8(cp2);
                        }
                    } else {
                        result += encodeUtf8(cp);
                    }
                    break;
                }
                default:
                    result += c;
                    break;
                }
            } else {
                result += c;
            }
        }
        expect('"');
        return std::make_shared<JsonNode>(result);
    }

    static std::string encodeUtf8(unsigned int cp) {
        std::string result;
        if (cp <= 0x7F) {
            result += static_cast<char>(cp);
        } else if (cp <= 0x7FF) {
            result += static_cast<char>(0xC0 | (cp >> 6));
            result += static_cast<char>(0x80 | (cp & 0x3F));
        } else if (cp <= 0xFFFF) {
            result += static_cast<char>(0xE0 | (cp >> 12));
            result += static_cast<char>(0x80 | ((cp >> 6) & 0x3F));
            result += static_cast<char>(0x80 | (cp & 0x3F));
        } else if (cp <= 0x10FFFF) {
            result += static_cast<char>(0xF0 | (cp >> 18));
            result += static_cast<char>(0x80 | ((cp >> 12) & 0x3F));
            result += static_cast<char>(0x80 | ((cp >> 6) & 0x3F));
            result += static_cast<char>(0x80 | (cp & 0x3F));
        }
        return result;
    }

    std::shared_ptr<JsonNode> parseNumber() {
        size_t start = pos_;
        if (peek() == '-') advance();
        if (peek() == '0') {
            advance();
        } else if (peek() >= '1' && peek() <= '9') {
            while (pos_ < input_.size() && input_[pos_] >= '0' && input_[pos_] <= '9') advance();
        }
        if (peek() == '.') {
            advance();
            while (pos_ < input_.size() && input_[pos_] >= '0' && input_[pos_] <= '9') advance();
        }
        if (peek() == 'e' || peek() == 'E') {
            advance();
            if (peek() == '+' || peek() == '-') advance();
            while (pos_ < input_.size() && input_[pos_] >= '0' && input_[pos_] <= '9') advance();
        }
        std::string numStr = input_.substr(start, pos_ - start);
        double val = std::stod(numStr);
        return std::make_shared<JsonNode>(val);
    }

    std::shared_ptr<JsonNode> parseArray() {
        expect('[');
        auto node = std::make_shared<JsonNode>(JsonNode::Array);
        skipWhitespace();
        if (peek() == ']') {
            advance();
            return node;
        }
        while (true) {
            skipWhitespace();
            node->asArray().push_back(parseValue());
            skipWhitespace();
            if (peek() == ',') {
                advance();
            } else {
                break;
            }
        }
        skipWhitespace();
        expect(']');
        return node;
    }

    std::shared_ptr<JsonNode> parseObject() {
        expect('{');
        auto node = std::make_shared<JsonNode>(JsonNode::Object);
        skipWhitespace();
        if (peek() == '}') {
            advance();
            return node;
        }
        while (true) {
            skipWhitespace();
            auto keyNode = parseString();
            skipWhitespace();
            expect(':');
            skipWhitespace();
            auto valNode = parseValue();
            node->asObject()[keyNode->asString()] = valNode;
            skipWhitespace();
            if (peek() == ',') {
                advance();
            } else {
                break;
            }
        }
        skipWhitespace();
        expect('}');
        return node;
    }

    std::shared_ptr<JsonNode> parseLiteral(const char* literal, bool val) {
        for (const char* p = literal; *p; ++p) {
            if (peek() != *p) {
                throw std::runtime_error(std::string("expected '") + literal + "' at position " + std::to_string(pos_));
            }
            advance();
        }
        return std::make_shared<JsonNode>(val);
    }

    std::shared_ptr<JsonNode> parseLiteral(const char* literal, JsonNode::Type type) {
        for (const char* p = literal; *p; ++p) {
            if (peek() != *p) {
                throw std::runtime_error(std::string("expected '") + literal + "' at position " + std::to_string(pos_));
            }
            advance();
        }
        return std::make_shared<JsonNode>(type);
    }
};

std::shared_ptr<JsonNode> JsonUtils::parse(const std::string& json) {
    Parser parser(json);
    return parser.parse();
}

std::string JsonUtils::stringify(const JsonNode& node, int indent) {
    return node.stringify(indent);
}

}

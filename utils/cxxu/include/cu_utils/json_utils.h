#ifndef CU_UTILS_JSON_UTILS_H
#define CU_UTILS_JSON_UTILS_H

#include "cu_utils/export.h"
#include <string>
#include <vector>
#include <map>
#include <variant>
#include <memory>

namespace cu {

class JsonNode;
using JsonObject = std::map<std::string, std::shared_ptr<JsonNode>>;
using JsonArray = std::vector<std::shared_ptr<JsonNode>>;

class CXXU_API JsonNode {
public:
    enum Type { Null, Bool, Number, String, Array, Object };

    JsonNode();
    explicit JsonNode(bool v);
    explicit JsonNode(double v);
    explicit JsonNode(const std::string& v);
    explicit JsonNode(Type type);

    Type type() const;

    bool asBool() const;
    double asNumber() const;
    const std::string& asString() const;
    JsonArray& asArray();
    const JsonArray& asArray() const;
    JsonObject& asObject();
    const JsonObject& asObject() const;

    std::shared_ptr<JsonNode> get(const std::string& key) const;
    std::shared_ptr<JsonNode> get(size_t index) const;

    std::string stringify(int indent = -1) const;

private:
    Type type_;
    bool bool_val_ = false;
    double num_val_ = 0.0;
    std::string str_val_;
    JsonArray arr_val_;
    JsonObject obj_val_;
    friend class JsonUtils;
};

class CXXU_API JsonUtils {
public:
    JsonUtils() = delete;
    JsonUtils(const JsonUtils&) = delete;
    JsonUtils& operator=(const JsonUtils&) = delete;

    static std::shared_ptr<JsonNode> parse(const std::string& json);
    static std::string stringify(const JsonNode& node, int indent = -1);
};

}

#endif

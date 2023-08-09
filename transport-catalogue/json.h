#pragma once

#include <iostream>
#include <map>
#include <string>
#include <vector>
#include <variant>

namespace json {

    class Node;
    using Dict = std::map<std::string, Node>;
    using Array = std::vector<Node>;
   

    // Ёта ошибка должна выбрасыватьс€ при ошибках парсинга JSON
    class ParsingError : public std::runtime_error {
    public:
        using runtime_error::runtime_error;
    };

    class Node : public std::variant<std::nullptr_t, Array, Dict, bool, int, double, std::string> {
    public:
        using Value = std::variant<std::nullptr_t, Array, Dict, bool, int, double, std::string>;
        using variant::variant;

        bool IsInt() const;
        bool IsDouble() const;
        bool IsPureDouble() const;
        bool IsBool() const;
        bool IsString() const;
        bool IsNull() const;
        bool IsArray() const;
        bool IsDict() const;

        const Array& AsArray() const;
        const Dict& AsDict() const;
        bool AsBool() const;
        int AsInt() const;
        double AsDouble() const;
        const std::string& AsString() const;

        const Value& GetValue() const;
        Value& GetValue();
    };

    bool operator==(const Node& lhs, const Node& rhs);
    bool operator!=(const Node& lhs, const Node& rhs);

    class Document {
    public:
        explicit Document(Node root);

        const Node& GetRoot() const;

    private:
        Node root_;
    };

    bool operator==(const Document& lhs, const Document& rhs);
    bool operator!=(const Document& lhs, const Document& rhs);

    Document Load(std::istream& input);

    //  онтекст вывода, хранит ссылку на поток вывода и текущий отсуп
    struct PrintContext {
        std::ostream& out;
        int indent_step = 2;
        int indent = 0;

        PrintContext(std::ostream& out, int indent_step, int indent)
            :out(out), indent_step(indent_step), indent(indent) {
        }

        void PrintIndent() const {
            out << std::endl;
            for (int i = 0; i < indent; ++i) {
                out.put(' ');
            }
        }

        // ¬озвращает новый контекст вывода с увеличенным смещением
        PrintContext Indented() const {
            return { out, indent_step, indent_step + indent };
        }
    };

    void Print(const Document& doc, std::ostream& output);

}  // namespace json
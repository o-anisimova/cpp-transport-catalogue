#include "json.h"

using namespace std;

namespace json {

    namespace {

        Node LoadNode(istream& input);

        Node LoadNull(istream& input) {
            string str;
            char c;
            input >> c;
            while (c != '}' && c != ',') {
                if (c != '\t' && c != '\r' && c != '\n') {
                    str.push_back(c);
                }
                input >> c;
            }
            input.putback(c);

            if (str == "null"s) {
                return Node{};
            }
            throw ParsingError("Unexpected value"s);
        }

        Node LoadArray(istream& input) {
            Array result;
            char c;
            while (input >> c && c != ']') {
                if (c != ',') {
                    input.putback(c);
                }
                result.push_back(LoadNode(input));
            }

            if (!result.empty() || c == ']') {
                return Node(move(result));
            }
            else {
                throw ParsingError("Unvalid array"s);
            }
        }

        Node LoadNumber(istream& input) {
            using namespace std::literals;

            std::string parsed_num;

            // Считывает в parsed_num очередной символ из input
            auto read_char = [&parsed_num, &input] {
                parsed_num += static_cast<char>(input.get());
                if (!input) {
                    throw ParsingError("Failed to read number from stream"s);
                }
            };

            // Считывает одну или более цифр в parsed_num из input
            auto read_digits = [&input, read_char] {
                if (!std::isdigit(input.peek())) {
                    throw ParsingError("A digit is expected"s);
                }
                while (std::isdigit(input.peek())) {
                    read_char();
                }
            };

            if (input.peek() == '-') {
                read_char();
            }
            // Парсим целую часть числа
            if (input.peek() == '0') {
                read_char();
                // После 0 в JSON не могут идти другие цифры
            }
            else {
                read_digits();
            }

            bool is_int = true;
            // Парсим дробную часть числа
            if (input.peek() == '.') {
                read_char();
                read_digits();
                is_int = false;
            }

            // Парсим экспоненциальную часть числа
            if (int ch = input.peek(); ch == 'e' || ch == 'E') {
                read_char();
                if (ch = input.peek(); ch == '+' || ch == '-') {
                    read_char();
                }
                read_digits();
                is_int = false;
            }

            try {
                if (is_int) {
                    // Сначала пробуем преобразовать строку в int
                    try {
                        return Node(std::stoi(parsed_num));
                    }
                    catch (...) {
                        // В случае неудачи, например, при переполнении,
                        // код ниже попробует преобразовать строку в double
                    }
                }
                return Node(std::stod(parsed_num));
            }
            catch (...) {
                throw ParsingError("Failed to convert "s + parsed_num + " to number"s);
            }
        }

        // Считывает содержимое строкового литерала JSON-документа
        // Функцию следует использовать после считывания открывающего символа ":
        Node LoadString(istream& input) {
            using namespace std::literals;

            auto it = std::istreambuf_iterator<char>(input);
            auto end = std::istreambuf_iterator<char>();
            std::string s;
            while (true) {
                if (it == end) {
                    // Поток закончился до того, как встретили закрывающую кавычку?
                    throw ParsingError("String parsing error");
                }
                const char ch = *it;
                if (ch == '"') {
                    // Встретили закрывающую кавычку
                    ++it;
                    break;
                }
                else if (ch == '\\') {
                    // Встретили начало escape-последовательности
                    ++it;
                    if (it == end) {
                        // Поток завершился сразу после символа обратной косой черты
                        throw ParsingError("String parsing error");
                    }
                    const char escaped_char = *(it);
                    // Обрабатываем одну из последовательностей: \\, \n, \t, \r, \"
                    switch (escaped_char) {
                    case 'n':
                        s.push_back('\n');
                        break;
                    case 't':
                        s.push_back('\t');
                        break;
                    case 'r':
                        s.push_back('\r');
                        break;
                    case '"':
                        s.push_back('"');
                        break;
                    case '\\':
                        s.push_back('\\');
                        break;
                    default:
                        // Встретили неизвестную escape-последовательность
                        throw ParsingError("Unrecognized escape sequence \\"s + escaped_char);
                    }
                }
                else if (ch == '\n' || ch == '\r') {
                    // Строковый литерал внутри- JSON не может прерываться символами \r или \n
                    throw ParsingError("Unexpected end of line"s);
                }
                else {
                    // Просто считываем очередной символ и помещаем его в результирующую строку
                    s.push_back(ch);
                }
                ++it;
            }

            return Node(s);
        }

        Node LoadBool(istream& input) {
            string str;
            char c;
            input >> c;
            while (c != '}' && c != ',') {
                if (c != '\t' && c != '\r' && c != '\n') {
                    str.push_back(c);
                }
                input >> c;
            }
            input.putback(c);

            if (str == "true"s) {
                return Node(true);
            }
            else if (str == "false"s) {
                return Node(false);
            }
            throw ParsingError("Unexpected value"s);
        }

        Node LoadDict(istream& input) {
            Dict result;

            char c;
            while (input >> c && c != '}') {
                if (c != ',' && c != '\t' && c != '\r' && c != '\n' && c != ' ') {
                    string key;
                    getline(input, key, '"');

                    input >> c;
                    result.insert({ move(key), LoadNode(input) });
                }
            }

            if (!result.empty() || c == '}') {
                return Node(move(result));
            }
            else {
                throw ParsingError("Unvalid dict"s);
            }
        }

        Node LoadNode(istream& input) {
            char c;
            input >> c;
            while (input) {
                if (c != ' ' && c != '\n' && c != '\r' && c != '\t') {
                    break;
                }
                input >> c;
            }

            if (c == 'n') {
                input.putback(c);
                return LoadNull(input);
            }
            else if (c == 't' || c == 'f') {
                input.putback(c);
                return LoadBool(input);
            }
            else if (c == '[') {
                return LoadArray(input);
            }
            else if (c == '{') {
                return LoadDict(input);
            }
            else if (c == '"') {
                return LoadString(input);
            }
            else if (std::isdigit(c) || c == '-') {
                input.putback(c);
                return LoadNumber(input);
            }
            else {
                throw ParsingError("Unknown node"s);
            }
        }

    }  // namespace

    bool Node::IsInt() const {
        return holds_alternative<int>(*this);
    }

    bool Node::IsDouble() const {
        return holds_alternative<double>(*this) || holds_alternative<int>(*this);
    }

    bool Node::IsPureDouble() const {
        return holds_alternative<double>(*this);
    }

    bool Node::IsBool() const {
        return holds_alternative<bool>(*this);
    }

    bool Node::IsString() const {
        return  holds_alternative<string>(*this);
    }

    bool Node::IsNull() const {
        return holds_alternative<nullptr_t>(*this);
    }

    bool Node::IsArray() const {
        return holds_alternative<Array>(*this);
    }

    bool Node::IsMap() const {
        return holds_alternative<Dict>(*this);
    }

    const Array& Node::AsArray() const {
        if (holds_alternative<Array>(*this)) {
            return get<Array>(*this);
        }
        throw logic_error("Unvalid value type"s);
    }

    const Dict& Node::AsMap() const {
        if (holds_alternative<Dict>(*this)) {
            return get<Dict>(*this);
        }
        throw logic_error("Unvalid value type"s);
    }

    bool Node::AsBool() const {
        if (holds_alternative<bool>(*this)) {
            return get<bool>(*this);
        }
        throw logic_error("Unvalid value type"s);
    }

    int Node::AsInt() const {
        if (holds_alternative<int>(*this)) {
            return get<int>(*this);
        }
        throw logic_error("Unvalid value type"s);
    }

    double Node::AsDouble() const {
        if (holds_alternative<double>(*this)) {
            return get<double>(*this);
        }
        else if (holds_alternative<int>(*this)) {
            return get<int>(*this);
        }
        throw logic_error("Unvalid value type"s);
    }

    const string& Node::AsString() const {
        if (holds_alternative<string>(*this)) {
            return get<string>(*this);
        }
        throw logic_error("Unvalid value type"s);
    }

    const Value& Node::GetValue() const {
        return *this;
    }

    bool operator==(const Node& lhs, const Node& rhs) {
        return lhs.GetValue() == rhs.GetValue();
    }

    bool operator!=(const Node& lhs, const Node& rhs) {
        return !(lhs == rhs);
    }

    namespace {

        void PrintNode(const Node& node, const PrintContext& context);

        void PrintValue(nullptr_t, const PrintContext& context) {
            context.out << "null"sv;
        }

        void PrintValue(int value, const PrintContext& context) {
            context.out << value;
        }

        void PrintValue(double value, const PrintContext& context) {
            context.out << value;
        }

        void PrintValue(const string& value, const PrintContext& context) {
            context.out << '"';
            for (char ch : value) {
                switch (ch) {
                case ('\r'): {
                    context.out << "\\r"s;
                    break;
                }
                case ('\n'): {
                    context.out << "\\n"s;
                    break;
                }
                case ('\\'): {
                    context.out << "\\\\"s;
                    break;
                }
                case ('"'): {
                    context.out << "\\\""s;
                    break;
                }
                default: {
                    context.out << ch;
                }
                }
            }
            context.out << '"';
        }

        void PrintValue(bool value, const PrintContext& context) {
            if (value) {
                context.out << "true"sv;
            }
            else {
                context.out << "false"sv;
            }
        }

        void PrintValue(Array arr, const PrintContext& context) {
            context.out << '[';
            if (!arr.empty()) {
                PrintContext new_context(context.Indented());
                new_context.PrintIndent();
                PrintNode(arr[0], new_context);
                for (size_t i = 1; i < arr.size(); ++i) {
                    new_context.out << ", "s;
                    new_context.PrintIndent();
                    PrintNode(arr[i], new_context);
                }
            }
            context.PrintIndent();
            context.out << ']';
        }

        void PrintValue(Dict dict, const PrintContext& context) {
            context.out << '{';
            if (!dict.empty()) {
                PrintContext new_context(context.Indented());
                new_context.PrintIndent();
                auto it = dict.begin();
                new_context.out << '"' << it->first << "\": "s;
                PrintNode(it->second, new_context.Indented());
                ++it;
                while (it != dict.end()) {
                    new_context.out << ", "s;
                    new_context.PrintIndent();
                    new_context.out << '"' << it->first << "\": "s;
                    PrintNode(it->second, new_context.Indented());
                    ++it;
                }
            }
            context.PrintIndent();
            context.out << '}';
        }

        void PrintNode(const Node& node, const PrintContext& context) {
            std::visit(
                [&context](const auto& value) { PrintValue(value, context); },
                node.GetValue());
        }

    } //namespace

    Document::Document(Node root)
        : root_(move(root)) {
    }

    bool operator==(const Document& lhs, const Document& rhs) {
        return lhs.GetRoot() == rhs.GetRoot();
    }

    bool operator!=(const Document& lhs, const Document& rhs) {
        return !(lhs.GetRoot() == rhs.GetRoot());
    }

    const Node& Document::GetRoot() const {
        return root_;
    }

    Document Load(istream& input) {
        return Document{ LoadNode(input) };
    }

    void Print(const Document& doc, std::ostream& output) {
        PrintContext context(output, 2, 0);
        PrintNode(doc.GetRoot(), context);
    }

}  // namespace json
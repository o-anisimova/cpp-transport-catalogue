#include "json_builder.h"

using namespace std;

namespace json {
	
	//BUILDER-----------------------------------------------------------------------------------------------------------------------------------------------------------

	Builder::Builder() {
		AddCommandToCommandLog(CommandType::CONSTRUCTOR, false);
	}

	Builder::KeyItemContext Builder::Key(const std::string& key) {
		AddCommandToCommandLog(CommandType::KEY);
		Node* dict_node_ptr = GetLastNodePtr();
		if (!dict_node_ptr->IsDict()) {
			throw std::logic_error("Invalid node type : Dict was expected"s);
		}
		Dict& val = std::get<Dict>(dict_node_ptr->GetValue());
		nodes_stack_.emplace_back(&(val[key] = Node{}));
		return BaseContext{ *this };
	}

	Builder& Builder::Value(const Node::Value& value) {
		AddCommandToCommandLog(CommandType::VALUE);
		AddValue(value);
		return *this;
	}

	Builder::DictItemContext Builder::StartDict() {
		AddCommandToCommandLog(CommandType::START_DICT);
		AddValue(Dict{});
		return BaseContext{ *this };
	}

	Builder::ArrayItemContext Builder::StartArray() {
		AddCommandToCommandLog(CommandType::START_ARRAY);
		AddValue(Array{});
		return BaseContext{ *this };
	}

	Builder& Builder::EndDict() {
		AddCommandToCommandLog(CommandType::END_DICT);
		if (!GetLastNodePtr()->IsDict()) {
			throw std::logic_error("Invalid node type : Dict was expected"s);
		}
		//Если в корневом узле создали словарь через метод Value, стек будет пустой 
		if (!nodes_stack_.empty()) {
			nodes_stack_.pop_back();
		}
		return *this;
	}

	Builder& Builder::EndArray() {
		AddCommandToCommandLog(CommandType::END_ARRAY);
		if (!GetLastNodePtr()->IsArray()) {
			throw std::logic_error("Invalid node type : Array was expected"s);
		}
		//Если в корневом узле создали массив через метод Value, стек будет пустой 
		if (!nodes_stack_.empty()) {
			nodes_stack_.pop_back();
		}
		return *this;
	}

	const Node& Builder::Build() {
		if (!nodes_stack_.empty() || root_.IsNull()) {
			throw std::logic_error("Found unclosed node or root node undefined"s);
		}
		return root_;
	}

	Node* Builder::GetLastNodePtr() {
		if (!nodes_stack_.empty()) {
			return nodes_stack_.back();
		}
		//Если в стеке ничего нет, то тогда считаем, что последним является корневой узел
		return &root_;
	}

	void Builder::AddValue(const Node::Value& value) {
		Node* node_ptr = GetLastNodePtr();
		//Изначальное значение - массив
		if (node_ptr->IsArray()) {
			Array& array = std::get<Array>(node_ptr->GetValue());
			array.emplace_back(static_cast<const Node&>(value));
			Node* val = &(array.back());
			if (val->IsArray()) {
				if (val->AsArray().empty()) {
					nodes_stack_.emplace_back(val);
				}
			}
			if (val->IsDict()) {
				if (val->AsDict().empty()) {
					nodes_stack_.emplace_back(val);
				}
			}
		}
		else {
			node_ptr->GetValue() = value;
			//Присвоенное значение - не массив, не словарь, узел не является корневым
			if (!node_ptr->IsArray() && !node_ptr->IsDict() && node_ptr != &root_) {
				nodes_stack_.pop_back();
			}
		}
	}

	void Builder::AddCommandToCommandLog(CommandType current_command, bool is_validation_required) {
		if (is_validation_required) {
			CommandType last_command = command_log_.back();
			if (current_command == CommandType::KEY && last_command == CommandType::KEY) {
				throw std::logic_error("Invalid command order: current command = KEY"s);
			}
			if (current_command == CommandType::VALUE && (last_command != CommandType::KEY && last_command != CommandType::VALUE && !root_.IsNull() && !GetLastNodePtr()->IsArray())) {
				throw std::logic_error("Invalid command order: current command = VALUE"s);
			}
			if (current_command == CommandType::START_DICT && (last_command != CommandType::KEY && !root_.IsNull() && !GetLastNodePtr()->IsArray())) {
				throw std::logic_error("Invalid command order: current command = START_DICT"s);
			}
			if (current_command == CommandType::START_ARRAY && (last_command != CommandType::KEY && !root_.IsNull() && !GetLastNodePtr()->IsArray())) {
				throw std::logic_error("Invalid command order: current command = START_DICT"s);
			}
		}
		command_log_.push_back(current_command);
	}

	//BASE CONTEXT-----------------------------------------------------------------------------------------------------------------------------------------------------------

	Builder::BaseContext::BaseContext(Builder& builder)
		:builder_(builder) {
	}

	Builder::BaseContext Builder::BaseContext::Key(const std::string& key) {
		return builder_.Key(key);
	}

	Builder::BaseContext Builder::BaseContext::Value(const Node::Value& value) {
		return builder_.Value(value);
	}

	Builder::BaseContext Builder::BaseContext::StartDict() {
		return builder_.StartDict();
	}

	Builder::BaseContext Builder::BaseContext::StartArray() {
		return builder_.StartArray();
	}

	Builder::BaseContext Builder::BaseContext::EndDict() {
		return builder_.EndDict();
	}

	Builder::BaseContext Builder::BaseContext::EndArray() {
		return builder_.EndArray();
	}

	const Node& Builder::BaseContext::Build() {
		return builder_.Build();
	}

	//KEY ITEM CONTEXT-----------------------------------------------------------------------------------------------------------------------------------------------------------

	Builder::KeyItemContext::KeyItemContext(BaseContext base)
		: BaseContext(base) {
	}

	Builder::DictItemContext Builder::KeyItemContext::Value(const Node::Value& value) {
		return BaseContext::Value(value);
	}

	//DICT ITEM CONTEXT-----------------------------------------------------------------------------------------------------------------------------------------------------------

	Builder::DictItemContext::DictItemContext(BaseContext base)
		: BaseContext(base) {
	}

	Builder::KeyItemContext Builder::DictItemContext::Key(const std::string& key) {
		return BaseContext::Key(key);
	}

	//ARRAY ITEM CONTEXT-----------------------------------------------------------------------------------------------------------------------------------------------------------

	Builder::ArrayItemContext::ArrayItemContext(BaseContext base)
		: BaseContext(base) {
	}

	Builder::ArrayItemContext Builder::ArrayItemContext::Value(const Node::Value& value) {
		return BaseContext::Value(value);
	}

} //namespace json
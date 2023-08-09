#pragma once

#include "json.h"

namespace json {

	class Builder {
	private:
		class BaseContext;
		class KeyItemContext;
		class KeyValueContext;
		class DictItemContext;
		class ArrayItemContext;
		class ArrayValueContext;
	public:
		Builder();
		Builder::KeyItemContext Key(const std::string& key);
		Builder& Value(const Node::Value& value);
		Builder::DictItemContext StartDict();
		Builder::ArrayItemContext StartArray();
		Builder& EndDict();
		Builder& EndArray();
		const Node& Build();

	private:
		class BaseContext {
		public:
			BaseContext(Builder& builder);
			BaseContext Key(const std::string& key);
			BaseContext Value(const Node::Value& value);
			BaseContext StartDict();
			BaseContext StartArray();
			BaseContext EndDict();
			BaseContext EndArray();
			const Node& Build();

		private:
			Builder& builder_;
		};

		//Правило 1
		class KeyItemContext : public BaseContext {
		public:
			KeyItemContext(BaseContext base);

			KeyItemContext Key(const std::string& key) = delete;
			KeyItemContext EndDict() = delete;
			KeyItemContext EndArray() = delete;
			const Node& Build() = delete;

			KeyValueContext Value(const Node::Value& value);
		};

		//Правило 2
		class KeyValueContext : public BaseContext {
		public:
			KeyValueContext(BaseContext base);

			KeyValueContext Value(const Node::Value& value) = delete;
			KeyValueContext StartDict() = delete;
			KeyValueContext StartArray() = delete;
			KeyValueContext EndArray() = delete;
			const Node& Build() = delete;
		};

		//Правило 3
		class DictItemContext : public BaseContext {
		public:
			DictItemContext(BaseContext base);

			DictItemContext StartDict() = delete;
			DictItemContext StartArray() = delete;
			DictItemContext EndArray() = delete;
			const Node& Build() = delete;

			KeyItemContext Key(const std::string& key);

		};

		//Правило 4
		class ArrayItemContext : public BaseContext {
		public:
			ArrayItemContext(BaseContext base);
			KeyItemContext Key(const std::string& key) = delete;
			Builder& EndDict() = delete;
			const Node& Build() = delete;

			ArrayValueContext Value(const Node::Value& value);
		};

		//Правило 5
		class ArrayValueContext : public BaseContext {
		public:
			ArrayValueContext(BaseContext base);
			ArrayValueContext Key(const std::string& key) = delete;
			ArrayValueContext EndDict() = delete;
			const Node& Build() = delete;

			ArrayValueContext Value(const Node::Value& value);
		};

		enum CommandType {
			CONSTRUCTOR,
			KEY,
			VALUE,
			START_DICT,
			START_ARRAY,
			END_DICT,
			END_ARRAY,
		};

		Node root_;
		std::vector<Node*> nodes_stack_;
		std::vector<CommandType> command_log_;

		Node* GetLastNodePtr();

		void AddValue(const Node::Value& value);

		void AddCommandToCommandLog(CommandType current_command, bool is_validation_required = true);
	};

} //namespace json
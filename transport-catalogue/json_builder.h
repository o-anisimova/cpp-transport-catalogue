#pragma once

#include "json.h"

namespace json {

	class Builder {
	private:
		class BaseContext;
		class KeyItemContext;
		class DictItemContext;
		class ArrayItemContext;
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

		class KeyItemContext : public BaseContext {
		public:
			KeyItemContext(BaseContext base);

			KeyItemContext Key(const std::string& key) = delete;
			KeyItemContext EndDict() = delete;
			KeyItemContext EndArray() = delete;
			const Node& Build() = delete;

			DictItemContext Value(const Node::Value& value);
		};

		class DictItemContext : public BaseContext {
		public:
			DictItemContext(BaseContext base);

			DictItemContext Value(const Node::Value& value) = delete;
			DictItemContext StartDict() = delete;
			DictItemContext StartArray() = delete;
			DictItemContext EndArray() = delete;
			const Node& Build() = delete;

			KeyItemContext Key(const std::string& key);

		};

		class ArrayItemContext : public BaseContext {
		public:
			ArrayItemContext(BaseContext base);
			KeyItemContext Key(const std::string& key) = delete;
			Builder& EndDict() = delete;
			const Node& Build() = delete;

			ArrayItemContext Value(const Node::Value& value);
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
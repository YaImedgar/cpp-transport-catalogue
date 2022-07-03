#pragma once

#include "json.h"

#include <optional>
#include <memory>
#include <stack>
#include <set>
#include <functional>
#include <vector>
#include <map>

namespace json
{
	class Builder;
	class DictItemContext;
	class ArrayItemContext;
	class KeyItemContext;
	class DictValueItemContext;
	class ArrayValueItemContext;

	class BaseContext
	{
	public:
		BaseContext() = delete;
		BaseContext(const BaseContext&) = delete;
		BaseContext(BaseContext&&) = default;
		BaseContext& operator=(const BaseContext&) = delete;
		BaseContext& operator=(BaseContext&& other) = delete;
		~BaseContext() = default;

		BaseContext( Builder& builder );

		BaseContext Value(const json::Node& value);
		DictItemContext StartDict();
		KeyItemContext Key(const std::string& key);
		BaseContext EndDict();
		ArrayItemContext StartArray();
		BaseContext EndArray();
		json::Node Build();
	protected:
		void CheckLastItem(const Node& value);
		Builder& builder_;
	};

	class  DictItemContext final : public BaseContext
	{
	public:
		DictItemContext() = delete;
		DictItemContext(const DictItemContext&) = delete;
		DictItemContext(DictItemContext&&) = delete;
		DictItemContext& operator=(const DictItemContext&) = delete;
		DictItemContext& operator=(DictItemContext&& other) = delete;

		BaseContext Value(const json::Node& value) = delete;
		DictItemContext StartDict() = delete;
		ArrayItemContext StartArray() = delete;
		BaseContext EndArray() = delete;
		json::Node Build() = delete;
	};

	class  KeyItemContext final : public BaseContext
	{
	public:
		KeyItemContext() = delete;
		KeyItemContext(const KeyItemContext&) = delete;
		KeyItemContext(KeyItemContext&&) = delete;
		KeyItemContext& operator=(const KeyItemContext&) = delete;
		KeyItemContext& operator=(KeyItemContext&& other) = delete;

		DictValueItemContext Value(const json::Node& value);

		KeyItemContext Key(const std::string& key) = delete;
		BaseContext EndDict() = delete;
		BaseContext EndArray() = delete;
		json::Node Build() = delete;
	};

	class  ArrayItemContext final : public BaseContext
	{
	public:
		ArrayItemContext() = delete;
		ArrayItemContext(const ArrayItemContext&) = delete;
		ArrayItemContext(ArrayItemContext&&) = delete;
		ArrayItemContext& operator=(const ArrayItemContext&) = delete;
		ArrayItemContext& operator=(ArrayItemContext&& other) = delete;

		ArrayValueItemContext Value(const json::Node& value);
		KeyItemContext Key(const std::string& key) = delete;
		BaseContext EndDict() = delete;
		json::Node Build() = delete;
	};

	class ArrayValueItemContext final : public BaseContext
	{
		ArrayValueItemContext() = delete;
	public:
		explicit ArrayValueItemContext( BaseContext& context );

		ArrayValueItemContext Value(const json::Node& value);
		KeyItemContext Key(const std::string& key) = delete;
		BaseContext EndDict() = delete;
		json::Node Build() = delete;
	};

	class DictValueItemContext final : public BaseContext
	{
		DictValueItemContext() = delete;
	public:
		explicit DictValueItemContext( BaseContext& context );

		BaseContext Value(const json::Node& value) = delete;
		DictItemContext StartDict() = delete;
		ArrayItemContext StartArray() = delete;
		BaseContext EndArray() = delete;
		json::Node Build() = delete;
	};

	class BaseValueItemContext final : public BaseContext
	{
		BaseValueItemContext() = delete;
	public:
		explicit BaseValueItemContext( BaseContext& context );

		BaseValueItemContext Value(const json::Node& value) = delete;
		DictItemContext StartDict() = delete;
		KeyItemContext Key(const std::string& key) = delete;
		BaseContext EndDict() = delete;
		ArrayItemContext StartArray() = delete;
		BaseContext EndArray() = delete;
	};

	class Builder
	{
	public:
		Builder() = default;

		Builder(const Builder&) = delete;
		Builder(Builder&&) = default;

		Builder& operator=(const Builder&) = delete;
		Builder& operator=(Builder&& other) = default;

		~Builder() = default;

		BaseContext Value(const json::Node& value);

		DictItemContext StartDict();
		KeyItemContext Key(const std::string& key);
		BaseContext EndDict();

		ArrayItemContext StartArray();
		BaseContext EndArray();

		json::Node Build();
	private:
		void CheckReady(const std::string& function_name) const;
		void CheckUsage(const std::string& function_name) const;
		void CheckEnd(const std::string& function_name) const;
		void CheckKey() const;
		void EndStructure(const std::string& function_name);
		[[nodiscard]] bool InitHead(const Node& node);

		bool is_ready_ = false;
		bool is_wait_key = false;
		bool is_key_ = false;
		bool is_array_ = false;

		std::optional<Node> head_;
		std::stack<Node*> nodes_stack_;
	};
}

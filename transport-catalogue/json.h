#pragma once

#include <iostream>
#include <map>
#include <string>
#include <sstream>
#include <cassert>
#include <vector>
#include <variant>
#include <unordered_map>

namespace json
{
	class Node;
	using Dict = std::map<std::string, Node>;
	using Array = std::vector<Node>;
	using NodeVariant = std::variant<std::nullptr_t, Array, Dict, bool, int, double, std::string>;

	// Эта ошибка должна выбрасываться при ошибках парсинга JSON
	class ParsingError : public std::runtime_error {
	public:
		using runtime_error::runtime_error;
	};

	class Node {
	public:
		Node() = default;

		template <typename Type>
		Node(Type value)
			: json_node_(value)
		{}

		bool operator==(const Node& other) const;
		bool operator!=(const Node& other) const;

		bool IsNull() const;
		bool IsArray() const;
		bool IsMap() const;
		bool IsBool() const;
		bool IsInt() const;
		bool IsDouble() const;
		bool IsPureDouble() const;
		bool IsString() const;

		const Array& AsArray() const;
		const Dict& AsMap() const;
		bool AsBool() const;
		int AsInt() const;
		double AsDouble() const;
		const std::string& AsString() const;

	private:
		NodeVariant json_node_;

		template <typename T>
		const T& CheckAndReturnType() const
		{
			return (std::holds_alternative<T>(json_node_) ?
				std::get<T>(json_node_) :
				throw std::logic_error("Bad convertation"));
		}
	};

	class Document {
	public:
		explicit Document(Node root);

		const Node& GetRoot() const;

	private:
		Node root_;
	};

	bool operator==(json::Document lhs, json::Document rhs);
	bool operator!=(json::Document lhs, json::Document rhs);

	Document Load(std::istream& input);

	void Print(const Document& doc, std::ostream& output);

	std::string PrintStr(const Node& node);
}  // namespace json
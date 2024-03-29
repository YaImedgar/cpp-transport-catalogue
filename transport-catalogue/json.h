#pragma once

#include <iostream>
#include <map>
#include <string>
#include <variant>
#include <sstream>
#include <vector>

namespace json
{
	class Node;
	using Dict = std::map<std::string, Node>;
	using Array = std::vector<Node>;

	class ParsingError : public std::runtime_error
	{
	public:
		using runtime_error::runtime_error;
	};

	class Node final
		: private std::variant<std::nullptr_t, Array, Dict, bool, int, double, std::string>
	{
	public:
		friend class Builder;
		using variant::variant;
		using Value = variant;

		[[nodiscard]] bool IsInt() const;
		[[nodiscard]] int AsInt() const;

		[[nodiscard]] bool IsPureDouble() const;
		[[nodiscard]] bool IsDouble() const;
		[[nodiscard]] double AsDouble() const;

		[[nodiscard]] bool IsBool() const;
		[[nodiscard]] bool AsBool() const;

		[[nodiscard]] bool IsNull() const;

		[[nodiscard]] bool IsArray() const;

		[[nodiscard]] const Array& AsArray() const;

		[[nodiscard]] bool IsString() const;

		[[nodiscard]] const std::string& AsString() const;

		[[nodiscard]] bool IsDict() const;

		[[nodiscard]] const Dict& AsDict() const;

		bool operator==(const Node& rhs) const;

		[[nodiscard]] const Value& GetValue() const;

	private:
		Value& GetValue();
	};

	inline bool operator!=(const Node& lhs, const Node& rhs)
	{
		return !(lhs == rhs);
	}

	class Document
	{
	public:
		explicit Document( Node root );

		[[nodiscard]] const Node& GetRoot() const;

	private:
		Node root_;
	};

	inline bool operator==(const Document& lhs, const Document& rhs)
	{
		return lhs.GetRoot() == rhs.GetRoot();
	}

	inline bool operator!=(const Document& lhs, const Document& rhs)
	{
		return !(lhs == rhs);
	}

	Document Load(std::istream& input);

	void Print(const Document& doc, std::ostream& output = std::cout);

}  // namespace json

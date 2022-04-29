#include "json.h"

using namespace std;

namespace json
{
	namespace
	{
		Node LoadNode(istream& input);

		Node LoadArray(istream& input) {
			Array result;

			for (char c; input >> c && c != ']';) {
				if (c != ',') {
					input.putback(c);
				}
				result.push_back(LoadNode(input));
			}

			if (!input)
			{
				throw ParsingError("Failed with Array"s);
			}

			return Node(move(result));
		}

		using Number = std::variant<int, double>;

		Number LoadNumber(std::istream& input) {
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
						return std::stoi(parsed_num);
					}
					catch (...) {
						// В случае неудачи, например, при переполнении,
						// код ниже попробует преобразовать строку в double
					}
				}
				return std::stod(parsed_num);
			}
			catch (...) {
				throw ParsingError("Failed to convert "s + parsed_num + " to number"s);
			}
		}

		Node LoadString(istream& input)
		{
			static unordered_map<std::string, char> symbols
			{
				{"\\\\"s, '\\'},
				{"\\\""s, '"'},
				{"\\n"s, '\n'},
				{"\\r"s, '\r'},
				{"\\/"s, '/'},
				{"\\b"s, '\b'},
				{"\\f"s, '\f'},
				{"\\t"s, '\t'}
			};

			if (!input)
			{
				throw ParsingError("String parsing failed"s);
			}

			char ch;
			vector<char> str;
			while (input.get(ch) && ch != '\"')
			{
				if (ch == '\\')
				{
					char next_ch;
					input.get(next_ch);
					char escape;
					try
					{
						std::string esc = "";
						esc += ch;
						esc += next_ch;
						escape = symbols.at(esc);
					}
					catch (const std::exception&)
					{
						throw ParsingError("Bad escape"s);
					}
					str.push_back(escape);
				}
				else
				{
					str.push_back(ch);
				}
			}

			if (!input || ch != '\"')
			{
				throw ParsingError("String parsing failed"s);
			}

			return Node(move(std::string{ str.begin(), str.end() }));
		}

		Node LoadDict(istream& input)
		{
			Dict result;

			for (char c; input >> c && c != '}';) {
				if (c == ',') {
					input >> c;
				}

				string key = LoadString(input).AsString();
				input >> c;
				result.insert({ move(key), LoadNode(input) });
			}

			if (!input)
			{
				throw ParsingError("Failed with Array"s);
			}

			return Node(move(result));
		}

		Node LoadNode(istream& input)
		{
			char c;

			Number num;

			if (!(input >> c))
			{
				throw ParsingError("Failed pars"s);
			}
			switch (c)
			{
			case '[':
				return LoadArray(input);
			case '{':
				return LoadDict(input);
			case '"':
				return LoadString(input);
			case '-':
			case '+':
			case '0':
			case '1':
			case '2':
			case '3':
			case '4':
			case '5':
			case '6':
			case '7':
			case '8':
			case '9':
				input.putback(c);
				num = LoadNumber(input);
				if (std::holds_alternative<int>(num))
				{
					return Node{ std::get<int>(num) };
				}
				else if (std::holds_alternative<double>(num))
				{
					return Node{ std::get<double>(num) };
				}
				break;
			case 't':
				if (input.get() == 'r' && input.get() == 'u' && input.get() == 'e')
				{
					return Node{ true };
				}
				break;
			case 'f':
				if (input.get() == 'a' && input.get() == 'l' && input.get() == 's' && input.get() == 'e')
				{
					return Node{ false };
				}
				break;
			case 'n':
				if (input.get() == 'u' && input.get() == 'l' && input.get() == 'l')
				{
					return Node{ nullptr };
				}
				break;
			}
			throw ParsingError("Failed pars with \""s + c + "\""s);
		}
	}  // namespace

	Document::Document(Node root)
		: root_(move(root)) {
	}

	const Node& Document::GetRoot() const
	{
		return root_;
	}

	bool operator==(json::Document lhs, json::Document rhs)
	{
		return lhs.GetRoot() == rhs.GetRoot();
	}

	bool operator!=(json::Document lhs, json::Document rhs)
	{
		return lhs.GetRoot() != rhs.GetRoot();
	}

	Document Load(istream& input)
	{
		return Document{ LoadNode(input) };
	}

	bool CheckEscapes(char c, std::stringstream& ss)
	{
		switch (c)
		{
		case '\"':
			ss << "\\" << c;
			return true;
		case '\\':
			ss << "\\" << c;
			return true;
		case '\b':
			ss << "\\" << 'b';
			return true;
		case '\f':
			ss << "\\" << 'f';
			return true;
		case '\n':
			ss << "\\" << 'n';
			return true;
		case '\r':
			ss << "\\" << 'r';
			return true;
		}
		return false;
	}

	void Print(const Document& doc, std::ostream& output)
	{
		json::Node rootNode = doc.GetRoot();

		if (rootNode.IsNull())
		{
			output << "null";
		}
		else if (rootNode.IsInt())
		{
			output << rootNode.AsInt();
		}
		else if (rootNode.IsPureDouble())
		{
			output << rootNode.AsDouble();
		}
		else if (rootNode.IsBool())
		{
			output << (rootNode.AsBool() ? "true"s : "false"s);
		}
		else if (rootNode.IsString())
		{
			std::stringstream ss;
			ss << "\"";

			for (auto ch : rootNode.AsString())
			{
				if (!CheckEscapes(ch, ss))
				{
					ss << ch;
				}
			}
			ss << "\"";
			output << ss.str();
		}
		else if (rootNode.IsArray())
		{
			bool isFirst = true;
			output << '[';
			for (const auto& node : rootNode.AsArray())
			{
				if (!isFirst)
				{
					output << ", ";
				}
				else
				{
					isFirst = false;
				}

				output << json::PrintStr(node);
			}

			output << ']';
		}
		else if (rootNode.IsMap())
		{
			bool isFirst = true;
			output << "{";
			for (const auto& node : rootNode.AsMap())
			{
				if (!isFirst)
				{
					output << ",";
				}
				else
				{
					isFirst = false;
				}

				output << json::PrintStr(node.first);
				output << ":";
				output << json::PrintStr(node.second);
			}

			output << "}";
		}
	}

	std::string PrintStr(const Node& node)
	{
		std::ostringstream out;
		Print(Document{ node }, out);
		return out.str();
	}

	bool Node::operator==(const Node& other) const
	{
		return this->json_node_ == other.json_node_;
	}

	bool Node::operator!=(const Node& other) const
	{
		return this->json_node_ != other.json_node_;
	}

	bool Node::IsNull() const
	{
		return std::holds_alternative<std::nullptr_t>(json_node_);
	}

	bool Node::IsArray() const
	{
		return std::holds_alternative<json::Array>(json_node_);
	}

	bool Node::IsMap() const
	{
		return std::holds_alternative<json::Dict>(json_node_);
	}

	bool Node::IsBool() const
	{
		return std::holds_alternative<bool>(json_node_);
	}

	bool Node::IsInt() const
	{
		return std::holds_alternative<int>(json_node_);
	}

	bool Node::IsDouble() const
	{
		return std::holds_alternative<int>(json_node_) ||
			std::holds_alternative<double>(json_node_);
	}

	bool Node::IsPureDouble() const
	{
		return std::holds_alternative<double>(json_node_);
	}

	bool Node::IsString() const
	{
		return std::holds_alternative<std::string>(json_node_);
	}

	const Array& Node::AsArray() const
	{
		return CheckAndReturnType<Array>();
	}

	const Dict& Node::AsMap() const
	{
		return CheckAndReturnType<Dict>();
	}

	bool Node::AsBool() const
	{
		return CheckAndReturnType<bool>();
	}

	int Node::AsInt() const
	{
		return CheckAndReturnType<int>();
	}

	double Node::AsDouble() const
	{
		if (IsPureDouble())
		{
			return CheckAndReturnType<double>();
		}
		else
		{
			return CheckAndReturnType<int>();
		}
	}

	const std::string& Node::AsString() const
	{
		return CheckAndReturnType<std::string>();
	}
}  // namespace json
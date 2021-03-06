
/**
 * @file var.h
 * @brief A universal type much like a JavaScript object
 * @author Ike Hirzel
 * 
 * Copyright 2021 Ike Hirzel
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in the
 * Software without restriction, including without limitation the rights to use, copy,
 * modify, merge, publish, distribute, sublicense, and/or sell copies of the Software,
 * and to permit persons to whom the Software is furnished to do so, subject to the
 * following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all copies or
 * substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
 * INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A 
 * PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
 * OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#ifndef VAR_H
#define VAR_H

#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>

namespace hirzel
{
	class var
	{
	private:

		union Data
		{
			bool _boolean;
			char _character;
			double _float;
			std::string* _string;
			unsigned long long _unsigned;
			long long _integer = 0;
			std::vector<var>* _array;
			std::unordered_map<std::string, var>* _map;
		};
		Data _data;
		// type of data
		char _type = 0;

		static var parse_json_object(const std::string& src, size_t& i);
		static var parse_json_array(const std::string& src, size_t& i);
		static var parse_json_value(const std::string& src, size_t& i);

	public:

		enum Type
		{
			NULL_TYPE,
			ERROR_TYPE,
			INT_TYPE,
			UINT_TYPE,
			FLOAT_TYPE,
			CHAR_TYPE,
			BOOL_TYPE,
			STR_TYPE,
			ARRAY_TYPE,
			MAP_TYPE
		};

		var() = default;

		var(const var& other);
		var(var&& other);

		var(Type t);

		var(long long i);
		inline var(short i) : var((long long)i) {}
		inline var(int i) : var((long long)i) {}
		inline var(long i) : var((long long)i) {}

		var(unsigned long long u);
		inline var(unsigned short u): var((unsigned long long)u) {}
		inline var(unsigned int u) : var((unsigned long long )u) {}
		inline var(unsigned long u) : var((unsigned long long)u) {}
		
		var(double d);
		inline var(float f) : var((double)f) {}

		var(bool b);

		var(char c);

		var(const std::string& s, bool error = false);
		inline var(char* c) : var(std::string(c)) {}
		inline var(const char* c) : var(std::string(c)) {}

		var(const std::initializer_list<var>& list);

		~var();

		static var parse_json(const std::string& src);
		inline static var error(const std::string& msg) { return var(msg, true); }
		
		intmax_t to_int() const;
		uintmax_t to_uint() const;
		double to_double() const;
		char to_char() const;
		bool to_bool() const;
		std::string to_string() const;

		inline bool is_error() const { return _type == ERROR_TYPE; }
		inline Data data() const { return _data; }
		inline size_t size() const
		{
			switch (_type)
			{
			case INT_TYPE:
				return sizeof(_data._integer);
			case UINT_TYPE:
				return sizeof(_data._unsigned);
			case FLOAT_TYPE:
				return sizeof(_data._float);			
			case CHAR_TYPE:
				return sizeof(_data._character);
			case BOOL_TYPE:
				return sizeof(_data._boolean);
			case STR_TYPE:
				return _data._string->size();
			case ARRAY_TYPE:
				return _data._array->size();
			case MAP_TYPE:
				return _data._map->size();
			}
			return 0;
		}
		
		inline int type() const { return (int)_type; }

		var& operator=(const var& other);

		var& operator[](size_t i);
		inline const var& operator[](size_t i) const { return (*this)[i]; }

		var& operator[](const std::string& key);
		inline const var& operator[](const std::string& key) const { return (*this)[key]; }

		friend std::ostream& operator<<(std::ostream& out, const var& v);
	};
}

#endif // VAR_H

#ifdef HIRZEL_VAR_IMPLEMENTATION
#undef HIRZEL_VAR_IMPLEMENTATION

namespace hirzel
{
	var::var(const var& other)
	{
		*this = other;
	}

	var::var(var&& other)
	{
		_type = other._type;
		_data = other._data;
		other._type = NULL_TYPE;
	}

	var::var(Type t)
	{
		_type = t;
		switch (t)
		{
		case NULL_TYPE:
			_data._integer = 0;
			break;
		case ERROR_TYPE:
			_data._string = new std::string();
			break;
		case INT_TYPE:
			_data._integer = 0;
			break;
		case UINT_TYPE:
			_data._unsigned = 0;
			break;
		case FLOAT_TYPE:
			_data._float = 0.0;
			break;
		case CHAR_TYPE:
			_data._character = 0;
			break;
		case BOOL_TYPE:
			_data._boolean = false;
			break;
		case STR_TYPE:
			_data._string = new std::string();
			break;
		case ARRAY_TYPE:
			_data._array = new std::vector<var>();
			break;
		case MAP_TYPE:
			_data._map = new std::unordered_map<std::string,var>();
			break;
		}
	}

	var::var(long long i)
	{
		_data._integer = i;
		_type = var::INT_TYPE;
	}

	var::var(long long unsigned u)
	{
		_data._unsigned = u;
		_type = var::UINT_TYPE;
	}

	var::var(char c)
	{
		_data._character = c;
		_type = var::CHAR_TYPE;
	}

	var::var(double d)
	{
		_data._float = d;
		_type = var::FLOAT_TYPE;
	}

	var::var(bool b)
	{
		_data._boolean = b;
		_type = var::BOOL_TYPE;
	}

	var::var(const std::string& s, bool error)
	{
		if (error)
			_type = ERROR_TYPE;
		else
			_type = STR_TYPE;
		_data._string = new std::string(s);
	}

	var::var(const std::initializer_list<var>& list)
	{
		_type = ARRAY_TYPE;
		_data._array = new std::vector<var>(list);
	}

	var::~var()
	{
		switch (_type)
		{
		case ERROR_TYPE:
		case STR_TYPE:
			delete _data._string;
			break;

		case ARRAY_TYPE:
			delete _data._array;
			break;

		case MAP_TYPE:
			delete _data._map;
			break;
		}
	}

	intmax_t var::to_int() const
	{
		switch(_type)
		{
			case INT_TYPE:
				return _data._integer;
			case UINT_TYPE:
				return (intmax_t)_data._unsigned;
			case BOOL_TYPE:
				return (intmax_t)_data._boolean;
			case CHAR_TYPE:
				return (intmax_t)_data._character;
			case FLOAT_TYPE:
				return (intmax_t)_data._float;
			case STR_TYPE:
				try
				{
					return std::stoll(*_data._string);
				}
				catch(std::invalid_argument e)
				{
					return 0;
				}
			default:
				return 0;
		}
	}

	uintmax_t var::to_uint() const
	{
		switch(_type)
		{
			case INT_TYPE:
				return (uintmax_t)_data._integer;
			case UINT_TYPE:
				return (uintmax_t)_data._unsigned;
			case BOOL_TYPE:
				return (uintmax_t)_data._boolean;
			case CHAR_TYPE:
				return (uintmax_t)_data._character;
			case FLOAT_TYPE:
				return (uintmax_t)_data._float;
			case STR_TYPE:
				try
				{
					return std::stoull(*_data._string);
				}
				catch (std::invalid_argument e)
				{
					return 0;
				}
			default:
				return 0;
		}
	}

	double var::to_double() const
	{
		switch(_type)
		{
			case NULL_TYPE:
				return 0.0;
			case INT_TYPE:
				return (double)_data._integer;
			case UINT_TYPE:
				return (double)_data._unsigned;
			case BOOL_TYPE:
				return (double)_data._boolean;
			case CHAR_TYPE:
				return (double)_data._character;
			case FLOAT_TYPE:
				return _data._float;
			case STR_TYPE:
				try
				{
					return std::stod(*_data._string);
				}
				catch(std::invalid_argument e)
				{
					return 0.0;
				}
			default:
				return 0.0;
		}
	}

	char var::to_char() const
	{
		switch(_type)
		{
			case INT_TYPE:
				return (char)_data._integer;
			case UINT_TYPE:
				return (char)_data._unsigned;
			case BOOL_TYPE:
				return (char)_data._boolean;
			case CHAR_TYPE:
				return (char)_data._character;
			case FLOAT_TYPE:
				return (char)_data._float;
			case STR_TYPE:
				return (_data._string->empty() ? 0 : (*_data._string)[0]);
			default:
				return 0;
		}
	}

	bool var::to_bool() const
	{
		switch(_type)
		{
			case INT_TYPE:
				return (bool)_data._integer;
			case UINT_TYPE:
				return (bool)_data._unsigned;
			case BOOL_TYPE:
				return _data._boolean;
			case CHAR_TYPE:
				return (bool)_data._character;
			case FLOAT_TYPE:
				return (bool)_data._float;
			case STR_TYPE:
				return !_data._string->empty();
			default:
				return false;
		}
	}

	std::string var::to_string() const
	{
		std::string out;
		switch(_type)
		{
			case NULL_TYPE:
				out = "null";
				break;
			case INT_TYPE:
				out = std::to_string(_data._integer);
				break;
			case UINT_TYPE:
				out = std::to_string(_data._unsigned);
				break;
			case BOOL_TYPE:
				out = (_data._boolean ? "true" : "false");
				break;
			case CHAR_TYPE:
				out =  std::string(1, _data._character);
				break;
			case FLOAT_TYPE:
				out = std::to_string(_data._float);
				break;
			case ERROR_TYPE:
			case STR_TYPE:
				out = *_data._string;
				break;
			case ARRAY_TYPE:
				out = "[";
				for (int i = 0; i < _data._array->size(); i++)
				{
					if (i > 0) out += ", ";
					out += (*_data._array)[i].to_string();
				}
				out += "]";
				break;

			case MAP_TYPE:
				out = "{\n";
				int i;
				int size;
				size = _data._map->size();
				int curr;
				curr = 0;
				for (const std::pair<std::string, var>& p : *_data._map)
				{
					out +="\t\"" + p.first + "\": ";
					i = out.size();
					out += p.second.to_string();
					for (int j = i; j < out.size(); j++)
					{
						if (out[j] == '\n')
						{
							j++;
							out.insert(out.begin() + j, '\t');
						}
					}
					if (curr < size - 1) out += ',';
					out += '\n';
					curr++;
				}
				out += '}';
				break;
		}
		return out;
	}

	var& var::operator=(const var& other)
	{
		_type = other.type();
		switch(_type)
		{
		case ARRAY_TYPE:
			_data._array = new std::vector<var>(*other.data()._array);
			break;

		case MAP_TYPE:
			_data._map = new std::unordered_map<std::string, var>(*other.data()._map);
			break;

		case ERROR_TYPE:
		case STR_TYPE:
			_data._string = new std::string(*other.data()._string);
			break;

		default:
			_data = other.data();
			break;
		}

		return *this;
	}

	var& var::operator[](const std::string& key)
	{
		switch (_type)
		{
		case ARRAY_TYPE:
			std::vector<var>* arr;
			arr = _data._array;
			_type = MAP_TYPE;
			_data._map = new std::unordered_map<std::string, var>();

			for (int i = 0; i < arr->size(); i++)
			{
				(*_data._map)[std::to_string(i)] = (*arr)[i];
			}
			return (*_data._map)[key];

		case MAP_TYPE:
			return (*_data._map)[key];

		default:
			_type = MAP_TYPE;
			_data._map = new std::unordered_map<std::string, var>();
			return (*_data._map)[key];
		}
	}

	var& var::operator[](size_t i)
	{
		switch (_type)
		{
		case ARRAY_TYPE:
			if (i >= _data._array->size()) (*_data._array).resize(i + 1);
			return (*_data._array)[i];

		case MAP_TYPE:
			return (*_data._map)[std::to_string(i)];

		default:
			_type = var::ARRAY_TYPE;
			_data._array = new std::vector<var>(i + 1);
			return (*_data._array)[i];
		}
	}

	std::ostream& operator<<(std::ostream& out, const var& v)
	{
		out << v.to_string();
		return out;
	}

	// Static functions

	var var::parse_json_value(const std::string& src, size_t& i)
	{
		//std::cout << "PARSING PRIM starting with " << src[i] << "\n";
		char first = src[i];
		if (first >= '0' && first <= '9' || first == '-')
		{
			char tmp[128];
			char* pos = tmp;
			*pos++ = first;
			i++;
			bool dec = false;
			bool neg = false;
			if (first == '-') neg = true;

			while (true)
			{
				if (i == src.size()) break;
				char c = src[i];
				// not a number
				if (c == '.')
				{
					if (dec) return error("JSON: stray '.' found in number literal at position: " + std::to_string(i));
					dec = true;
				}
				else if (c < '0' || c > '9') break;
				*pos++ = c;
				i++;
			}
			*pos = 0;

			if (dec)
			{
				return var(std::stod(tmp));
			}
			else
			{
				if (neg)
					return var(std::stoll(tmp));
				else
					return var(std::stoull(tmp));
			}
		}
		else if (first == '\"')
		{
			char tmp[256];
			char* pos = tmp;
			i++;
			while (src[i] != '\"')
			{
				*pos++ = src[i++];
			}
			i++;
			*pos = 0;
			return var(tmp);
		}
		else
		{
			const char* match;
			int size;
			switch(first)
			{
			case '{':
				return parse_json_object(src, i);
			case '[':
				return parse_json_array(src, i);
			case 't':
				match = "true";
				size = 4;
				break;

			case 'f':
				match = "false";
				size = 5;
				break;

			case 'n':
				match = "null";
				size = 4;
				break;

			default:
				return error("JSON: invalid token '" + std::string(1, first) + "' at position: " + std::to_string(i));
			}

			const char* c = match + 1;
			size_t start_of_literal = i;
			i++;
			while (true)
			{	
				if (c - match == size) break;
				if (i == src.size())
				{
					return error("unexpected token '" + std::string(1, src[start_of_literal]) + "' in primitive literal at position: " + std::to_string(start_of_literal));
				}
				if (src[i++] != *c++)
				{
					return error("unexpected token '" + std::string(1, *c) + "' in primitive literal at position: " + std::to_string(i));
				}
			}

			switch (first)
			{
			case 't':
				return var(true);
			case 'f':
				return var(false);
			case 'n':
				return var();
			}
		}
		
		return var();
	}

	var var::parse_json_array(const std::string& src, size_t& i)
	{
		var arr(ARRAY_TYPE);
		int index = 0;
		i++;
		if (src[i] == ']') return arr;
		bool new_elem = true;
		while (new_elem)
		{
			arr[index++] = parse_json_value(src, i);

			if (src[i] == ',')
				i++;
			else
				new_elem = false;
		}
		i++;
		return arr;
	}
	
	var var::parse_json_object(const std::string& src, size_t& i)
	{
		var obj(var::MAP_TYPE);
		int index = 0;
		i++;
		if (src[i] == '}') return obj;
		bool new_member = true;
		while (new_member)
		{
			char label[128];
			char* pos = label;
		
			if (src[i] != '\"') return error("JSON: invalid label given for member at position: " + std::to_string(i));
			i++;
			while (src[i] != '\"')
			{
				*pos++ = src[i++];
			}
			i++;
			*pos = 0;

			if (i == src.size() || src[i] != ':') return error("JSON: stray string at position: " + std::to_string(i));
			i++;
			
			var& m = obj[label];
			m = parse_json_value(src, i);
			if (m.is_error()) return m;
			
			if (src[i] != ',')
			{
				new_member = false;
				if (src[i] != '}') return error("JSON: unexpected token '" + std::string(1, src[i]) + "' at position: " + std::to_string(i));
			}
			else
			{
				i++;
			}
		}

		return obj;
	}

	var var::parse_json(const std::string& src)
	{
		std::string src_mod(src.size(), 0);
		// removing all non vital white space
		size_t oi = 0;
		// maximum depth of 128 for json file 
		char pairs[128];
		char* pair = pairs;
		for (size_t i = 0; i < src.size(); i++)
		{
			if (src[i] < 33) continue;
			switch(src[i])
			{
			case '\"':
				src_mod[oi++] = src[i++];
				while (true)
				{
					if (i >= src.size()) return error("JSON: unterminated string at position: " + std::to_string(i - 1));
					// end of string
					if (src[i] == '\"' && src[i - 1] != '\\') break;
					src_mod[oi++] = src[i++];
				}
				break;

			case '[':
				*++pair = ']';
				break;

			case '{':
				*++pair = '}';
				break;

			case ']':
			case '}':
				if (*pair == src[i])
					pair--;
				else
					return error("JSON: stray '" + std::string(1, src[i]) + "' at position: " + std::to_string(i));
				break;
			}

			src_mod[oi++] = src[i];
		}

		src_mod.resize(oi);

		if (src_mod.empty()) return error("JSON: source string was empty");
		
		if (pair > pairs)
		{
			std::string name = *pair == ']' ? "array" : "object";
			return error("JSON: unterminated " + name + " definition");
		}


		size_t i = 0;

		return parse_json_value(src_mod, i);
	}
} // namespace hirzel

#endif // HIRZEL_VAR_IMPLEMENTATION
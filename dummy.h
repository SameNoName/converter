#include <string_view>
#include <vector>
#include <map>
#include <set>
#include "csv.h"
#include "gmpxx.h"

namespace udecoder
{
	class Strings
	{
		std::vector<std::string> lStringsVector;
		std::vector<std::string> rStringsVector;
		std::string l;
		std::string r;
		bool isSlash;
		int lCount;
		int rCount;

		int count(const std::string& s, char c)
		{
			if (s == "") return -1;
			int a = 0;
			for (int i = 0; i < s.length(); ++i)
			{
				if (s[i] == c) ++a;
			}
			return a;
		}

		std::string spaceRemover(const std::string& s1)
		{
			std::string s = s1;
			for (int i = 0; i < s.length(); ++i)
			{
				if (s[i] == ' ')
				{
					s.erase(i, 1);
					--i;
				}
			}
			return s;
		}

		std::vector<std::string> vectorMaker(const std::string& s) {
			std::vector<std::string> v;
			int vectorIterator = 0;
			v.push_back("");
			for (int i = 0; i < s.length(); ++i)
			{
				if (s[i] == '*')
				{
					v.push_back("");
					++vectorIterator;
				}
				else
				{
					v[vectorIterator].push_back(s[i]);
				}
			}
			return v;
		}

	public:
		Strings(const std::string& s1)
		{
			std::string s = spaceRemover(s1);
			if (count(s, '/') > 0) isSlash = 1;
			else isSlash = 0;
			if (isSlash) 
			{ 
				int a = s.find('/');
				l.append(s, 0, a); 
				r.append(s, a + 1, s.length() - a);
			}
			else 
			{
				l = s;
				r = "";
			}
			lCount = count(l, '*') + 1;
			rCount = count(r, '*') + 1;
			lStringsVector = vectorMaker(l);
			if (isSlash) rStringsVector = vectorMaker(r);
		}

		~Strings() {}

		int getLCount() { return lCount; }
		int getRCount() { return rCount; }
		std::string getString(bool a, int b) 
		{
			if (a) return rStringsVector[b];
			else return lStringsVector[b];
		};
	};




	class Converter
	{
		std::map <std::string, int> mp;
		std::map <std::string, int> ::iterator mapIt;
		std::set <std::string> chain;
		std::set <std::string> ::iterator setIt;
		std::string csv;
	public:
		Converter(std::map <std::string, int> m, std::string s)
		{
			mp = m;
			csv = s;
		}
		~Converter() {}
		mpf_class find(std::string str, mpf_class answer)
		{
			chain.insert(str);
			io::CSVReader<3> in(csv);
			in.read_header(io::ignore_extra_column, "S", "T", "Value");
			std::string S; std::string T; double Value;
			while (in.read_row(S, T, Value))
			{
				if (S == str)
				{
					mapIt = mp.find(T);
					setIt = chain.find(T);
					if (mapIt != mp.end() && mapIt->second > 0)
					{
						answer *= Value;
						mapIt->second -= 1;
						chain.clear();
						return answer;
					}
					else if (setIt == chain.end())
					{
						mpf_class buff = answer * Value;
						mpf_class buff1 = find(T, buff);
						if (buff1 != -400)
						{
							return buff1;
						}
					}
				}
				else if (T == str)
				{
					mapIt = mp.find(S);
					setIt = chain.find(S);
					if (mapIt != mp.end() && mapIt->second > 0)
					{
						answer /= Value;
						mapIt->second -= 1;
						chain.clear();
						return answer;
					}
					else if (setIt == chain.end())
					{
						mpf_class buff = answer / Value;
						mpf_class buff1 = find(S, buff);
						if (buff1 != -400)
						{
							return buff1;
						}
					}
				}
			}
			return -400;
		}
	};

	std::string convert(const std::string& from, const std::string& to, const std::string& csv);
	bool inCSV(const std::string& s, const std::string& csv);
}

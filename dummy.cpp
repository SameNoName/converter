#include "udecoder/dummy.h"

bool udecoder::inCSV(const std::string& s, const std::string& csv)
{
	io::CSVReader<3> in(csv);
	in.read_header(io::ignore_extra_column, "S", "T", "Value");
	std::string S; std::string T; double Value;
	while (in.read_row(S, T, Value)) {
		if (s == S || s == T) return 1;
	}
	return 0;
}

std::string udecoder::convert(const std::string& from, const std::string& to, const std::string& csv)
{
    udecoder::Strings obj1(from);
	udecoder::Strings obj2(to);
	std::map <std::string, int> nominator, denominator;
	std::map <std::string, int> ::iterator it, it1;
	bool notFoundCheck = 1;
	for (int i = 0; i < obj1.getLCount(); ++i)
	{
		std::string s = obj1.getString(0, i);
		if (!inCSV(s, csv)) return "-404";
		nominator[s] += 1;
	}
	for (int i = 0; i < obj1.getRCount(); ++i)
	{
		std::string s = obj1.getString(1, i);
		if (!inCSV(s, csv)) return "-404";
		denominator[s] += 1;
	}
	for (int i = 0; i < obj2.getLCount(); ++i)
	{
		std::string s = obj2.getString(0, i);
		if (!inCSV(s, csv)) return "-404";
		denominator[s] += 1;
	}
	for (int i = 0; i < obj2.getRCount(); ++i)
	{
		std::string s = obj2.getString(1, i);
		if (!inCSV(s, csv)) return "-404";
		nominator[s] += 1;
	}

	it = nominator.begin();
	for (; it != nominator.end(); ++it) 
	{
		it1 = denominator.find(it->first);
		if (it1 != denominator.end() && it->second > 0 && it1->second > 0)
		{
			it->second -= 1;
			it1->second -= 1;
		}
	}

	mpf_class answer{ 1 };
	Converter denom(denominator, csv);
	it = nominator.begin();
	while(it != nominator.end())
	{
		if (it->second == 0) 
		{ 
			it++;
			continue; 
		}
		mpf_class buff = denom.find(it->first, answer);
		if (buff != -400)
		{
			it->second -= 1;
			answer = buff;
		}
		else 
		{
			return "-400";
		}
	}
	mp_exp_t exp;
	std::string s = answer.get_str(exp, 10, 15);
	int size = s.size();
	int e = exp;
	if (size > e && e > 0) s.insert(e, ".");
	else if (size <= e)
	{
		for (int i = 0; i < e - size; ++i)
		{
			s += "0";
		}
	}
	else
	{
		std::string buff = "0.";
		for (int i = 0; i < -exp; ++i)
		{
			buff += "0";
		}
		s = buff + s;

	}
	return s;
}
// cme.cpp - parse CME Group futures and options file
/*
:!./cme < stleqt.20171108
*/
#include <cassert>
#include <cstring>
#include <ctime>
#include <iostream>
#include <string>
#include <tuple>
#include <variant>

using namespace std;

// Database
const char* create_futures = R"xyz(
	create table cme.futures (
		id integer primary key not null,
		date date not null,
		name text not null,
		month integer not null,
		year integer not null,
		open double,
		high double,
		low double,
		last double,
		sett double,
		vol bigint
	);
)xyz";

/*
          1         2         3         4         5
012345678901234567890123456789012345678901234567890123456789
                PRELIMINARY PRICES AS OF 11/08/17 03:45 PM (CST)
*/
inline tm timestamp(const string& line)
{
	tm tm;
	static const char* format = "%D %I:%M %p";

	memset(&tm, 0, sizeof(tm));
	assert (0 != strptime(&line[41], format, &tm));

	return tm;
}

inline char inst_type(const string& line)
{
	if (string::npos != line.find("CALL"))
		return 'C';
	if (string::npos != line.find("PUT"))
		return 'P';

	return 'F';
}

inline double parse_double(const string& line, size_t pos)
{
	double d;
	char* e;

	d = strtod(&line[pos], &e);
	if (&line[pos] == e) {
		d = numeric_limits<double>::quiet_NaN();
	}

cout << "parse_double: " << d << ":" << line.substr(pos, 12) << endl;


	return d;
}

/*
          1         2         3         4         5         6         7         8         9
01234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789
      |         |         |         |         |         |       |          |
STRIKE     OPEN      HIGH      LOW       LAST      SETT    CHGE     EST.VOL       SETT         VOL         INT
DEC17   2582.20   2592.20B  2580.00   2591.20   2590.90   +4.10        2368    2586.80        3023       62200
*/
inline auto parse(const string& line)
{
	double open = parse_double(line, 6);
	double high = parse_double(line, 16);
	double low  = parse_double(line, 26);
	double last = parse_double(line, 36);
	double sett = parse_double(line, 46);
	double vol  = parse_double(line, 64);

	return make_tuple(open, high, low, last, sett, vol);
}

int main()
{
	string line;

	getline(cin, line);
	tm tm = timestamp(line);
	//puts (asctime(&tm));

//MTH/                 ---- DAILY ---                        PT                     -------  PRIOR  DAY  -------
//STRIKE     OPEN      HIGH      LOW       LAST      SETT    CHGE     EST.VOL       SETT         VOL         INT
	// skip two lines
	getline(cin, line);
	assert (0 == line.compare(0, 3, "MTH"));
	getline(cin, line);
	assert (0 == line.compare(0, 6, "STRIKE"));

	string inst;
	char type;
	while (getline(cin, line)) {
cout << ">" << line << "<\n";
		if (line.length() < 86) {
			if (0 == line.compare(0, 5, "TOTAL"))
				continue;
			if (0 == line.compare(0, 3, "***"))
				continue;
			inst = line;	
			type = inst_type(inst);
cout << "inst: " << inst << endl;

			continue;
		}

		if (type == 'F') {
			string month = line.substr(0,5); // MMMYY
			auto [open, high, low, last, sett, vol] = parse(line);
cout << inst.substr(0,10) << "," << type << "," << open << endl;
		}
		else {
			unsigned long strike = stoul(line);
			auto [open, high, low, last, sett, vol] = parse(line);
cout << inst.substr(0,10) << "," << type << "," << open << endl;
		}
	}

	return 0;
}

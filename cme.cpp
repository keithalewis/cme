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

#include <sqlite3.h>

namespace sqlite {
	class db {
		sqlite3* pdb;
	public:
		db(const char* file)
		{
			assert (SQLITE_OK == sqlite3_open(file, &pdb));
		}
		db(const db&) = delete;
		db& operator=(const db&) = delete;
		~db()
		{
			sqlite3_close(pdb);
		}
		operator sqlite3*() {
			return pdb;
		}
		int exec(const char* sql, int(*cb)(void*, int, char**, char**) = 0)
		{
			return sqlite3_exec(pdb, sql, cb, 0, 0);
		}
		class stmt {
			sqlite3_stmt* pstmt;
		public:
			stmt()
				: pstmt(nullptr)
			{ }
			stmt(const stmt&) = default;
			stmt& operator=(const stmt&) = default;
			~stmt()
			{
				if (pstmt != nullptr)
					sqlite3_finalize(pstmt);
			}
			sqlite3_stmt** operator&()
			{
				return &pstmt;
			}
			int reset()
			{
				return sqlite3_reset(pstmt);
			}
			int step()
			{
				return sqlite3_step(pstmt);
			}
			int bind(int col, int i)
			{
				return sqlite3_bind_int(pstmt, col, i);
			}
			int bind(int col, sqlite_int64 i)
			{
				return sqlite3_bind_int64(pstmt, col, i);
			}
			int bind(int col, double d)
			{
				return sqlite3_bind_double(pstmt, col, d);
			}
			int bind(int col, const char* t, int n = -1)
			{
				return sqlite3_bind_text(pstmt, col, t, n, SQLITE_STATIC);
			}
		};
		stmt prepare(const char* sql, int nsql = -1)
		{
			stmt stmt;
			assert (SQLITE_OK == sqlite3_prepare_v2(pdb, sql, nsql, &stmt, 0));

			return std::move(stmt);
		}
	};
}

int callback(void *, int argc, char **argv, char **argn) {
	for (int i = 0; i < argc; ++i) {
		std::cout << argn[i] << "=" << argv[i] << std::endl;
	}

	return 0;
}

void test_sqlite()
{
	int result;
	sqlite::db db("tmp.db");
	result = db.exec("create table tbl1(one varchar(10), two smallint)");
	result = db.exec("insert into tbl1 values('hello!',10);");
	result = db.exec("insert into tbl1 values('goodbye',20);");
	result = db.exec("select * from tbl1", callback);
}

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
	test_sqlite();
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
		if (0 == line.compare(0, 5, "TOTAL"))
			continue;
		if (0 == line.compare(0, 3, "***"))
			continue;

		string symbol = line.substr(0, line.find(' '));
		string mmmyy;
		if (line.length() < 86) {
			type = inst_type(line);
cout << "inst: " << symbol << endl;
			if (type == 'C' || type == 'P') {
				auto b = 1 + line.find(' ');
				auto e = line.find(' ', b);
				mmmyy = line.substr(b, e - b);
cout << "mmmyy: " << mmmyy << endl;
			}

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

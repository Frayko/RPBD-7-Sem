#include "SouvenirDataMapper.h"

SouvenirDataMapper::SouvenirDataMapper() {
	int rtc = initDB();
	if (rtc == 1) {
		cout << "Connection succesful" << endl;
		createTables();
		loadSouvenirs();
	}
	else {
		cout << "Connection UNsuccesful: " << rtc << endl;
	}
}

SouvenirDataMapper::~SouvenirDataMapper() {
	int rtc = disconnectDB();
	if (rtc == 2) {
		cout << "Disconnect succesful" << endl;
	}
	else {
		cout << "Disconnect UNsuccesful: " << rtc << endl;
	}
}

vector<Souvenir> SouvenirDataMapper::getSouvenirs() {
	return this->souvenirs;
}

void SouvenirDataMapper::loadSouvenirs() {
	int szId = -1, szIdBefore = -1;
	double bufPrice = -1, szPrice = -1;
	char bufName[256] = "", szName[256] = "", szTag[65] = "";
	vector<string> tags;

	retcode = SQLExecDirect(hstmt,
		(SQLWCHAR*)L"SELECT souvenirs.id, souvenirs.name, souvenirs.price, t.name FROM souvenirs "
		"INNER JOIN souvenir_tags st ON st.id_souvenir = souvenirs.id "
		"INNER JOIN tags t ON t.id = st.id_tag "
		"ORDER BY souvenirs.id ASC",
		SQL_NTS);
	if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO) {
		retcode = SQLBindCol(hstmt, 1, SQL_C_SLONG, &szId, 0, NULL);
		retcode = SQLBindCol(hstmt, 2, SQL_C_CHAR, szName, 255, NULL);
		retcode = SQLBindCol(hstmt, 3, SQL_C_DOUBLE, &szPrice, 0, NULL);
		retcode = SQLBindCol(hstmt, 4, SQL_C_CHAR, szTag, 64, NULL);

		for (int i = 0; ; i++) {
			retcode = SQLFetch(hstmt);
			if (retcode == SQL_ERROR || retcode == SQL_SUCCESS_WITH_INFO)
				cout << "Error in Fetch" << endl;
			if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO) {
				if (i == 0 || szId == szIdBefore) {
					if (bufPrice != szPrice)
						bufPrice = szPrice;
					if (strcmp(bufName, szName))
						strcpy_s(bufName, 255, szName);

					szIdBefore = szId;
					tags.push_back(szTag);
				}
				else {
					souvenirs.push_back(Souvenir(szIdBefore, bufName, bufPrice, tags));
					tags.clear();

					szIdBefore = szId;
					tags.push_back(szTag);
				}
				
			}
			else {
				if (tags.size() != 0) {
					souvenirs.push_back(Souvenir(szIdBefore, bufName, bufPrice, tags));
				}

				break;
			}
		}

		SQLFreeStmt(hstmt, SQL_CLOSE);
	}
	else {
		cout << "Error loading souvenirs" << endl;
	}
}

bool SouvenirDataMapper::insert(Souvenir souvenir) {
	if (!contains(souvenir)) {
		int bind_souvenir_id = -1, last_tag_id = -1;
		double price = souvenir.getPrice();
		char * name = new char[256], * tag = new char[65];
		strcpy_s(name, 255, souvenir.getName().c_str());

		retcode = SQLPrepare(hstmt, (SQLWCHAR*)L"INSERT INTO tags (name) VALUES (?);", SQL_NTS);
		for (string t : souvenir.getTags()) {
			strcpy_s(tag, 64, t.c_str());
			retcode = SQLBindParameter(hstmt, 1, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_CHAR, 64, 0, tag, 64, NULL);
			retcode = SQLExecute(hstmt);
		}
		retcode = SQLFreeStmt(hstmt, SQL_CLOSE);

		retcode = SQLExecDirect(hstmt, (SQLWCHAR*)L"SELECT max(souvenirs.id), max(tags.id) FROM souvenirs, tags;", SQL_NTS);
		retcode = SQLBindCol(hstmt, 1, SQL_C_SLONG, &bind_souvenir_id, 0, NULL);
		retcode = SQLBindCol(hstmt, 2, SQL_C_SLONG, &last_tag_id, 0, NULL);
		retcode = SQLFetch(hstmt);
		SQLFreeStmt(hstmt, SQL_CLOSE);

		++bind_souvenir_id;
		cout << bind_souvenir_id << endl;

		retcode = SQLPrepare(hstmt, (SQLWCHAR*)L"INSERT INTO souvenir_tags (id_souvenir, id_tag) VALUES (?,?);", SQL_NTS);
		for (int i = 1; i <= souvenir.getTags().size(); ++i) {
			retcode = SQLBindParameter(hstmt, 1, SQL_PARAM_INPUT, SQL_C_SLONG, SQL_INTEGER, 0, 0, &bind_souvenir_id, 0, NULL);
			int bind_tag_id = last_tag_id - souvenir.getTags().size() + i;
			retcode = SQLBindParameter(hstmt, 2, SQL_PARAM_INPUT, SQL_C_SLONG, SQL_INTEGER, 0, 0, &bind_tag_id, 0, NULL);
			retcode = SQLExecute(hstmt);
		}
		retcode = SQLFreeStmt(hstmt, SQL_CLOSE);

		retcode = SQLPrepare(hstmt, (SQLWCHAR*) L"INSERT INTO souvenirs (name, price) VALUES (?,?);", SQL_NTS);
		retcode = SQLBindParameter(hstmt, 1, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_CHAR, 255, 0, name, 255, NULL);
		retcode = SQLBindParameter(hstmt, 2, SQL_PARAM_INPUT, SQL_C_DOUBLE, SQL_DOUBLE, 0, 0, &price, 0, NULL);
		retcode = SQLExecute(hstmt);
		retcode = SQLFreeStmt(hstmt, SQL_CLOSE);

		souvenir.setId(bind_souvenir_id);

		delete[] name;
		delete[] tag;

		souvenirs.push_back(souvenir);

		return true;
	}
	else {
		cout << "Error! Same item is in souvenirs" << endl;
	}

	return false;
}

bool SouvenirDataMapper::remove(int index) {
	if (index >= 0 && index < souvenirs.size()) {
		int souvenir_id = souvenirs[index].getId(), deleted_tag_id = -1;
		vector<int> deleted_tags_id;

		retcode = SQLBindCol(hstmt, 1, SQL_C_SLONG, &deleted_tag_id, 0, NULL);
		retcode = SQLBindParameter(hstmt, 1, SQL_PARAM_INPUT, SQL_C_SLONG, SQL_INTEGER, 0, 0, &souvenir_id, 0, NULL);
		retcode = SQLExecDirect(hstmt, (SQLWCHAR*)L"SELECT souvenir_tags.id_tag FROM souvenir_tags "
			"INNER JOIN souvenirs s ON s.id = souvenir_tags.id_souvenir "
			"WHERE s.id = ?;",
			SQL_NTS);
		while (1) {
			retcode = SQLFetch(hstmt);
			if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO) {
				deleted_tags_id.push_back(deleted_tag_id);
			}
			else {
				break;
			}
		}
		SQLFreeStmt(hstmt, SQL_CLOSE);

		retcode = SQLPrepare(hstmt, (SQLWCHAR*)L"DELETE FROM souvenirs WHERE id = ?;", SQL_NTS);
		retcode = SQLBindParameter(hstmt, 1, SQL_PARAM_INPUT, SQL_C_SLONG, SQL_INTEGER, 0, 0, &souvenir_id, 0, NULL);
		retcode = SQLExecute(hstmt);
		retcode = SQLFreeStmt(hstmt, SQL_CLOSE);

		retcode = SQLPrepare(hstmt, (SQLWCHAR*)L"DELETE FROM souvenir_tags WHERE id_souvenir = ?;", SQL_NTS);
		retcode = SQLBindParameter(hstmt, 1, SQL_PARAM_INPUT, SQL_C_SLONG, SQL_INTEGER, 0, 0, &souvenir_id, 0, NULL);
		retcode = SQLExecute(hstmt);
		retcode = SQLFreeStmt(hstmt, SQL_CLOSE);

		retcode = SQLPrepare(hstmt, (SQLWCHAR*)L"DELETE FROM tags WHERE id = ?;", SQL_NTS);
		for (int i = 0; i < deleted_tags_id.size(); ++i) {
			retcode = SQLBindParameter(hstmt, 1, SQL_PARAM_INPUT, SQL_C_SLONG, SQL_INTEGER, 0, 0, &deleted_tags_id[i], 0, NULL);
			retcode = SQLExecute(hstmt);
		}
		retcode = SQLFreeStmt(hstmt, SQL_CLOSE);

		souvenirs.erase(souvenirs.begin() + index);

		return true;
	}
	else {
		cout << "Error! This item isn't in souvenirs" << endl;
	}

	return false;
}

bool SouvenirDataMapper::update(Souvenir souvenir) {
	for (int i = 0; i < souvenirs.size(); ++i) {
		if (souvenirs[i].getId() == souvenir.getId()) {
			int souvenir_id = souvenirs[i].getId(), last_tag_id = -1, deleted_tag_id = -1;
			vector<int> deleted_tags_id;
			double price = souvenir.getPrice();
			char* name = new char[256], * tag = new char[65];
			strcpy_s(name, 255, souvenir.getName().c_str());

			retcode = SQLBindCol(hstmt, 1, SQL_C_SLONG, &deleted_tag_id, 0, NULL);
			retcode = SQLBindParameter(hstmt, 1, SQL_PARAM_INPUT, SQL_C_SLONG, SQL_INTEGER, 0, 0, &souvenir_id, 0, NULL);
			retcode = SQLExecDirect(hstmt, (SQLWCHAR*)L"SELECT souvenir_tags.id_tag FROM souvenir_tags "
				"INNER JOIN souvenirs s ON s.id = souvenir_tags.id_souvenir "
				"WHERE s.id = ?;",
				SQL_NTS);
			while (1) {
				retcode = SQLFetch(hstmt);
				if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO) {
					deleted_tags_id.push_back(deleted_tag_id);
				}
				else {
					break;
				}
			}
			SQLFreeStmt(hstmt, SQL_CLOSE);

			retcode = SQLPrepare(hstmt, (SQLWCHAR*)L"DELETE FROM souvenir_tags WHERE id_souvenir = ?;", SQL_NTS);
			retcode = SQLBindParameter(hstmt, 1, SQL_PARAM_INPUT, SQL_C_SLONG, SQL_INTEGER, 0, 0, &souvenir_id, 0, NULL);
			retcode = SQLExecute(hstmt);
			retcode = SQLFreeStmt(hstmt, SQL_CLOSE);

			retcode = SQLPrepare(hstmt, (SQLWCHAR*)L"DELETE FROM tags WHERE id = ?;", SQL_NTS);
			for (int i = 0; i < deleted_tags_id.size(); ++i) {
				retcode = SQLBindParameter(hstmt, 1, SQL_PARAM_INPUT, SQL_C_SLONG, SQL_INTEGER, 0, 0, &deleted_tags_id[i], 0, NULL);
				retcode = SQLExecute(hstmt);
			}
			retcode = SQLFreeStmt(hstmt, SQL_CLOSE);

			retcode = SQLPrepare(hstmt, (SQLWCHAR*)L"INSERT INTO tags (name) VALUES (?)", SQL_NTS);
			for (string t : souvenir.getTags()) {
				strcpy_s(tag, 64, t.c_str());
				retcode = SQLBindParameter(hstmt, 1, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_CHAR, 64, 0, tag, 64, NULL);
				retcode = SQLExecute(hstmt);
			}
			retcode = SQLFreeStmt(hstmt, SQL_CLOSE);

			retcode = SQLExecDirect(hstmt, (SQLWCHAR*)L"SELECT max(id) FROM tags;", SQL_NTS);
			retcode = SQLBindCol(hstmt, 1, SQL_C_SLONG, &last_tag_id, 0, NULL);
			retcode = SQLFetch(hstmt);
			SQLFreeStmt(hstmt, SQL_CLOSE);

			retcode = SQLPrepare(hstmt, (SQLWCHAR*)L"INSERT INTO souvenir_tags (id_souvenir, id_tag) VALUES (?,?);", SQL_NTS);
			for (int i = 1; i <= souvenir.getTags().size(); ++i) {
				retcode = SQLBindParameter(hstmt, 1, SQL_PARAM_INPUT, SQL_C_SLONG, SQL_INTEGER, 0, 0, &souvenir_id, 0, NULL);
				int bind_tag_id = last_tag_id - souvenir.getTags().size() + i;
				retcode = SQLBindParameter(hstmt, 2, SQL_PARAM_INPUT, SQL_C_SLONG, SQL_INTEGER, 0, 0, &bind_tag_id, 0, NULL);
				retcode = SQLExecute(hstmt);
			}
			retcode = SQLFreeStmt(hstmt, SQL_CLOSE);

			retcode = SQLPrepare(hstmt, (SQLWCHAR*)L"UPDATE souvenirs SET name = ?, price = ? WHERE id = ?", SQL_NTS);
			retcode = SQLBindParameter(hstmt, 1, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_CHAR, 255, 0, name, 255, NULL);
			retcode = SQLBindParameter(hstmt, 2, SQL_PARAM_INPUT, SQL_C_DOUBLE, SQL_DOUBLE, 0, 0, &price, 0, NULL);
			retcode = SQLBindParameter(hstmt, 3, SQL_PARAM_INPUT, SQL_C_SLONG, SQL_INTEGER, 0, 0, &souvenir_id, 0, NULL);
			retcode = SQLExecute(hstmt);
			retcode = SQLFreeStmt(hstmt, SQL_CLOSE);

			delete[] name;

			souvenirs[i] = souvenir;

			return true;
		}
	}

	return false;
}

vector<Souvenir> SouvenirDataMapper::findAll() {
	vector<Souvenir> s;
	int szId = -1, szIdBefore = -1;
	double bufPrice = -1, szPrice = -1;
	char bufName[256] = "", szName[256] = "", szTag[65] = "";
	vector<string> tags;

	retcode = SQLExecDirect(hstmt,
		(SQLWCHAR*)L"SELECT souvenirs.id, souvenirs.name, souvenirs.price, t.name FROM souvenirs "
		"INNER JOIN souvenir_tags st ON st.id_souvenir = souvenirs.id "
		"INNER JOIN tags t ON t.id = st.id_tag "
		"ORDER BY souvenirs.id ASC",
		SQL_NTS);
	if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO) {
		retcode = SQLBindCol(hstmt, 1, SQL_C_SLONG, &szId, 0, NULL);
		retcode = SQLBindCol(hstmt, 2, SQL_C_CHAR, szName, 255, NULL);
		retcode = SQLBindCol(hstmt, 3, SQL_C_DOUBLE, &szPrice, 0, NULL);
		retcode = SQLBindCol(hstmt, 4, SQL_C_CHAR, szTag, 64, NULL);

		for (int i = 0; ; i++) {
			retcode = SQLFetch(hstmt);
			if (retcode == SQL_ERROR || retcode == SQL_SUCCESS_WITH_INFO)
				cout << "Error in Fetch" << endl;
			if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO) {
				if (i == 0 || szId == szIdBefore) {
					if (bufPrice != szPrice)
						bufPrice = szPrice;
					if (strcmp(bufName, szName))
						strcpy_s(bufName, 255, szName);

					szIdBefore = szId;
					tags.push_back(szTag);
				}
				else {
					s.push_back(Souvenir(szIdBefore, bufName, bufPrice, tags));
					tags.clear();

					szIdBefore = szId;
					tags.push_back(szTag);
				}

			}
			else {
				if (tags.size() != 0) {
					s.push_back(Souvenir(szIdBefore, bufName, bufPrice, tags));
				}

				break;
			}
		}

		SQLFreeStmt(hstmt, SQL_CLOSE);
	}

	return s;
}

Souvenir* SouvenirDataMapper::find(int index) {
	if (index >= 0 && index < souvenirs.size()) {
		int szId = -1, souvenir_id = souvenirs[index].getId();
		double bufPrice = -1, szPrice = -1;
		char bufName[256] = "", szName[256] = "", szTag[65] = "";
		vector<string> tags;
		Souvenir* souvenir = nullptr;

		retcode = SQLPrepare(hstmt,
			(SQLWCHAR*)L"SELECT souvenirs.id, souvenirs.name, souvenirs.price, t.name FROM souvenirs "
			"INNER JOIN souvenir_tags st ON st.id_souvenir = souvenirs.id "
			"INNER JOIN tags t ON t.id = st.id_tag "
			"WHERE souvenirs.id = ?",
			SQL_NTS);
		retcode = SQLBindCol(hstmt, 1, SQL_C_SLONG, &szId, 0, NULL);
		retcode = SQLBindCol(hstmt, 2, SQL_C_CHAR, szName, 255, NULL);
		retcode = SQLBindCol(hstmt, 3, SQL_C_DOUBLE, &szPrice, 0, NULL);
		retcode = SQLBindCol(hstmt, 4, SQL_C_CHAR, szTag, 64, NULL);
		retcode = SQLBindParameter(hstmt, 1, SQL_PARAM_INPUT, SQL_C_SLONG, SQL_INTEGER, 0, 0, &souvenir_id, 0, NULL);
		retcode = SQLExecute(hstmt);

		if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO) {
			for (int i = 0; ; i++) {
				retcode = SQLFetch(hstmt);
				if (retcode == SQL_ERROR || retcode == SQL_SUCCESS_WITH_INFO)
					cout << "Error in Fetch" << endl;
				if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO) {
					if (i == 0 || szId == souvenir_id) {
						if (bufPrice != szPrice)
							bufPrice = szPrice;
						if (strcmp(bufName, szName))
							strcpy_s(bufName, 255, szName);

						tags.push_back(szTag);
					}

				}
				else {
					if (bufPrice != -1)
						souvenir = new Souvenir(souvenir_id, bufName, bufPrice, tags);

					break;
				}
			}
		}

		SQLFreeStmt(hstmt, SQL_CLOSE);

		return souvenir;
	}
	else {
		return nullptr;
	}
}

int SouvenirDataMapper::find(int index, double M) {
	if (index >= 0 && index < souvenirs.size()) {
		int szId = -1, souvenir_id = souvenirs[index].getId();
		double bufPrice = -1, szPrice = -1;
		char bufName[256] = "", szName[256] = "", szTag[65] = "";
		vector<string> tags;

		retcode = SQLPrepare(hstmt,
			(SQLWCHAR*)L"SELECT souvenirs.price FROM souvenirs "
			"WHERE souvenirs.id = ?",
			SQL_NTS);
		retcode = SQLBindCol(hstmt, 1, SQL_C_DOUBLE, &szPrice, 0, NULL);
		retcode = SQLBindParameter(hstmt, 1, SQL_PARAM_INPUT, SQL_C_SLONG, SQL_INTEGER, 0, 0, &souvenir_id, 0, NULL);
		retcode = SQLExecute(hstmt);
		retcode = SQLFetch(hstmt);
		SQLFreeStmt(hstmt, SQL_CLOSE);

		int count = 0;
		while(M > 0) {
			M -= szPrice;
			if (M > 0 || M == 0)
				++count;
		}

		return count;
	}
	else {
		return -1;
	}
}

Souvenir* SouvenirDataMapper::find(vector<string> t) {
	int szId = -1, szIdBefore = -1;
	double bufPrice = -1, szPrice = -1;
	char bufName[256] = "", szName[256] = "", szTag[65] = "";
	vector<string> tags;

	retcode = SQLExecDirect(hstmt,
		(SQLWCHAR*)L"SELECT souvenirs.id, souvenirs.name, souvenirs.price, t.name FROM souvenirs "
		"INNER JOIN souvenir_tags st ON st.id_souvenir = souvenirs.id "
		"INNER JOIN tags t ON t.id = st.id_tag "
		"ORDER BY souvenirs.id ASC",
		SQL_NTS);
	if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO) {
		retcode = SQLBindCol(hstmt, 1, SQL_C_SLONG, &szId, 0, NULL);
		retcode = SQLBindCol(hstmt, 2, SQL_C_CHAR, szName, 255, NULL);
		retcode = SQLBindCol(hstmt, 3, SQL_C_DOUBLE, &szPrice, 0, NULL);
		retcode = SQLBindCol(hstmt, 4, SQL_C_CHAR, szTag, 64, NULL);

		for (int i = 0; ; i++) {
			retcode = SQLFetch(hstmt);
			if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO) {
				if (i == 0 || szId == szIdBefore) {
					if (bufPrice != szPrice)
						bufPrice = szPrice;
					if (strcmp(bufName, szName))
						strcpy_s(bufName, 255, szName);

					szIdBefore = szId;
					tags.push_back(szTag);
				}
				else {
					int count = 0;
					for (int i = 0, isFind = false; i < t.size(); isFind = false, ++i) {
						for (int j = 0; !isFind && j < tags.size(); ++j) {
							if (tags[i] == t[j]) {
								isFind = true;
								++count;
							}
						}
						if (!isFind)
							break;
					}

					if (count == t.size()) {
						SQLFreeStmt(hstmt, SQL_CLOSE);
						return new Souvenir(szIdBefore, bufName, bufPrice, tags);
					}

					tags.clear();

					szIdBefore = szId;
					tags.push_back(szTag);
				}
			}
			else {
				int count = 0;
				for (int i = 0, isFind = false; i < t.size(); isFind = false, ++i) {
					for (int j = 0; !isFind && j < tags.size(); ++j) {
						if (tags[i] == t[j]) {
							isFind = true;
							++count;
						}
					}
					if (!isFind)
						break;
				}

				if (count == t.size()) {
					SQLFreeStmt(hstmt, SQL_CLOSE);
					return new Souvenir(szIdBefore, bufName, bufPrice, tags);
				}


				break;
			}
		}

		SQLFreeStmt(hstmt, SQL_CLOSE);
	}

	return nullptr;
}

bool SouvenirDataMapper::contains(Souvenir B) {
	for (Souvenir souvenir : souvenirs) {
		if (souvenir == B)
			return true;
	}
	
	return false;
}

bool SouvenirDataMapper::contains(int id) {
	for (Souvenir souvenir : souvenirs) {
		if (souvenir.contains(id))
			return true;
	}

	return false;
}

int SouvenirDataMapper::initDB() {
	retcode = SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &henv);
	if (retcode < 0)
		return -1;
	retcode = SQLSetEnvAttr(henv, SQL_ATTR_ODBC_VERSION, (void*)SQL_OV_ODBC3, SQL_NTS);
	if (retcode < 0)
		return -2;
	retcode = SQLAllocHandle(SQL_HANDLE_DBC, henv, &hdbc);
	if (retcode < 0)
		return -3;

	retcode = SQLConnect(hdbc, (SQLWCHAR*)L"test", SQL_NTS,
		(SQLWCHAR*)L"test", SQL_NTS, (SQLWCHAR*)L"123", SQL_NTS);

	if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO) {
		retcode = SQLAllocHandle(SQL_HANDLE_STMT, hdbc, &hstmt);
		if (retcode < 0)
			return -6;

		return 1;
	}
	if (retcode < 0)
		return -4;

	return -5;
}

int SouvenirDataMapper::createTables() {
	retcode = SQLExecDirect(hstmt, (SQLWCHAR*)L"CREATE TABLE IF NOT EXISTS souvenirs \
		(id SERIAL,\
		name varchar(255),\
		price double,\
		CONSTRAINT pk_id PRIMARY KEY(id),\
		)\
	ALTER TABLE souvenirs\
		OWNER TO test; ", SQL_NTS);
	if (retcode < 0)
		return -7;

	retcode = SQLExecDirect(hstmt, (SQLWCHAR*)L"CREATE TABLE IF NOT EXISTS souvenir_tags \
		(id_souvenir integer,\
		id_tag integer,\
		)\
	ALTER TABLE souvenir_tags\
		OWNER TO test; ", SQL_NTS);
	if (retcode < 0)
		return -8;

	retcode = SQLExecDirect(hstmt, (SQLWCHAR*)L"CREATE TABLE IF NOT EXISTS tags \
		(id SERIAL,\
		name varchar(64),\
		CONSTRAINT pk_id PRIMARY KEY(id),\
		)\
	ALTER TABLE tags\
		OWNER TO test; ", SQL_NTS);
	if (retcode < 0)
		return -9;

	return 1;
}

int SouvenirDataMapper::disconnectDB() {
	retcode = SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
	if (retcode < 0)
		return -21;
	retcode = SQLDisconnect(hdbc);
	if (retcode < 0)
		return -22;
	retcode = SQLFreeHandle(SQL_HANDLE_DBC, hdbc);
	if (retcode < 0)
		return -23;
	retcode = SQLFreeHandle(SQL_HANDLE_ENV, henv);
	if (retcode < 0)
		return -24;

	return 2;
}
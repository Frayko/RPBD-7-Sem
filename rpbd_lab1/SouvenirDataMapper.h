#pragma once
#include <iostream>
#include <windows.h>
#include <odbcinst.h> 
#include <sqlext.h>
#include <vector>
#include "Souvenir.h"

using namespace std;

class SouvenirDataMapper {
private:
	vector<Souvenir> souvenirs;

	SQLHENV		henv;     	// Дескриптор окружения
	SQLHDBC		hdbc; 		// Дескриптор соединения
	SQLHSTMT	hstmt; 		// Дескриптор оператора
	SQLRETURN	retcode; 	// Код возврата

	bool contains(Souvenir B);
	bool contains(int id);
	int createTables();
	int initDB();
	int disconnectDB();
	void loadSouvenirs();

public:
	SouvenirDataMapper();
	~SouvenirDataMapper();
	vector<Souvenir> getSouvenirs();
	bool insert(Souvenir souvenir);
	bool remove(int index);
	bool update(Souvenir souvenir);
	vector<Souvenir> findAll();
	Souvenir* find(int index);
	int find(int index, double M);
	Souvenir* find(vector<string> tags);
};
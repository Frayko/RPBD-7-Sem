#pragma once
#include <iostream>
#include <string>
#include <vector>

using namespace std;

class Souvenir {
private:
	int id;
	string name;
	double price;
	vector<string> tags;

public:
	Souvenir(int id, string name, double price);
	Souvenir(int id, string name, double price, vector<string> tags);
	Souvenir(const Souvenir& s);
	~Souvenir();

	void setId(int id);
	int getId();
	void setName(string name);
	string getName();
	void setPrice(double price);
	double getPrice();
	void setTags(vector<string> tags);
	vector<string> getTags();

	void print();
	void print(int number);
	bool contains(int id);
	bool operator==(Souvenir& B);
	bool operator!=(Souvenir& B);
};
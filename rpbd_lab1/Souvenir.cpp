#include "Souvenir.h"

Souvenir::Souvenir(int id, string name, double price) :
	id(id), name(name), price(price) {}

Souvenir::Souvenir(int id, string name, double price, vector<string> tags) :
	id(id), name(name), price(price), tags(tags) {}

Souvenir::Souvenir(const Souvenir& s) {
	id = s.id;
	name = s.name;
	price = s.price;
	tags = s.tags;
}

Souvenir::~Souvenir() {}

void Souvenir::setId(int id) {
	this->id = id;
}

int Souvenir::getId() {
	return this->id;
}

void Souvenir::setName(string name) {
	this->name = name;
}

string Souvenir::getName() {
	return this->name;
}

void Souvenir::setPrice(double price) {
	this->price = price;
}

double Souvenir::getPrice() {
	return this->price;
}

void Souvenir::setTags(vector<string> tags) {
	this->tags = tags;
}

vector<string> Souvenir::getTags() {
	return this->tags;
}

void Souvenir::print() {
	cout << "Souvenir" << endl
		<< "Name: " << this->name << endl
		<< "Price: " << this->price << endl
		<< "Tags: ";
	for (string tag : tags)
		cout << tag << " ";
	cout << endl;
}

void Souvenir::print(int number) {
	cout << "Souvenir #" << number << "" << endl
		<< "Name: " << this->name << endl
		<< "Price: " << this->price << endl
		<< "Tags: ";
	for (string tag : tags)
		cout << tag << " ";
	cout << endl << endl;
}

bool Souvenir::contains(int id) {
	return this->id == id;
}

bool Souvenir::operator==(Souvenir& B) {
	return name == B.name &&
		price == B.price &&
		tags == B.tags;
}

bool Souvenir::operator!=(Souvenir& B) {
	return name != B.name ||
		price != B.price ||
		tags != B.tags;
}
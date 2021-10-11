#include <iostream>
#include <windows.h>
#include <odbcinst.h> 
#include <sqlext.h>
#include "SouvenirDataMapper.h"
#include "Souvenir.h"

using namespace std;

void printMenu() {
	cout << "Menu: " << endl
		<< "[1] insert" << endl
		<< "[2] delete" << endl
		<< "[3] update" << endl
		<< "[4] find all" << endl
		<< "[5] find by number in list" << endl
		<< "[6] find by N and M" << endl
		<< "[7] find by tags" << endl
		<< "[0] exit" << endl << endl << "> ";
}

int main() {
	SouvenirDataMapper souvenirDataMapper;

	int input;

	while (1) {
		system("cls");
		int i = 1;
		for (Souvenir souvenir : souvenirDataMapper.getSouvenirs()) {
			souvenir.print(i);
			++i;
		}
		printMenu();

		cin >> input;

		switch (input)
		{
		case 1: {
			cout << "Insert" << endl;
			double price;
			char name[255];
			string tag;
			vector<string> tags;
			cout << "Name: ";
			cin >> name;
			cout << "Price: ";
			cin >> price;
			cout << "Tags (0 - exit): ";
			while (1) {
				cin >> tag;

				if (!strcmp(tag.c_str(), "0"))
					break;
				else
					tags.push_back(tag);
			}

			Souvenir souvenir(-1, name, price, tags);
			cout << "Result: " << souvenirDataMapper.insert(souvenir) << endl;

			system("pause");
			break;
		}
		case 2: {
			cout << "Delete" << endl;
			int number;
			cout << "Write number of souvenirs in list: ";
			cin >> number;
			cout << "Result: " << souvenirDataMapper.remove(number - 1) << endl;

			system("pause");
			break;
		}

		case 3: {
			cout << "Update" << endl;
			int number;
			cout << "Write number of souvenirs in list: ";
			cin >> number;

			if (number > 0 && number <= souvenirDataMapper.getSouvenirs().size()) {
				Souvenir souvenir = souvenirDataMapper.getSouvenirs()[number - 1];
				double price;
				string name, type, color, size, tag;
				vector<string> tags;
				cout << "Name: ";
				cin >> name;
				cout << "Price: ";
				cin >> price;
				cout << "Tags (0 - exit): ";
				while (1) {
					cin >> tag;

					if (!strcmp(tag.c_str(), "0"))
						break;
					else
						tags.push_back(tag);
				}

				souvenir.setName(name.substr(0, 255));
				souvenir.setPrice(price);
				souvenir.setTags(tags);

				souvenirDataMapper.update(souvenir);
			}
			else {
				cout << "Wrong number" << endl;
			}

			system("pause");
			break;
		}

		case 4: {
			cout << "Find All" << endl;
			vector<Souvenir> souvenirs = souvenirDataMapper.findAll();

			int i = 1;
			for (Souvenir souvenir : souvenirs) {
				souvenir.print(i);
				++i;
			}

			system("pause");
			break;
		}

		case 5: {
			cout << "Find by number in list" << endl;
			cout << "Number in list: " << endl;
			int number;
			cin >> number;

			cout << endl;
			Souvenir* souvenir = souvenirDataMapper.find(number - 1);
			if (souvenir)
				souvenir->print();
			else
				cout << "No results!" << endl;

			system("pause");
			break;
		}

		case 6: {
			cout << "Find by N and M" << endl;
			int number, N;
			double M;
			cout << "Number in list: ";
			cin >> number;
			cout << "M: ";
			cin >> M;

			cout << endl;
			cout << souvenirDataMapper.find(number - 1, M) << endl << endl;

			system("pause");
			break;
		}

		case 7: {
			cout << "Find by tags" << endl;
			string tag;
			vector<string> tags;

			cout << "Tags (0 - exit): ";
			while (1) {
				cin >> tag;

				if (!strcmp(tag.c_str(), "0"))
					break;
				else
					tags.push_back(tag);
			}

			cout << endl;
			Souvenir *souvenir = souvenirDataMapper.find(tags);
			if (souvenir)
				souvenir->print();
			else
				cout << "No results!" << endl;

			system("pause");
			break;
		}
		case 0: {
			return 0;
		}
		default:
			cout << "Wrong input number" << endl;
			system("pause");
		}
	}

	return 0;
}
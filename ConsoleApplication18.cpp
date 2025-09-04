#include <bits/stdc++.h>
#include <chrono>
#include <unordered_map>
#include <unordered_set>

#define all(a) a.begin(), a.end()

using namespace std;

class User {
protected:
	string name, userId, email;
	vector<string> borrowedBooks;
public:
	User(const string& name, const string& userId, const string& email) : 
	name(name), userId(userId), email(email)
	{};
	virtual ~User() = default;

	virtual int getMaxBooks() const = 0;
	virtual int getBorrowDays() const = 0;
	virtual double getFinePerDay() const = 0;

	bool canBorrow() const { return borrowedBooks.size() < getMaxBooks(); }
	
	const string& getName() const { return name; }
	const string& getUserId() const { return userId; }
	const string& getEmail() const { return email; }
	const vector<string>& borrowed() const { return borrowedBooks; }

	void AddIsnb(const string& isbn) { borrowedBooks.push_back(isbn); }
	bool EraseIsbn(const string& isbn) {
		if (find(all(borrowedBooks), isbn) != borrowedBooks.end()) {
			borrowedBooks.erase(find(all(borrowedBooks), isbn));
			return true;
		}
		return false;

	}
};
class Student : public User {
public:
	Student(const string& name, const string& userId, const string& email) :
		User(name, userId, email) {}
	int getMaxBooks() const override { return 3; }
	int getBorrowDays() const override { return 14; }
	double getFinePerDay() const override { return 1; }
};
class Guest : public User {
public:
	Guest(const string& name, const string& userId, const string& email) :
		User(name, userId, email) {}
	int getMaxBooks() const override { return 10; }
	int getBorrowDays() const override { return 30; }
	double getFinePerDay() const override { return 1; }
};
class Faculty : public User {
public:
	Faculty(const string& name, const string& userId, const string& email) :
		User(name, userId, email) {}
	int getMaxBooks() const override { return 1; }
	int getBorrowDays() const override { return 7; }
	double getFinePerDay() const override { return 1; }
};

class LibraryOperations {
private:
	class Book {
	private:
		string title, author, isbn, genre;
		bool possible;
	public:
		Book() : title(), author(), isbn(), genre(), possible(true) {}
		Book(const string& title, const string& author, const string& isbn, const string& genre):
			title(title), author(author), isbn(isbn), genre(genre), possible(true)
		{}
		bool getPossible() const { return possible; }
		void setPossible(bool another) { possible = another; }
		const string& getTitle() const { return title; }
		const string& getAuthor() const { return author; }
		const string& getIsbn() const { return isbn; }
		const string& getGenre() const { return genre; }

	};
	class BorrowingRecord {
	public:
		string userId, isbn;
		bool returned = false;
		chrono::system_clock::time_point st_;
		chrono::system_clock::time_point end_;
		BorrowingRecord(const string& userId, const string& isbn, chrono::system_clock::time_point st) :
			userId(userId), isbn(isbn), st_(st)
		{}
	};
	unordered_set<string> Genres;
	unordered_map<string, shared_ptr<User>> mapUser; 
	unordered_map<string, Book> mapBookIsbn; 
	vector <BorrowingRecord> history;
public:
	bool addBook(const string& title, const string& author, const string& isbn, const string& genre) {
		if (mapBookIsbn.count(isbn)) return false;
		Book book(title, author, isbn, genre);
		mapBookIsbn[isbn] = book;
		Genres.insert(genre);
		return true;
	}
	bool removeBook(const string& isbn) {
		if (mapBookIsbn.find(isbn) == mapBookIsbn.end()) return false;
		Book& book = mapBookIsbn[isbn];
		if (!book.getPossible()) return false;
		mapBookIsbn.erase(isbn);
		return true;
	}
	Book* findBook(const string& isbn) {
		if(mapBookIsbn.find(isbn) != mapBookIsbn.end())
			return &mapBookIsbn[isbn];
		return nullptr;
	}
	vector<Book*> searchBooks(const string& query) {
		vector <Book*> ans;
		for (auto& to : mapBookIsbn) {
			if (to.second.getAuthor() == query || to.second.getTitle() == query || to.second.getIsbn() == query)
				ans.emplace_back(&to.second);
		}
		return ans;
	}
	
	bool registerUser(const string& name, const string& userId, const string& email, const string& t) {
		if (mapUser.find(userId) != mapUser.end()) return false;
		string T = t;
		if (T == "student") {
			shared_ptr<User> ptr = make_shared<Student>(name, userId, email); 
			mapUser[userId] = ptr;
			return true;
		}
		if (T == "guest") {
			shared_ptr<User> ptr = make_shared<Guest>(name, userId, email);
			mapUser[userId] = ptr;
			return true;
		}
		if (T == "faculty") {
			shared_ptr<User> ptr = make_shared<Faculty>(name, userId, email);
			mapUser[userId] = ptr;
			return true;
		}
	}
	shared_ptr<User> findUser(const string& userId) {
		if (mapUser.find(userId) == mapUser.end()) return nullptr;
		return mapUser.find(userId)->second;
	}
	
	bool borrowBook(const string& userId, const string& isbn) {
		auto user = findUser(userId);
		auto book = findBook(isbn);
		if (user == nullptr || book == nullptr || !book->getPossible() || !user->canBorrow()) return false;
		book->setPossible(false);
		user->AddIsnb(isbn);
		history.emplace_back(userId, isbn, chrono::system_clock::now());

		return true;
	}
	bool returnBook(const string& userId, const string& isbn) {
		auto user = findUser(userId);
		auto book = findBook(isbn);
		if (user == nullptr || book == nullptr || !user->EraseIsbn(isbn)) return false;
		book->setPossible(true);
		for (auto ob = history.rbegin(); ob != history.rend(); ++ob) {
			if (ob->returned) continue;
			if (ob->isbn == isbn && ob->userId == userId) {
				ob->returned = true;
				ob->end_ = chrono::system_clock::now();
				return true;
			}
		}
		return true;
	}
	vector<BorrowingRecord> getOverdueBooks() const {
		vector<BorrowingRecord> ans;
		auto time_ = chrono::system_clock::now();
		for (const auto &to : history) {
			
			if (to.returned) continue;
			double difference = chrono::duration_cast<chrono::hours>(time_ - to.st_).count() / 24;
			if (difference > mapUser.find(to.userId)->second->getBorrowDays()) {
				ans.push_back(to);
			}
		}
		return ans;
	}	
	double totalDebt() const {
		vector<BorrowingRecord> ans = getOverdueBooks();
		double total = 0.0;
		auto time_ = chrono::system_clock::now();
		for (const auto& to : ans) {
			auto user = mapUser.find(to.userId)->second;
			double difference = chrono::duration_cast<chrono::hours>(time_ - to.st_).count() / 24;
			total += (difference - user->getBorrowDays()) * user->getFinePerDay();
		}
		return total;
	}

	vector<string> WriteGenres() const {
		vector <string> ans;
		for (const auto& to : Genres) ans.push_back(to);
		return ans;
	}
};

class LibraryConsole {
private:
	LibraryOperations lib_;
	void showMenu() {
		cout << "\n=== Library Management ===\n";
		cout << "1. Book Management\n";
		cout << "2. User Management\n";
		cout << "3. Borrowing Operations\n";
		cout << "0. Exit\n";
	}
	int getIntInput(const string& s) {
		while (true) {
			cout << s;
			string line;
			if (!(cin >> line)) return 0;
			try {
				size_t idx;
				int v = stoi(line, &idx);
				if (idx == line.size()) return v;
			}
			catch (...) {}
			cout << "Please enter a valid number.\n";
		}
	}

	void handleBookManagement() {
		cout << "\n==Book Management==\n";
		cout << "1. add Book\n";
		cout << "2. remove Book\n";
		cout << "3. find Book\n";
		cout << "4. search Books\n";
		cout << "5. write Genres\n";
		cout << "6. back\n";
		int choice = getIntInput("Enter choice: ");
		if (choice == 1) {
			string title; cout << "\nTitle: "; cin >> title;
			string author; cout << "\nAuthor: "; cin >> author;
			string isbn; cout << "\nISBN: "; cin >> isbn;
			string genre; cout << "\nGenre: "; cin >> genre;
			
			if (!lib_.addBook(title, author, isbn, genre)) { cout << "already exists\n"; }
			else cout << "\nthanks for Book.\n";

		}
		else if (choice == 2) {
			string isbn; cout << "\nISBN: "; cin >> isbn;
			if (lib_.removeBook(isbn)) cout << "Book removed.\n";
			else cout << "not found or currently borrowed.\n";
		}
		else if (choice == 3) {
			string isbn; cout << "ISBN: "; cin >> isbn;
			auto to = lib_.findBook(isbn);
			if (to == nullptr) { cout << "no found\n"; return; }
			cout << "author - " << to->getAuthor() << ' ' <<
				"genre - " << to->getGenre() << ' ' <<
				"isbn - " << to->getIsbn() << ' ' <<
				"title - " << to->getTitle() << ' ' <<
				"possible - " << (to->getPossible() ? "true" : "false") << '\n';

		}
		else if (choice == 4) {
			string s; cout << "cin >> "; cin >> s;
			auto vec = lib_.searchBooks(s);
			cout << vec.size() << '\n';
			for (auto& to : vec)
				cout << "author - " << to->getAuthor() << ' ' <<
				"genre - " << to->getGenre() << ' ' <<
				"isbn - " << to->getIsbn() << ' ' <<
				"title - " << to->getTitle() << ' ' <<
				"possible - " << (to->getPossible() ? "true" : "false") << '\n';

		}
		else if (choice == 5) {
			auto vec = lib_.WriteGenres();
			cout << "Genres list:\n";
			for (const auto& to : vec) cout << to << ' ';
			cout << '\n';
		}
		else if (choice == 6) return;
		else cout << "Invalid choice\n";	

	}
	void handleUserManagement() {
		cout << "\n==User Managment==\n";
		cout << "1. register User\n";
		cout << "2. find User\n";
		cout << "3. back\n";
		int choice = getIntInput("Enter choice: ");
		if (choice == 1) {
			string name; cout << "\nname: ";   cin >> name;
			string userId; cout << "\nuserId: "; cin >> userId;
			string email; cout << "\nemail: ";  cin >> email;
			string typeUser; cout << "\ntypeUser(student/guest/faculty)"; cin >> typeUser;
			if (lib_.registerUser(name, userId, email, typeUser)) cout << "\nthanks for register\n";
			else cout << "\nalready exists\n";
		}
		else if (choice == 2) {
			string userId; cout << "userId: "; cin >> userId;
			auto to = lib_.findUser(userId);
			if (to == nullptr) cout << "\nno found\n";
			else {
				cout << "userId - " << to->getUserId() << ' ' <<
					"email - " << to->getEmail() << ' ' <<
					"name - " << to->getName() << ' ' <<
					"borrowed - " << to->borrowed().size() << '\n';
				if (to->borrowed().size()) {
					cout << "borrowed list: ";
					for (auto t : to->borrowed()) cout << t << ' ';
					cout << '\n';
				}
			}

		}
		else if (choice == 3) return;
		else cout << "Invalid choice\n";
	}
	void handleBorrowing() {
		cout << "\n==BorrowingManagement==\n";
		cout << "1. borrow Book\n";
		cout << "2. return Book\n";
		cout << "3. getOverdueBooks\n";
		cout << "4. total Delt\n";
		cout << "5. back\n";
		int choice = getIntInput("Enter choice: ");
		if (choice == 1) {
			string userId; cout << "userId: "; cin >> userId;
			string isbn; cout << "isbn: "; cin >> isbn;
			if (!lib_.borrowBook(userId, isbn)) cout << "no exists user or book or can not borrow\n";
			else cout << "nice!\n";
		}
		else if (choice == 2) {
			string userId; cout << "userId: "; cin >> userId;
			string isbn; cout << "isbn: "; cin >> isbn;
			if (!lib_.returnBook(userId, isbn)) cout << "no exists user or book or already erase\n";
			else cout << "nice!\n";
		}
		else if (choice == 3) {
			auto vec = lib_.getOverdueBooks();
			auto now = chrono::system_clock::now();
			for (const auto& to : vec) {
				cout << "userId - " << to.userId << ' ' <<
					"isbn - " << to.isbn << ' ' <<
					"between days" << chrono::duration_cast<chrono::hours>(now - to.st_).count() / 24 << '\n';
			}
		}
		else if (choice == 4) {
			cout << lib_.totalDebt() << '\n';
		}
		else if (choice == 5) return;
		else cout << "Invalid choice\n";
	}
public:
	LibraryConsole() = default;
	void run() {
		while (true) {
			showMenu();
			int choice = getIntInput("Enter choice: ");
			
			if (choice == 1) {
				handleBookManagement();
			}
			else if (choice == 2) {
				handleUserManagement(); 
			}
			else if (choice == 3) { 
				handleBorrowing(); 
			}
			else if (choice == 0) {
				return;
			}
			else cout << "Invalid choice\n";
		}
	}
	
};
int main() {
	LibraryConsole lc;
	lc.run();
    return 0;
}


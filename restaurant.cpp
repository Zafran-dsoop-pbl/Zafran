// =============================================================
//  PROJECT : Restaurant Management System (Zafran Kitchen)
//  COURSE  : Data Structures & Object-Oriented Programming (MTE-227)
//  GROUP   : Group 3 - 25MTE, Dept. of Mechatronics Eng., MUET Jamshoro
//  COMPILE : g++ -std=c++17 restaurant.cpp -lsqlite3 -o restaurant
// =============================================================
//  Lab-16 DS & OOP Concepts Implemented:
//    1. Classes & Objects (FoodItem, MenuItem, Order)
//    2. Single Inheritance (MenuItem : public FoodItem)
//    3. Polymorphism (virtual display() called via FoodItem* pointer)
//    4. Exception Handling (try, throw, catch for invalid price/qty)
//    5. File Handling (ofstream appends receipts to receipts.txt)
//    6. Data Structures (vector dynamic arrays & FIFO queue)
//    7. Algorithms (Linear Search O(n) & Bubble Sort O(n^2))
//    8. SQLite Database (restaurant.db - menu & live orders tables)
// =============================================================

#include <iostream>
#include <iomanip>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <queue>
#include <ctime>
#include <cstdlib>
#include <sqlite3.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>

using namespace std;

const double TAX_RATE = 0.05; // 5% tax

// -------------------------------------------------------------
//  1. OOP CLASSES (Inheritance, Polymorphism & Exception Handling)
// -------------------------------------------------------------
class FoodItem {
protected:
    int id;
    string name;
    double price;

public:
    FoodItem(int i = 0, string n = "", double p = 0.0) {
        if (p < 0) throw p; // exception if price is negative
        id = i;
        name = n;
        price = p;
    }

    int getId() const { return id; }
    string getName() const { return name; }
    double getPrice() const { return price; }

    // Virtual function for runtime polymorphism
    virtual void display() const {
        cout << left << setw(6) << id << setw(26) << name
             << right << setw(10) << fixed << setprecision(2) << price << "\n";
    }

    virtual ~FoodItem() {}
};

// Derived class inheriting from FoodItem
class MenuItem : public FoodItem {
private:
    string category;
    string description;

public:
    MenuItem(int i = 0, string n = "", string cat = "", string desc = "", double p = 0.0)
        : FoodItem(i, n, p) {
        category = cat;
        description = desc;
    }

    string getCategory() const { return category; }
    string getDescription() const { return description; }

    // Overriding base class virtual function
    void display() const override {
        cout << left << setw(6) << id
             << setw(24) << name
             << setw(18) << category
             << right << setw(10) << fixed << setprecision(2) << price << "\n";
    }
};

struct OrderLine {
    string name;
    double price;
    int quantity;
};

class Order {
private:
    vector<OrderLine> items;

public:
    void addLine(string dishName, double price, int qty) {
        if (qty <= 0) throw qty; // exception if quantity <= 0
        for (size_t i = 0; i < items.size(); i++) {
            if (items[i].name == dishName) {
                items[i].quantity += qty;
                return;
            }
        }
        items.push_back({dishName, price, qty});
    }

    bool removeLine(int lineNo) {
        if (lineNo < 1 || lineNo > (int)items.size()) return false;
        items.erase(items.begin() + (lineNo - 1));
        return true;
    }

    bool isEmpty() const { return items.empty(); }
    void clear() { items.clear(); }
    const vector<OrderLine>& getItems() const { return items; }

    double getSubtotal() const {
        double s = 0;
        for (size_t i = 0; i < items.size(); i++)
            s += items[i].price * items[i].quantity;
        return s;
    }
    double getTax() const { return getSubtotal() * TAX_RATE; }
    double getTotal() const { return getSubtotal() + getTax(); }
};

// -------------------------------------------------------------
//  2. GLOBAL DATA STRUCTURES
// -------------------------------------------------------------
vector<MenuItem> menuList;
Order currentOrder;
queue<string> kitchenQueue; // FIFO Queue for kitchen order tickets
sqlite3* db = NULL;

// -------------------------------------------------------------
//  3. HELPER FUNCTIONS
// -------------------------------------------------------------
string getNowDate() {
    time_t now = time(0);
    tm* t = localtime(&now);
    char buf[40];
    strftime(buf, sizeof(buf), "%d %b %Y, %H:%M", t);
    return string(buf);
}

string makeReceiptNo() {
    static int seq = 1;
    time_t now = time(0);
    tm* t = localtime(&now);
    char d[16];
    strftime(d, sizeof(d), "%y%m%d", t);
    return "ZF-" + string(d) + "-" + to_string(1000 + (now % 8000) + (seq++));
}

string cleanSql(string s) {
    string out = "";
    for (size_t i = 0; i < s.size(); i++) {
        if (s[i] == '\'') out += "''";
        else out += s[i];
    }
    return out;
}

string cleanJson(string s) {
    string out = "";
    for (size_t i = 0; i < s.size(); i++) {
        if (s[i] == '"') out += "\\\"";
        else if (s[i] == '\\') out += "\\\\";
        else if (s[i] == '\n' || s[i] == '\r') out += " ";
        else out += s[i];
    }
    return out;
}

int readInt(string prompt) {
    int v;
    while (true) {
        cout << prompt;
        if (cin >> v) return v;
        cin.clear();
        cin.ignore(1000, '\n');
        cout << "   !! Invalid input. Numbers only.\n";
    }
}

// -------------------------------------------------------------
//  4. SQLITE DATABASE & FILE HANDLING (Starts with 0 pre-filled orders!)
// -------------------------------------------------------------
void initDatabase() {
    sqlite3_open("restaurant.db", &db);

    const char* setupSql =
        "CREATE TABLE IF NOT EXISTS menu ("
        "  id INTEGER PRIMARY KEY, name TEXT, category TEXT, description TEXT, price REAL);"
        "CREATE TABLE IF NOT EXISTS orders ("
        "  id INTEGER PRIMARY KEY AUTOINCREMENT, receipt_no TEXT, customer_name TEXT,"
        "  phone TEXT, service_type TEXT, items_summary TEXT, subtotal REAL,"
        "  tax REAL, total REAL, notes TEXT, created_at TEXT);"
        "INSERT OR IGNORE INTO menu VALUES"
        "(1,'Chicken Biryani','From the kitchen','Long-grain basmati layered with saffron and bone-in chicken.',380),"
        "(2,'Beef Karahi','From the kitchen','Half a kilo of beef in tomato, ginger and black pepper.',950),"
        "(3,'Chicken Tikka','To begin','Yoghurt and red chilli marinade, charred on skewers.',520),"
        "(4,'Seekh Kebab (4 pcs)','To begin','Hand-minced beef, green chilli, fresh coriander.',420),"
        "(5,'Mutton Pulao','From the kitchen','Slow-braised mutton, yakhni rice, fried shallots.',680),"
        "(6,'Daal Makhani','From the kitchen','Black lentils simmered overnight with butter and cream.',320),"
        "(7,'Garlic Naan','Breads & sides','Tandoor naan brushed with garlic butter and coriander.',90),"
        "(8,'Fresh Salad','To begin','Cucumber, tomato, onion, lemon and chaat masala.',120),"
        "(9,'Kheer','Sweet & cold','Rice pudding with cardamom, pistachio and rose.',180),"
        "(10,'Soft Drink','Sweet & cold','Chilled bottle, regular or diet.',100);";

    sqlite3_exec(db, setupSql, NULL, NULL, NULL);
}

void loadMenuFromDB() {
    menuList.clear();
    sqlite3_stmt* stmt;
    const char* q = "SELECT id, name, category, description, price FROM menu ORDER BY id;";
    if (sqlite3_prepare_v2(db, q, -1, &stmt, NULL) == SQLITE_OK) {
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            menuList.push_back(MenuItem(
                sqlite3_column_int(stmt, 0),
                (const char*)sqlite3_column_text(stmt, 1),
                (const char*)sqlite3_column_text(stmt, 2),
                (const char*)sqlite3_column_text(stmt, 3),
                sqlite3_column_double(stmt, 4)
            ));
        }
    }
    sqlite3_finalize(stmt);
}

void saveOrderRecord(string rno, string guest, string phone, string serv,
                     string summary, double sub, double tax, double tot, string notes, string dt) {
    // 1. Insert into SQLite database (orders table)
    stringstream sql;
    sql << "INSERT INTO orders (receipt_no, customer_name, phone, service_type, "
        << "items_summary, subtotal, tax, total, notes, created_at) VALUES ('"
        << cleanSql(rno) << "','" << cleanSql(guest) << "','" << cleanSql(phone) << "','"
        << cleanSql(serv) << "','" << cleanSql(summary) << "',"
        << sub << "," << tax << "," << tot << ",'" << cleanSql(notes) << "','" << dt << "');";
    sqlite3_exec(db, sql.str().c_str(), NULL, NULL, NULL);

    // 2. Append to receipts.txt using C++ file handling (ofstream)
    ofstream file("receipts.txt", ios::app);
    if (file) {
        file << "========================================\n"
             << "ZAFRAN KITCHEN - RECEIPT (" << rno << ")\n"
             << "Date    : " << dt << "\n"
             << "Guest   : " << guest << " (" << phone << ")\n"
             << "Service : " << serv << "\n"
             << "Items   : " << summary << "\n"
             << fixed << setprecision(2)
             << "Subtotal: Rs. " << sub << " | Tax(5%): Rs. " << tax << "\n"
             << "TOTAL   : Rs. " << tot << "\n"
             << "========================================\n\n";
        file.close();
    }

    // 3. Push order into FIFO Kitchen Queue
    kitchenQueue.push(rno + " - " + guest);
}

// -------------------------------------------------------------
//  5. ALGORITHMS: LINEAR SEARCH O(n) & BUBBLE SORT O(n^2)
// -------------------------------------------------------------
int linearSearchById(int searchId) {
    for (size_t i = 0; i < menuList.size(); i++) {
        if (menuList[i].getId() == searchId) return (int)i;
    }
    return -1;
}

vector<MenuItem> getBubbleSortedMenu() {
    vector<MenuItem> arr = menuList;
    int n = (int)arr.size();
    for (int i = 0; i < n - 1; i++) {
        for (int j = 0; j < n - 1 - i; j++) {
            if (arr[j].getPrice() > arr[j + 1].getPrice()) {
                MenuItem temp = arr[j];
                arr[j] = arr[j + 1];
                arr[j + 1] = temp;
            }
        }
    }
    return arr;
}

// -------------------------------------------------------------
//  6. CONSOLE INTERFACE
// -------------------------------------------------------------
void showMenuConsole(const vector<MenuItem>& list, string heading) {
    cout << "\n============================================================\n  " << heading
         << "\n============================================================\n";
    cout << left << setw(6) << "No." << setw(24) << "Dish Name"
         << setw(18) << "Category" << right << setw(10) << "Price(Rs)\n"
         << "------------------------------------------------------------\n";
    for (size_t i = 0; i < list.size(); i++) {
        FoodItem* ptr = const_cast<MenuItem*>(&list[i]); // polymorphism via base pointer
        ptr->display();
    }
    cout << "============================================================\n";
}

void addDishConsole() {
    showMenuConsole(menuList, "ZAFRAN FOOD MENU");
    int id = readInt("Enter dish ID (0 to cancel): ");
    if (id == 0) return;

    int idx = linearSearchById(id);
    if (idx == -1) {
        cout << "   !! Dish ID not found.\n";
        return;
    }
    int qty = readInt("Enter quantity: ");
    try {
        currentOrder.addLine(menuList[idx].getName(), menuList[idx].getPrice(), qty);
        cout << "   Added " << qty << " x " << menuList[idx].getName() << "\n";
    } catch (int badQty) {
        cout << "   !! Error: Invalid quantity (" << badQty << ").\n";
    }
}

void checkoutConsole() {
    if (currentOrder.isEmpty()) {
        cout << "   !! Order is empty.\n";
        return;
    }
    cin.ignore();
    string guest, phone, tableNo;
    cout << "Customer Name : "; getline(cin, guest);
    cout << "Phone Number  : "; getline(cin, phone);
    cout << "Table No (or Enter for Pick-up): "; getline(cin, tableNo);
    string serv = tableNo.empty() ? "Pick-up" : ("Dine in (Table " + tableNo + ")");

    stringstream sum;
    const vector<OrderLine>& rows = currentOrder.getItems();
    cout << "\n------------------- ZAFRAN BILL -------------------\n";
    for (size_t i = 0; i < rows.size(); i++) {
        double amt = rows[i].price * rows[i].quantity;
        cout << left << setw(24) << rows[i].name << rows[i].quantity
             << " x " << rows[i].price << " = Rs. " << amt << "\n";
        if (i > 0) sum << ", ";
        sum << rows[i].quantity << " x " << rows[i].name;
    }
    double sub = currentOrder.getSubtotal();
    double tax = currentOrder.getTax();
    double tot = currentOrder.getTotal();
    cout << fixed << setprecision(2)
         << "Subtotal : Rs. " << sub << "  |  Tax (5%) : Rs. " << tax << "\n"
         << "TOTAL    : Rs. " << tot << "\n";

    string rno = makeReceiptNo();
    string dt = getNowDate();
    saveOrderRecord(rno, guest, phone, serv, sum.str(), sub, tax, tot, "", dt);
    currentOrder.clear();
    cout << "Saved Receipt " << rno << " into restaurant.db and receipts.txt!\n";
}

void showOrdersConsole() {
    cout << "\n============= SAVED ORDERS IN SQLITE (restaurant.db) =============\n";
    sqlite3_stmt* stmt;
    const char* q = "SELECT receipt_no, customer_name, service_type, total, created_at FROM orders ORDER BY id DESC;";
    int count = 0; double revenue = 0;
    if (sqlite3_prepare_v2(db, q, -1, &stmt, NULL) == SQLITE_OK) {
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            double t = sqlite3_column_double(stmt, 3);
            cout << left << setw(16) << (const char*)sqlite3_column_text(stmt, 0)
                 << setw(16) << (const char*)sqlite3_column_text(stmt, 1)
                 << setw(20) << (const char*)sqlite3_column_text(stmt, 2)
                 << "Rs. " << fixed << setprecision(2) << t << "\n";
            count++; revenue += t;
        }
    }
    sqlite3_finalize(stmt);
    if (count == 0) cout << "  No orders placed yet.\n";
    else cout << "------------------------------------------------------------------\n"
              << "Orders Completed: " << count << "  |  Total Sales: Rs. " << revenue << "\n";
}

// -------------------------------------------------------------
//  7. BUILT-IN C++ WEB SERVER (Serves Separate HTML Pages + SQLite API)
// -------------------------------------------------------------
string urlDecode(string s) {
    string out = "";
    for (size_t i = 0; i < s.size(); i++) {
        if (s[i] == '+') out += ' ';
        else if (s[i] == '%' && i + 2 < s.size()) {
            int hv = strtol(s.substr(i + 1, 2).c_str(), NULL, 16);
            out += (char)hv;
            i += 2;
        } else out += s[i];
    }
    return out;
}

string getFormField(const string& body, string key) {
    string pat = key + "=";
    size_t pos = body.find(pat);
    if (pos == string::npos) return "";
    size_t st = pos + pat.size();
    size_t en = body.find('&', st);
    if (en == string::npos) en = body.size();
    return urlDecode(body.substr(st, en - st));
}

string readFileText(string filename) {
    ifstream f(filename, ios::binary);
    if (!f) return "";
    stringstream s; s << f.rdbuf();
    return s.str();
}

void startWebServer(int port = 8080) {
    int serverFd = socket(AF_INET, SOCK_STREAM, 0);
    int opt = 1;
    setsockopt(serverFd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(port);

    if (::bind(serverFd, (sockaddr*)&addr, sizeof(addr)) < 0) {
        cout << "Error: Port " << port << " is busy.\n";
        return;
    }
    listen(serverFd, 10);
    cout << "Zafran C++ Web Backend running at http://localhost:" << port << "\n";

    while (true) {
        int clientFd = accept(serverFd, NULL, NULL);
        if (clientFd < 0) continue;

        char buffer[16384] = {0};
        int bytes = read(clientFd, buffer, sizeof(buffer) - 1);
        if (bytes <= 0) { close(clientFd); continue; }
        string req(buffer, bytes);

        // Parse HTTP method and path
        stringstream reqLine(req);
        string method, fullPath;
        reqLine >> method >> fullPath;
        string pathOnly = fullPath.substr(0, fullPath.find('?'));

        string contentType = "text/html; charset=utf-8";
        string body = "";

        if (method == "GET" && pathOnly == "/api/menu") {
            contentType = "application/json";
            bool sortPrice = (fullPath.find("sort=price") != string::npos);
            vector<MenuItem> list = sortPrice ? getBubbleSortedMenu() : menuList;
            stringstream js;
            js << "[";
            for (size_t i = 0; i < list.size(); i++) {
                if (i > 0) js << ",";
                js << "{\"id\":" << list[i].getId()
                   << ",\"name\":\"" << cleanJson(list[i].getName()) << "\""
                   << ",\"category\":\"" << cleanJson(list[i].getCategory()) << "\""
                   << ",\"description\":\"" << cleanJson(list[i].getDescription()) << "\""
                   << ",\"price\":" << list[i].getPrice() << "}";
            }
            js << "]";
            body = js.str();
        } else if (method == "GET" && pathOnly == "/api/orders") {
            contentType = "application/json";
            stringstream js;
            js << "[";
            sqlite3_stmt* stmt;
            const char* q = "SELECT receipt_no, customer_name, service_type, items_summary, total, created_at FROM orders ORDER BY id DESC;";
            bool first = true;
            if (sqlite3_prepare_v2(db, q, -1, &stmt, NULL) == SQLITE_OK) {
                while (sqlite3_step(stmt) == SQLITE_ROW) {
                    if (!first) js << ",";
                    first = false;
                    js << "{\"receipt_no\":\"" << cleanJson((const char*)sqlite3_column_text(stmt, 0)) << "\""
                       << ",\"customer_name\":\"" << cleanJson((const char*)sqlite3_column_text(stmt, 1)) << "\""
                       << ",\"service_type\":\"" << cleanJson((const char*)sqlite3_column_text(stmt, 2)) << "\""
                       << ",\"items_summary\":\"" << cleanJson((const char*)sqlite3_column_text(stmt, 3)) << "\""
                       << ",\"total\":" << fixed << setprecision(2) << sqlite3_column_double(stmt, 4)
                       << ",\"created_at\":\"" << cleanJson((const char*)sqlite3_column_text(stmt, 5)) << "\"}";
                }
            }
            sqlite3_finalize(stmt);
            js << "]";
            body = js.str();
        } else if (method == "POST" && pathOnly == "/api/order") {
            contentType = "application/json";
            size_t bPos = req.find("\r\n\r\n");
            string form = (bPos != string::npos) ? req.substr(bPos + 4) : "";

            Order webOrder;
            stringstream summary;
            for (size_t i = 0; i < menuList.size(); i++) {
                int id = menuList[i].getId();
                int q = atoi(getFormField(form, "qty_" + to_string(id)).c_str());
                if (q > 0) {
                    webOrder.addLine(menuList[i].getName(), menuList[i].getPrice(), q);
                    if (summary.tellp() > 0) summary << ", ";
                    summary << q << " x " << menuList[i].getName();
                }
            }

            string guest = getFormField(form, "guest_name");
            if (guest.empty()) guest = "Guest";
            string phone = getFormField(form, "phone");
            string serv = getFormField(form, "service");
            string tbl = getFormField(form, "table_no");
            string notes = getFormField(form, "notes");
            if (serv == "Dine in" && !tbl.empty()) serv += " (Table " + tbl + ")";

            string rno = makeReceiptNo();
            string dt = getNowDate();
            if (!webOrder.isEmpty()) {
                saveOrderRecord(rno, guest, phone, serv, summary.str(),
                                webOrder.getSubtotal(), webOrder.getTax(), webOrder.getTotal(), notes, dt);
            }
            body = "{\"status\":\"ok\",\"receipt_no\":\"" + rno + "\",\"created_at\":\"" + dt + "\"}";
        } else if (pathOnly.rfind("/images/", 0) == 0 && pathOnly.find("..") == string::npos) {
            // Serve dish pictures for the menu cards (e.g. /images/kheer.jpg)
            contentType = "image/jpeg";
            body = readFileText(pathOnly.substr(1));
        } else if (pathOnly == "/style.css") {
            contentType = "text/css";
            body = readFileText("style.css");
        } else if (pathOnly == "/script.js") {
            contentType = "application/javascript";
            body = readFileText("script.js");
        } else if (pathOnly == "/menu.html") {
            body = readFileText("menu.html");
        } else if (pathOnly == "/order.html") {
            body = readFileText("order.html");
        } else if (pathOnly == "/visit.html") {
            body = readFileText("visit.html");
        } else {
            body = readFileText("index.html");
        }

        stringstream res;
        res << "HTTP/1.1 200 OK\r\n"
            << "Content-Type: " << contentType << "\r\n"
            << "Content-Length: " << body.size() << "\r\n"
            << "Connection: close\r\n\r\n"
            << body;
        string raw = res.str();
        write(clientFd, raw.c_str(), raw.size());
        close(clientFd);
    }
}

// -------------------------------------------------------------
//  8. MAIN FUNCTION
// -------------------------------------------------------------
int main(int argc, char* argv[]) {
    initDatabase();
    loadMenuFromDB();

    if (argc >= 2 && string(argv[1]) == "--web") {
        startWebServer(8080);
        sqlite3_close(db);
        return 0;
    }

    int choice;
    do {
        cout << "\n============================================================\n"
             << "   RESTAURANT MANAGEMENT SYSTEM (C++ & SQLite - Group 3)\n"
             << "============================================================\n"
             << "  1. Show food menu\n"
             << "  2. Sort menu by price (Bubble Sort)\n"
             << "  3. Add dish to order (Linear Search)\n"
             << "  4. Checkout, print bill & save receipt\n"
             << "  5. View saved orders in SQLite (restaurant.db)\n"
             << "  6. Start Web Interface on http://localhost:8080\n"
             << "  0. Exit\n"
             << "============================================================\n";
        choice = readInt("Choose option: ");

        if (choice == 1) showMenuConsole(menuList, "ZAFRAN FOOD MENU");
        else if (choice == 2) showMenuConsole(getBubbleSortedMenu(), "SORTED BY PRICE (BUBBLE SORT)");
        else if (choice == 3) addDishConsole();
        else if (choice == 4) checkoutConsole();
        else if (choice == 5) showOrdersConsole();
        else if (choice == 6) startWebServer(8080);
    } while (choice != 0);

    sqlite3_close(db);
    return 0;
}

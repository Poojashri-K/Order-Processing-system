// main.cpp
#include <bits/stdc++.h>
using namespace std;


// Base Class for Polymorphism
class BaseOrder {
public:
    virtual void display() const = 0; // pure virtual function
    virtual ~BaseOrder() {}           // virtual destructor (important for polymorphism)
};


// Derived Class: Order
struct Order : public BaseOrder {
    int id;
    string customer;
    vector<string> items;
    double total;
    int priority; // higher = processed earlier
    string status; // "pending", "processed", "cancelled"
    bool urgent = false;

    void display() const override {
   cout << id << " | " << customer << " | " << total
             << " | p=" << priority << (urgent?" ⚠️ URGENT":"") 
             << " | " << status << "\n";
    }
};


// Node Class (Linked List Node)
struct Node {
    Order order;
    Node* next;
    Node(const Order& o): order(o), next(nullptr) {}
};


// OrderList Class (Abstraction)
class OrderList {
    Node* head;
    Node* tail;
public:
    OrderList(): head(nullptr), tail(nullptr) {}
    ~OrderList() {
        Node* cur = head;
        while(cur) {
            Node* nxt = cur->next;
            delete cur;
            cur = nxt;
        }
    }

    void pushBack(const Order& o) {
        Node* n = new Node(o);
        if(!head) head = tail = n;
        else {
            tail->next = n;
            tail = n;
        }
    }

    bool removeById(int id) {
        Node* cur = head;
        Node* prev = nullptr;
        while(cur) {
            if(cur->order.id == id) {
                if(prev) prev->next = cur->next;
                else head = cur->next;
                if(cur == tail) tail = prev;
                delete cur;
                return true;
            }
            prev = cur;
            cur = cur->next;
        }
        return false;
    }

    Order* findById(int id) {
        Node* cur = head;
        while(cur) {
            if(cur->order.id == id) return &cur->order;
            cur = cur->next;
        }
        return nullptr;
    }

    vector<Order> toVector() const {
        vector<Order> v;
        Node* cur = head;
        while(cur) {
            v.push_back(cur->order);
            cur = cur->next;
        }
        return v;
    }

    void clear() {
        Node* cur = head;
        while(cur) {
            Node* nxt = cur->next;
            delete cur;
            cur = nxt;
        }
        head = tail = nullptr;
    }

    // New function to demonstrate polymorphism
    void displayAll() const {
        Node* cur = head;
        while(cur) {
            const BaseOrder* basePtr = &cur->order;
            basePtr->display(); // polymorphic call
            cur = cur->next;
        }
    }
};


// Comparator for Priority Queue
// In priority queue comparator:
struct PQComp {
    bool operator()(const Order &a, const Order &b) const {
        if(a.urgent != b.urgent) return !a.urgent; // urgent comes first
        if(a.priority==b.priority) return a.id>b.id;
        return a.priority < b.priority;
    }
};


// File Persistence Functions
void saveToFile(const OrderList& list, const string& filename) {
    ofstream ofs(filename);
    if(!ofs) return;
    auto orders = list.toVector();
    for(const auto &o : orders) {
        ofs << o.id << "|" << o.customer << "|" << o.total << "|" << o.priority << "|" << o.status;
        ofs << "|";
        for(size_t i=0;i<o.items.size();++i) {
            if(i) ofs<<",";
            ofs<<o.items[i];
        }
        ofs << "\n";
    }
    ofs.close();
}

void loadFromFile(OrderList& list, const string& filename) {
    ifstream ifs(filename);
    if(!ifs) return;
    list.clear();
    string line;
    while(getline(ifs, line)) {
        if(line.empty()) continue;
        stringstream ss(line);
        string idS, customer, totalS, priorityS, status, itemsS;
        getline(ss, idS, '|');
        getline(ss, customer, '|');
        getline(ss, totalS, '|');
        getline(ss, priorityS, '|');
        getline(ss, status, '|');
        getline(ss, itemsS);
        Order o;
        o.id = stoi(idS);
        o.customer = customer;
        o.total = stod(totalS);
        o.priority = stoi(priorityS);
        o.status = status;
        o.items.clear();
        string item;
        stringstream is(itemsS);
        while(getline(is, item, ',')) if(!item.empty()) o.items.push_back(item);
        list.pushBack(o);
    }
    ifs.close();
}


// Main Program
int main() {
    OrderList orders;
    loadFromFile(orders, "orders.txt");

    // create a priority queue for processing
    priority_queue<Order, vector<Order>, PQComp> pq;
    // stack for processed orders (to show DS usage)
    stack<Order> processed;

    // push items from list into priority queue (only pending)
    auto v = orders.toVector();
    for(auto &o : v) {
        if(o.status == "pending") pq.push(o);
    }

    cout << "Order Processing System (C++ DS version)\n";
    cout << "Commands:\n 1 - Add order\n 2 - Cancel order\n 3 - Search order\n 4 - List orders\n 5 - Process next order (by priority)\n 6 - List processed orders\n 7 - Save & Exit\n";

    int choice;
    while(true) {
        cout << "\nEnter choice: ";
        if(!(cin >> choice)) break;
        if(choice == 1) {
            Order o;
            cout << "Enter ID (int): "; cin >> o.id;
            cin.ignore();
            cout << "Customer name: "; getline(cin, o.customer);
            cout << "Enter number of items: "; int n; cin >> n; cin.ignore();
            o.items.clear();
            for(int i=0;i<n;++i) {
                string it; cout << "Item " << (i+1) << ": "; getline(cin, it);
                o.items.push_back(it);
            }
            cout << "Total amount: "; cin >> o.total;
            cout << "Priority (higher processes earlier): "; cin >> o.priority;
            o.status = "pending";
            orders.pushBack(o);
            pq.push(o);
            cout << "Order added.\n";
        }
        else if(choice == 2) {
            int id; cout << "Enter order id to cancel: "; cin >> id;
            Order* found = orders.findById(id);
            if(found) {
                found->status = "cancelled";
                bool removed = orders.removeById(id);
                cout << "Order cancelled and removed from list.\n";
                // Rebuild pq from scratch (simple but safe)
                priority_queue<Order, vector<Order>, PQComp> newpq;
                auto vv = orders.toVector();
                for(auto &oo : vv) if(oo.status == "pending") newpq.push(oo);
                pq.swap(newpq);
            } else {
                cout << "Order id not found.\n";
            }
        }
        else if(choice == 3) {
            int id; cout << "Enter order id to search: "; cin >> id;
            Order* f = orders.findById(id);
            if(!f) cout << "Not found.\n";
            else {
                cout << "Order " << f->id << " | Customer: " << f->customer << " | Total: " << f->total
                     << " | Priority: " << f->priority << " | Status: " << f->status << "\nItems:\n";
                for(auto &it: f->items) cout << " - " << it << "\n";
            }
        }
        else if(choice == 4) {
            cout << "All orders:\n";
            orders.displayAll(); // Polymorphic display
        }
        else if(choice == 5) {
            if(pq.empty()) {
                cout << "No pending orders to process.\n";
            } else {
                Order top = pq.top(); pq.pop();
                Order* inList = orders.findById(top.id);
                if(inList) inList->status = "processed";
                top.status = "processed";
                processed.push(top);
                cout << "Processed order id: " << top.id << " customer: " << top.customer << "\n";
            }
        }
        else if(choice == 6) {
            cout << "Processed orders (most recent first):\n";
            if(processed.empty()) cout << "None\n";
            else {
                stack<Order> tmp = processed;
                while(!tmp.empty()) {
                    Order o = tmp.top(); tmp.pop();
                    o.display(); // polymorphic display
                }
            }
        }
        else if(choice == 7) {
            saveToFile(orders, "orders.txt");
            cout << "Saved to orders.txt. Exiting.\n";
            break;
        }
        else {
            cout << "Unknown choice.\n";
        }
    }

    return 0;
}

#include <iostream>
#include <vector>
#include <fstream>
#include <sstream>
#include <string>
#include <stdexcept>
#include <algorithm>  

class Order;
class Product;
class OrderItem;
class Customer;
class UserManager;

// User base class
class User {
protected:
    std::string username;
    std::string password;
    std::string email;

public:
    User(const std::string& username, const std::string& password, const std::string& email)
        : username(username), password(password), email(email) {}

    std::string getUsername() const {
        return username;
    }

    std::string getEmail() const {
        return email;
    }

    bool authenticate(const std::string& pass) const {
        return password == pass;
    }

    std::string getPassword() const {
        return password;
    }
};

// Customer class
class Customer : public User {
private:
    std::vector<Order*> orders;

public:
    Customer(const std::string& username, const std::string& password, const std::string& email)
        : User(username, password, email) {}

    void placeOrder(Order* order) {
        orders.push_back(order);
    }

    std::vector<Order*> getOrders() const {
        return orders;
    }
};

// Product class
class Product {
private:
    std::string name;
    std::string category;
    double price;
    int stock;

public:
    Product(const std::string& name, const std::string& category, double price, int stock)
        : name(name), category(category), price(price), stock(stock) {}

    std::string getName() const {
        return name;
    }

    std::string getCategory() const {
        return category;
    }

    double getPrice() const {
        return price;
    }

    int getStock() const {
        return stock;
    }

    void setStock(int stock) {
        this->stock = stock;
    }
};

// OrderItem class
class OrderItem {
private:
    Product* product;
    int quantity;

public:
    OrderItem(Product* product, int quantity) : product(product), quantity(quantity) {}

    Product* getProduct() const {
        return product;
    }

    int getQuantity() const {
        return quantity;
    }

    double getTotalPrice() const {
        return product->getPrice() * quantity;
    }

    void reduceQuantity(int quantity) {
        this->quantity -= quantity;
    }
};

// Order class
class Order {
private:
    std::vector<OrderItem*> items;
    Customer* customer;
    double totalPrice;
    std::string status;

public:
    Order(Customer* customer) : customer(customer), totalPrice(0.0), status("Pending") {}

    void addItem(OrderItem* item) {
        items.push_back(item);
        totalPrice += item->getTotalPrice();
    }

    void removeItem(OrderItem* item) {
        auto it = std::remove(items.begin(), items.end(), item);
        if (it != items.end()) {
            items.erase(it, items.end());
            totalPrice -= item->getTotalPrice();
        }
    }

    std::vector<OrderItem*> getItems() const {
        return items;
    }

    Customer* getCustomer() const {
        return customer;
    }

    double getTotalPrice() const {
        return totalPrice;
    }

    std::string getStatus() const {
        return status;
    }

    void setStatus(const std::string& status) {
        this->status = status;
    }
};

// UserManager class
class UserManager {
private:
    static const std::string USER_FILE;

public:
    UserManager() {
        std::ifstream file(USER_FILE);
        if (!file.good()) {
            std::ofstream newFile(USER_FILE);
        }
    }

    void registerUser(const User& user) {
        std::ofstream file(USER_FILE, std::ios::app);
        file << user.getUsername() << "," << user.getPassword() << "," << user.getEmail() << "\n";
    }

    User* authenticateUser(const std::string& username, const std::string& password) {
        std::ifstream file(USER_FILE);
        std::string line;
        while (std::getline(file, line)) {
            std::istringstream iss(line);
            std::string user, pass, email;
            std::getline(iss, user, ',');
            std::getline(iss, pass, ',');
            std::getline(iss, email);
            if (user == username && pass == password) {
                return new Customer(user, pass, email);
            }
        }
        return nullptr;
    }

    bool userExists(const std::string& username) {
        std::ifstream file(USER_FILE);
        std::string line;
        while (std::getline(file, line)) {
            std::istringstream iss(line);
            std::string user;
            std::getline(iss, user, ',');
            if (user == username) {
                return true;
            }
        }
        return false;
    }
};

const std::string UserManager::USER_FILE = "users.txt";

// GroceryPlatform class
class GroceryPlatform {
private:
    std::vector<Product*> products;
    std::vector<Order*> orders;
    UserManager userManager;

public:
    GroceryPlatform() : userManager() {}

    void addProduct(Product* product) {
        products.push_back(product);
    }

    std::vector<Product*> getProducts() const {
        return products;
    }

    void placeOrder(Order* order) {
        orders.push_back(order);
        order->getCustomer()->placeOrder(order);
    }

    std::vector<Order*> getOrders() const {
        return orders;
    }

    UserManager& getUserManager() {
        return userManager;
    }

    static void createAccount(std::istream& input, GroceryPlatform& platform) {
        std::string username, password, email;

        std::cout << "Enter new username: ";
        std::getline(input, username);

        if (platform.getUserManager().userExists(username)) {
            std::cout << "Username already exists. Please choose a different username.\n";
            return;
        }

        std::cout << "Enter new password: ";
        std::getline(input, password);
        std::cout << "Enter email: ";
        std::getline(input, email);

        User* newUser = new Customer(username, password, email);
        platform.getUserManager().registerUser(*newUser);
        std::cout << "Account created successfully! Please log in.\n";
    }

    static void handleCustomerActions(std::istream& input, GroceryPlatform& platform, Customer* customer) {
        while (true) {
            std::cout << "\n1. View Products\n2. View Cart\n3. Checkout\n4. Logout\nChoose an option: ";
            int choice;
            input >> choice;
            input.ignore(); 

            if (choice == 1) {
                viewProductsByCategory(input, platform, customer);
            } else if (choice == 2) {
                viewCart(input, customer);
            } else if (choice == 3) {
                checkout(customer);
                std::cout << "Thank you for shopping!\n";
                break;
            } else if (choice == 4) {
                std::cout << "Logged out successfully!\n";
                break;
            } else {
                std::cout << "Invalid choice. Please try again.\n";
            }
        }
    }

    static void viewProductsByCategory(std::istream& input, GroceryPlatform& platform, Customer* customer) {
        std::cout << "\n1. Dairy Products\n2. Snacks\n3. Fruits\n4. Vegetables\nChoose a category: ";
        int categoryChoice;
        input >> categoryChoice;
        input.ignore(); 

        std::string category;
        switch (categoryChoice) {
            case 1: category = "Dairy"; break;
            case 2: category = "Snacks"; break;
            case 3: category = "Fruits"; break;
            case 4: category = "Vegetables"; break;
            default: std::cout << "Invalid category choice.\n"; return;
        }

        std::vector<Product*> categoryProducts;
        for (auto& product : platform.getProducts()) {
            if (product->getCategory() == category) {
                categoryProducts.push_back(product);
            }
        }

        if (categoryProducts.empty()) {
            std::cout << "No products available in this category.\n";
            return;
        }

        std::cout << "\n" << category << " Products:\n";
        for (auto& product : categoryProducts) {
            std::cout << product->getName() << " - Rs " << product->getPrice() << " - Stock: " << product->getStock() << "\n";
        }

        addToCartOrContinue(input, platform, categoryProducts, customer);
    }

    static void addToCartOrContinue(std::istream& input, GroceryPlatform& platform, const std::vector<Product*>& products, Customer* customer) {
        while (true) {
            std::string productName;
            int quantity;

            std::cout << "Enter product name to add to cart: ";
            std::getline(input, productName);
            std::cout << "Enter quantity: ";
            input >> quantity;
            input.ignore(); 

            Product* productToAdd = nullptr;
            for (auto& product : products) {
                if (product->getName() == productName) {
                    productToAdd = product;
                    break;
                }
            }

            if (productToAdd && productToAdd->getStock() >= quantity) {
                Order* order = new Order(customer);
                OrderItem* item = new OrderItem(productToAdd, quantity);
                order->addItem(item);
                customer->placeOrder(order);
                productToAdd->setStock(productToAdd->getStock() - quantity);
                std::cout << "Item added to cart.\n";

                std::cout << "Do you want to add more items? (yes/no): ";
                std::string addMore;
                std::getline(input, addMore);
                if (addMore == "no") {
                    viewCart(input, customer);
                    break;
                } else if (addMore == "yes") {
                    viewProductsByCategory(input, platform, customer);
                    break;
                }
            } else {
                std::cout << "Product not available or insufficient stock!\n";
            }
        }
    }

    static void viewCart(std::istream& input, Customer* customer) {
        std::vector<Order*> orders = customer->getOrders();
        if (orders.empty()) {
            std::cout << "Your cart is empty.\n";
            return;
        }

        while (true) {
            std::cout << "Your cart:\n";
            for (auto& order : orders) {
                for (auto& item : order->getItems()) {
                    std::cout << item->getProduct()->getName() << " - Quantity: " << item->getQuantity() << " - Price: Rs " << item->getTotalPrice() << "\n";
                }
            }

            std::cout << "1. Delete an item\n2. Continue to checkout\nChoose an option: ";
            int choice;
            input >> choice;
            input.ignore(); 

            if (choice == 1) {
                std::string productName;
                int quantityToDelete;

                std::cout << "Enter product name to delete: ";
                std::getline(input, productName);
                std::cout << "Enter quantity to delete: ";
                input >> quantityToDelete;
                input.ignore(); 

                bool itemDeleted = false;
                for (auto& order : orders) {
                    for (auto& item : order->getItems()) {
                        if (item->getProduct()->getName() == productName && item->getQuantity() >= quantityToDelete) {
                            item->reduceQuantity(quantityToDelete);
                            item->getProduct()->setStock(item->getProduct()->getStock() + quantityToDelete);
                            std::cout << "Item quantity updated in cart.\n";

                            if (item->getQuantity() == 0) {
                                order->removeItem(item);
                                std::cout << "Item removed from cart.\n";
                            }

                            itemDeleted = true;
                            break;
                        }
                    }
                    if (itemDeleted) break;
                }

                if (!itemDeleted) {
                    std::cout << "Item not found in cart or insufficient quantity to delete.\n";
                }
            } else if (choice == 2) {
                break;
            } else {
                std::cout << "Invalid choice. Please try again.\n";
            }
        }
    }

    static void checkout(Customer* customer) {
        std::vector<Order*> orders = customer->getOrders();
        if (orders.empty()) {
            std::cout << "Your cart is empty.\n";
            return;
        }
        double totalAmount = 0;
        for (auto& order : orders) {
            totalAmount += order->getTotalPrice();
        }
        std::cout << "Total amount to pay: Rs " << totalAmount << "\n";
        // Clear cart after checkout
        customer->getOrders().clear();
    }
};

int main() {
    GroceryPlatform platform;

    // Sample Data
    platform.addProduct(new Product("Milk", "Dairy", 35, 100));
    platform.addProduct(new Product("Cheese", "Dairy", 20, 50));
    platform.addProduct(new Product("Chips", "Snacks", 20, 200));
    platform.addProduct(new Product("Cookies", "Snacks", 50, 150));
    platform.addProduct(new Product("Apples", "Fruits", 30, 100));
    platform.addProduct(new Product("Bananas", "Fruits", 10, 120));
    platform.addProduct(new Product("Carrots", "Vegetables", 25, 80));
    platform.addProduct(new Product("Broccoli", "Vegetables", 40, 60));

    std::cout << "======================================\n";
    std::cout << "        Welcome to XYZ Store!          \n";
    std::cout << "======================================\n";

    while (true) {
        User* user = nullptr;
        while (user == nullptr) {
            std::cout << "\n1. Login\n2. Create Account\nChoose an option: ";
            int choice;
            std::cin >> choice;
            std::cin.ignore(); 

            if (choice == 1) {
                // Login
                std::string username, password;
                std::cout << "Enter username: ";
                std::getline(std::cin, username);
                std::cout << "Enter password: ";
                std::getline(std::cin, password);

                try {
                    if (platform.getUserManager().userExists(username)) {
                        user = platform.getUserManager().authenticateUser(username, password);
                        if (user) {
                            std::cout << "Login successful!\n";
                            GroceryPlatform::handleCustomerActions(std::cin, platform, static_cast<Customer*>(user));
                            
                            return 0;
                        } else {
                            std::cout << "Login failed! Invalid username or password.\n";
                        }
                    } else {
                        std::cout << "No account with this username. Please create an account.\n";
                        GroceryPlatform::createAccount(std::cin, platform);
                    }
                } catch (const std::ios_base::failure& e) {
                    std::cout << "Error accessing user file: " << e.what() << "\n";
                }
            } else if (choice == 2) {
                
                GroceryPlatform::createAccount(std::cin, platform);
            } else {
                std::cout << "Invalid choice. Please try again.\n";
            }
        }
    }

    return 0;
}

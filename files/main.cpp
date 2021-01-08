#include <iostream>
#include <string>
#include <fstream>

#include <vector>

class Subscriber {
private:
    std::string name;
    int setup_year;
    long phone;
public:
    Subscriber(std::string name, int setup_year, long phone)
        : name{name}, setup_year{setup_year}, phone{phone} { }
    
    std::string get_name() { return name; }
    
    int get_setup_year() { return setup_year; }
    
    long get_phone() { return phone; }
    
    std::string to_text_entry() {
        return name + "\t" + std::to_string(setup_year) + "\t" + std::to_string(phone);
    }
    
    static Subscriber from_text_entry(std::string entry) {
        if (entry.empty())
            throw std::logic_error("empty entry");
        std::string name;
        std::string setup_year;
        std::string phone;
        
        int i = 0;
        for (; i < entry.size(); i++) {
            if (entry[i] == '\t') {
                i++;
                break;
            }
            name += entry[i];
        }
        for (; i < entry.size(); i++) {
            if (entry[i] == '\t'){
                i++;
                break;
            }
            setup_year += entry[i];
        }
        for (; i < entry.size(); i++) {
            phone += entry[i];
        }
        
        return Subscriber(name, std::stoi(setup_year), std::stol(phone));
    }
    
};

template<typename T>
class FileHandle {
private:
    std::string filename;
    std::string extension;
protected:
    FileHandle(std::string filename, std::string extension): filename{filename}, extension{extension} { }
    
    std::string get_file_name() {
        return filename + "." + extension;
    }
};

template<typename T>
class TextFileHandle : public FileHandle<T> {
public:
    TextFileHandle(std::string filename): FileHandle<T>(filename, "txt") { }
    
    bool write(T obj) {
        std::string file_name = this->get_file_name();
        std::ofstream file(file_name, std::ios::out | std::ios::app);
        if (!file.is_open()) {
            std::cout << "failed to open file '" << file_name << "'" << std::endl;
            return false;
        }
        
        std::string entry = obj.to_text_entry();
        file << entry << "\n";
        
        file.close();
        return true;
    }
    
    bool print_entries() {
        std::string file_name = this->get_file_name();
        std::ifstream file(file_name, std::ios::in);
        if (!file.is_open()) {
            std::cout << "no text database file found. create a first entry to initialize it" << std::endl;
            return false;
        }
        
        file.seekg(0, std::ios::seekdir::beg);
        
        while (!file.eof()) {
            std::string entry = read_entry(file);
            if (entry.empty())
                break;
            T item = T::from_text_entry(entry);
            std::cout << item << std::endl;;
        }
        
        file.close();
        return true;
    }
    
    bool print_reverse_entries() {
        std::string file_name = this->get_file_name();
        std::ifstream file(file_name, std::ios::in | std::ios::ate);
        if (!file.is_open()) {
            std::cout << "no text database file found. create a first entry to initialize it" << std::endl;
            return false;
        }
        
        if (file.tellg() == 0) {
            std::cout << "no entries in database" << std::endl;
        } else {
            file.seekg(-2, std::ios::seekdir::end);
            while (true) {
                
                for (; file.tellg() > 0 && file.peek() != '\n'; file.seekg(-1, std::ios::seekdir::cur));
                
                if (file.tellg() > 0)
                    file.ignore();
                std::string entry = read_entry(file);
                T item = T::from_text_entry(entry);
                std::cout << item << std::endl;;
                
                file.seekg(-entry.size() - 3, std::ios::seekdir::cur);
                if (file.peek() == -1)
                    break;
            }
        }
        return true;
    }
    
    void sort() {
        int count = bubble_sort();
        for (int i = 0; i < count - 1; i++) {
            bubble_sort();
        }
    }
    
    bool print_after(int year) {
        std::string file_name = this->get_file_name();
        std::ifstream file(file_name, std::ios::in);
        if (!file.is_open()) {
            std::cout << "no text database file found. create a first entry to initialize it" << std::endl;
            return false;
        }
        
        file.seekg(0, std::ios::seekdir::beg);
        
        while (!file.eof()) {
            std::string entry = read_entry(file);
            if (entry.empty())
                break;
            T item = T::from_text_entry(entry);
            if (item.get_setup_year() > year) {
                std::cout << item << std::endl;;
            }
        }
        
        file.close();
        return true;
    }
    
protected:
    virtual std::string read_entry(std::istream &file) {
        std::string entry;
        std::getline(file, entry, '\n');
        return entry;
    }
    
    int bubble_sort() {
        std::string file_name = this->get_file_name();
        std::fstream file(file_name, std::ios::in | std::ios::out);
        if (!file.is_open()) {
            std::cout << "no text database file found. create a first entry to initialize it" << std::endl;
            return false;
        }
        
        int count = 0;
        while (true) {
            std::string first = read_entry(file);
            if (first.empty()) break;
            std::string second = read_entry(file);
            if (second.empty()) break;
            
            count++;
            T first_item = T::from_text_entry(first);
            T second_item = T::from_text_entry(second);
            
            if (strcmp(first_item.get_name().c_str(), second_item.get_name().c_str()) > 0) {
                file.seekg(-second.size() - 1, std::ios::seekdir::cur);
                file.seekg(-first.size() - 1, std::ios::seekdir::cur);
                file << second << "\n";
                file << first << "\n";

                file.seekg(-first.size() - 1, std::ios::seekdir::cur);
            } else {
                file.seekg(-second.size() - 1, std::ios::seekdir::cur);
            }
            
        }
        file.close();
        return count;
    }
    
};

template<typename T>
class BinaryTextFileHandle : public TextFileHandle<T> {
public:
    BinaryTextFileHandle(std::string filename): TextFileHandle<T>(filename) { }
    
protected:
    std::string read_entry(std::istream &file) override {
        std::string entry;
         while (!file.eof()) {
             int cur = file.get();
             if (cur < 0 && entry.empty())
                 break;
             if (cur < 0)
                 throw std::logic_error("unexpected end of file");
             if (cur == '\n' || cur == '\r')
                 break;
             entry += ((char) cur);
         }
        return entry;
    }
};

std::ostream & operator<<(std::ostream &stream, Subscriber sub) {
    stream << "Sub[" << sub.get_name() << ", ";
    stream << sub.get_phone() << ", setup in ";
    stream << sub.get_setup_year() << " year ]";
    return stream;
}

int read_int() {
    while (true) {
        std::string buffer;
        std::getline(std::cin, buffer);
        try {
            return std::stoi(buffer);
        } catch (...) {
            std::cout << "not a number!" << std::endl;
            continue;
        }
    }
}

long read_long() {
    while (true) {
        std::string buffer;
        std::getline(std::cin, buffer);
        try {
            return std::stol(buffer);
        } catch (...) {
            std::cout << "not a number!" << std::endl;
            continue;
        }
    }
}


std::string read_string() {
    while (true) {
        std::string buffer;
        std::getline(std::cin, buffer);
        if (buffer.size() > 0) {
            return buffer;
        }
    }
}

int main(int argc, const char * argv[]) {
    std::cout << "Enter count of subscribers to add: ";
    int count = read_int();
    if (count < 0) {
        std::cout << count << " is invalid number of subscribers" << std::endl;
        return 0;
    }
    
    TextFileHandle<Subscriber> handle("database");
    
    for (int i = 0; i < count; i++) {
        std::cout << "name: ";
        std::string name = read_string();
        
        std::cout << "setup year: ";
        int year = read_int();
        
        std::cout << "phone number: ";
        long number = read_long();
        
        Subscriber sub(name, year, number);
        handle.write(sub);
    }
    
    std::cout << "database entries: " << std::endl;
    handle.print_entries();
    
    std::cout << "database entries in reversed order: " << std::endl;
    handle.print_reverse_entries();
    
    std::cout << "enter year to selectc subscribers: " << std::endl;
    int year = read_int();
    
    handle.sort();
    handle.print_after(year);
    
    return 0;
}

/*
单例模式
*/
#include <iostream>

class Student {
    public:
        Student(int id, std::string name) : id_(id), name_(name) {}
        int get_id() const { return id_; }
        static Student *GetInstance();
        int kaka;
    private:
        int id_;
        std::string name_;
};

Student *Student::GetInstance() {
    static Student instance(100, "Alice");
    return &instance;
}

int main() {
    Student *s1 = Student::GetInstance();
    s1->kaka = 1000;
    Student *s2 = Student::GetInstance();

    if (s1 == s2) {
        std::cout << "Singleton test passed: Both instances are the same." << std::endl;
    } else {
        std::cout << "Singleton test failed: Instances are different." << std::endl;
    }

    std::cout << "Student1 ID: " << s1->kaka << std::endl;
    std::cout << "Student2 ID: " << s2->kaka << std::endl;
    return 0;
}

#include <iostream>
#include "Vector.h"
#include "Number.h"

int main() {

    Number::Number a = Number::createNumber(10.0);
    Number::Number b = Number::createNumber(5.0);
    
    std::cout << "a = " << a.getValue() << std::endl;
    std::cout << "b = " << b.getValue() << std::endl;
    std::cout << "a + b = " << (a + b).getValue() << std::endl;
    std::cout << "a - b = " << (a - b).getValue() << std::endl;
    std::cout << "a * b = " << (a * b).getValue() << std::endl;
    std::cout << "a / b = " << (a / b).getValue() << std::endl;
    
    std::cout << "Zero = " << Number::Number::Zero.getValue() << std::endl;
    std::cout << "One = " << Number::Number::One.getValue() << std::endl;
    
    Vector v1(3.0, 4.0);
    Vector v2(1.0, 2.0);
    
    std::cout << "v1 = " << v1.toString() << std::endl;
    std::cout << "v2 = " << v2.toString() << std::endl;
    
    Vector v3 = v1.add(v2);
    std::cout << "v1 + v2 = " << v3.toString() << std::endl;
    
    std::cout << "Длина v1 = " << v1.getMagnitude() << std::endl;
    std::cout << "Угол v1 = " << v1.getAngle() << " рад" << std::endl;
    std::cout << "Полярные координаты v1: " << v1.toPolarString() << std::endl;
    
    Vector zeroVec = Vector_Zero();
    Vector oneVec = Vector_One();
    
    std::cout << "Vector_Zero() = " << zeroVec.toString() << std::endl;
    std::cout << "Vector_One() = " << oneVec.toString() << std::endl;
    
    return 0;
}
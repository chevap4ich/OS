#pragma once

class Vector {
private:
    void* impl; 

public:
    Vector();
    Vector(double x, double y);
    Vector(const Vector& other);
    ~Vector();

    Vector add(const Vector& other) const;
    Vector subtract(const Vector& other) const;

    double getMagnitude() const;
    double getAngle() const;

    const char* toString() const;
    const char* toPolarString() const;

    double getX() const;
    double getY() const;
};

extern "C" Vector Vector_Zero();
extern "C" Vector Vector_One();
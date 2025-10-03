#pragma once
namespace Number
{
    class Number
    {
    private:
        double value;

    public:
        Number();
        Number(double val);

        Number& operator=(const Number& other);

        Number operator+(const Number& other) const;
        Number operator-(const Number& other) const;
        Number operator*(const Number& other) const;
        Number operator/(const Number& other) const;

        double getValue() const;

        static Number Create(double value);

        // Статические константы
        static const Number Zero;
        static const Number One;
    };

    Number createNumber(double value);
}
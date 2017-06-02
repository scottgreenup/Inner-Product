#ifndef _VECTOR_H_
#define _VECTOR_H_

#include <string>
#include <vector>

class Vector {
public:
    Vector(uint32_t size);
    Vector(const Vector& other);
    ~Vector();

    double Dot(const Vector& other) const;
    std::string ToString() const;

    double& operator[](size_t index) {
        return m_elements[index];
    }

    const double& operator[](size_t index) const {
        return m_elements[index];
    }

private:
    std::vector<double> m_elements;
    uint32_t m_size;
};

#endif

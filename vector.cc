
#include <cstdint>
#include <sstream>

#include "vector.h"

Vector::Vector(uint32_t size)
: m_elements(size)
, m_size(size) {

}

Vector::Vector(const Vector& other)
: m_elements(other.m_elements.begin(), other.m_elements.end())
, m_size(other.m_size) {

}

Vector::~Vector() {

}

double Vector::Dot(const Vector& other) const {
    if (other.m_size != m_size) {
        return 0.0;
    }

    double sum = 0.0;
    for (uint32_t i = 0; i < m_size; i++) {
        sum += m_elements[i] * other.m_elements[i];
    }
    return sum;
}

std::string Vector::ToString() const {
    std::stringstream ss;
    ss << "[";
    if (m_size > 0) {
        ss << m_elements[0];
    }
    for (uint32_t i = 1; i < m_size; i++) {
        ss << ", " << m_elements[i];
    }
    ss << "]";

    return ss.str();
}

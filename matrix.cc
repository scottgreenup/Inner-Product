
#include "matrix.h"

#include <cstdint>
#include <iostream>
#include <sstream>

Matrix::Matrix(uint32_t nrows, uint32_t ncols)
: m_nrows(nrows)
, m_ncols(ncols) {
    for (uint32_t i = 0; i < nrows; i++) {
        m_rows.push_back(Vector(ncols));
    }
}

Matrix::Matrix(const Matrix& other)
: m_nrows(other.m_nrows)
, m_ncols(other.m_ncols) {
    for (const Vector &v : other.m_rows) {
        m_rows.push_back(Vector(v));
    }
}

Matrix::~Matrix() {
}

std::string Matrix::ToString() const {
    if (m_nrows == 0) {
        return "[ empty ]";
    }

    std::stringstream ss;
    ss << "[" << std::endl;
    for (const Vector &v : m_rows) {
        ss << "  " << v.ToString() << "," << std::endl;
    }
    ss << "]";

    return ss.str();
}


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

Matrix::Matrix(uint32_t* ser) {
    m_nrows = ser[0];
    m_ncols = ser[1];

    uint32_t index = 2;

    for (uint32_t i = 0; i < m_nrows; i++) {
        Vector row(m_ncols);
        for (uint32_t j = 0; j < m_ncols; j++) {
            row[j] = ser[index];
            ++index;
        }
        m_rows.push_back(row);
    }

}

Matrix::~Matrix() { }

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

uint32_t Matrix::SerializeSize() const {
    return 2 + m_nrows * m_ncols;
}

uint32_t* Matrix::Serialize() const {
    uint32_t sz = SerializeSize();
    uint32_t* buf = new uint32_t[sz];
    buf[0] = m_nrows;
    buf[1] = m_ncols;

    uint32_t index = 2;

    for (uint32_t i = 0; i < m_nrows; i++) {
        for (uint32_t j = 0; j < m_ncols; j++) {
            buf[index] = m_rows[i][j];
            ++index;
        }
    }

    return buf;
}

#ifndef _MATRIX_H_
#define _MATRIX_H_

#include <string>
#include <vector>

#include "vector.h"

class Matrix {
public:
    Matrix(uint32_t nrows, uint32_t ncols);
    Matrix(uint32_t* serialized_matrix);
    Matrix(const Matrix& other);

    ~Matrix();

    std::string ToString() const;

    Vector& operator[](size_t index) {
        return m_rows[index];
    }

    const Vector& operator[](size_t index) const {
        return m_rows[index];
    }


    uint32_t rows() const {
        return m_nrows;
    }

    uint32_t cols() const {
        return m_ncols;
    }

    uint32_t SerializeSize() const;
    uint32_t* Serialize() const;

private:
    uint32_t m_ncols;
    uint32_t m_nrows;

    std::vector<Vector> m_rows;
};

#endif

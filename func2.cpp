
#include <string.h>
#include <map>
#include <vector>
#include <cstddef>
#include <iostream>
#include <memory>
#include <bits/stdc++.h>
/*
# define __ASSERT_VOID_CAST (void)
# define assert(expr)		(__ASSERT_VOID_CAST (0))
*/


template <class T1, class T2, class T3>
class ReferenceGemmFunctor {
 public:
  void operator()(size_t m, size_t n,
                  size_t k, const T1* a, size_t lda, const T2* b, size_t ldb,
                  T3* c, size_t ldc) {
    const size_t a_i_stride = lda;
    const size_t a_l_stride = 1;
    const size_t b_j_stride = 1;
    const size_t b_l_stride = ldb;
    const size_t c_i_stride = ldc;
    const size_t c_j_stride = 1;
    size_t i, j, l;
    for (j = 0; j < n; j++) {
      for (i = 0; i < m; i++) {
        T3 total(0);
        for (l = 0; l < k; l++) {
          const size_t a_index = ((i * a_i_stride) + (l * a_l_stride));
          const T1 a_value = a[a_index];
          const size_t b_index = ((j * b_j_stride) + (l * b_l_stride));
          const T2 b_value = b[b_index];
          total += (a_value * b_value);
        }
        const size_t c_index = ((i * c_i_stride) + (j * c_j_stride));
        c[c_index] = total;
      }
    }
  }
};





void ReferenceEightBitIntGemm22(size_t m, size_t n, size_t k,
                              const size_t* a, size_t lda, const size_t* b, 
                              size_t ldb, size_t* c, size_t ldc) 
{

    const size_t a_i_stride = lda;
    const size_t a_l_stride = 1;
    const size_t b_j_stride = 1;
    const size_t b_l_stride = ldb;
    const size_t c_i_stride = ldc;
    const size_t c_j_stride = 1;
    size_t i, j, l;


  for (j = 0; j < n; j++) {
      for (i = 0; i < m; i++) {
        size_t total(0);
        for (l = 0; l < k; l++) {
          const size_t a_index = ((i * a_i_stride) + (l * a_l_stride));
          const size_t a_value = a[a_index];
          const size_t b_index = ((j * b_j_stride) + (l * b_l_stride));
          const size_t b_value = b[b_index];
          total += (a_value * b_value);
        }
        const size_t c_index = ((i * c_i_stride) + (j * c_j_stride));
        c[c_index] = total;
        std::cout<<total<<std::endl;
      }
    }
  }
    
    


int main()
{

    const size_t m = 4;
    const size_t n = 2;
    const size_t k = 3;
    // Matrix A (LHS) is:
    // |  7 | 10 | 13 | 16 |
    // |  8 | 11 | 14 | 17 |
    // |  9 | 12 | 15 | 18 |
    const size_t a_data[] = {7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18};
    // Matrix B (RHS) is:
    // |  1 |  3 |  5 |
    // |  2 |  4 |  6 |
    const size_t b_data[] = {1, 2, 3, 4, 5, 6};
    // Here are the results we expect, from hand calculations:
    // (1 * 7) + (3 * 8) + (5 * 9) = 76
    // (2 * 7) + (4 * 8) + (6 * 9) = 100
    // (1 * 10) + (3 * 11) + (5 * 12) = 103
    // (2 * 10) + (4 * 11) + (6 * 12) = 136
    // (1 * 13) + (3 * 14) + (5 * 15) = 130
    // (2 * 13) + (4 * 14) + (6 * 15) = 172
    // (1 * 16) + (3 * 17) + (5 * 18) = 157
    // (2 * 16) + (4 * 17) + (6 * 18) = 208
    // That means matrix C should be:
    // |  76 | 103 | 130 | 157 |
    // | 100 | 136 | 172 | 208 |
    const size_t expected_data[] = {76, 100, 103, 136, 130, 172, 157, 208};

    const size_t c_count = m * n;
    std::unique_ptr<size_t[]> output_data(new size_t[c_count]);

    
    const int lda = k;
    const int ldb = n;
    const int ldc = n;

   

    ReferenceEightBitIntGemm22( m, n, k, a_data, lda, b_data, ldb, output_data.get(),ldc);

    
    
    return 0;
  
  
  
  
}






#include <string.h>
#include <map>
#include <vector>
#include <cstddef>
#include <iostream>
#include <memory>
#include <bits/stdc++.h>
# define __ASSERT_VOID_CAST (void)
# define assert(expr)		(__ASSERT_VOID_CAST (0))



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





void ReferenceEightBitIntGemm22(bool transpose_a, bool transpose_b,
                              bool transpose_c, int m, int n, int k,
                              const std::uint8_t* a, std::int32_t a_offset,
                              int lda, const std::uint8_t* b,
                              std::int32_t b_offset, int ldb, std::uint8_t* c,
                              std::int32_t c_offset, std::int32_t c_mult_int,
                              std::int32_t c_shift, int ldc) 
{

  assert((c_shift >= 0) && (c_shift <= 32));

  assert(a != nullptr);
  assert(b != nullptr);
  assert(c != nullptr);

  int a_i_stride;
  int a_l_stride;
  if (transpose_a) {
    a_i_stride = lda;
    a_l_stride = 1;
  } else {
    a_i_stride = 1;
    a_l_stride = lda;
  }
  int b_j_stride;
  int b_l_stride;
  if (transpose_b) {
    b_j_stride = 1;
    b_l_stride = ldb;
  } else {
    b_j_stride = ldb;
    b_l_stride = 1;
  }
  int c_i_stride;
  int c_j_stride;
  if (transpose_c) {
    c_i_stride = ldc;
    c_j_stride = 1;
  } else {
    c_i_stride = 1;
    c_j_stride = ldc;
  }
  int i, j, l;

  const std::int32_t kRoundingTerm = (c_shift < 1) ? 0 : (1 << (c_shift - 1));

  for (j = 0; j < n; j++) {
    for (i = 0; i < m; i++) {
      std::int32_t total = 0;
      for (l = 0; l < k; l++) {
        const int a_index = i * a_i_stride + l * a_l_stride;
        const std::uint8_t a_as_byte = a[a_index];
        const std::int32_t a_as_int =
            static_cast<std::int32_t>(a_as_byte) + a_offset;
        const int b_index = j * b_j_stride + l * b_l_stride;
        const std::uint8_t b_as_byte = b[b_index];
        const std::int32_t b_as_int =
            static_cast<std::int32_t>(b_as_byte) + b_offset;
        const std::int32_t mult_as_int = a_as_int * b_as_int;
        total += mult_as_int;
      }
      std::int32_t output =
          (((total + c_offset) * c_mult_int) + kRoundingTerm) >> c_shift;
      if (output > 255) {
        output = 255;
      }
      if (output < 0) {
        output = 0;
      }
      const int c_index = i * c_i_stride + j * c_j_stride;
        
      std::cout<<output<< "\n";
        
      c[c_index] = static_cast<std::uint8_t>(output);
    }
  }
    
    
   
    
}






int main()
{

    const int m = 4;
    const int n = 2;
    const int k = 3;
    // Matrix A (LHS) is:
    // |  7 | 10 | 13 | 16 |
    // |  8 | 11 | 14 | 17 |
    // |  9 | 12 | 15 | 18 |
    const uint8_t a_data[] = {7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18};
    // Matrix B (RHS) is:
    // |  1 |  3 |  5 |
    // |  2 |  4 |  6 |
    const uint8_t b_data[] = {1, 2, 3, 4, 5, 6};
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
    const uint8_t expected_data[] = {76, 100, 103, 136, 130, 172, 157, 208};

    const int c_count = m * n;
    std::unique_ptr<uint8_t[]> output_data(new uint8_t[c_count]);

    const bool is_a_transposed = true;
    const bool is_b_transposed = true;
    const bool is_c_transposed = true;
    const int lda = k;
    const int ldb = n;
    const int ldc = n;

    const int a_offset = 0;
    const int b_offset = 0;
    const int c_offset = 0;
    const int c_mult = 1;
    const int c_shift = 0;

    ReferenceEightBitIntGemm22(is_a_transposed, is_b_transposed, is_c_transposed, m, n, k, a_data,
    a_offset, lda, b_data, b_offset, ldb, output_data.get(), c_offset,
    c_mult, c_shift, ldc);

    
    const int input_data[] = {7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18};
    const int filter_data[] = {1, 2, 3, 4, 5, 6};
    std::unique_ptr<int[]> output_data1(new int[c_count]);
    

    std::transform(ReferenceGemmFunctor( m, n, k, input_data, lda, filter_data, ldb,
                   output_data1, ldc)
                  );
    
    
    
    
    return 0;
  
  
  
  
}






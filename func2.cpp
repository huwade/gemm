
#include <string.h>
#include <map>
#include <vector>
#include <cstddef>
#include <iostream>
#include <memory>
#include <bits/stdc++.h>


void ReferenceEightBitIntGemm22(int m, int n, int k,
                              const int* a, int lda, const int* b, 
                              int ldb, int* c, int ldc) 
{

    const int a_i_stride = lda;
    const int a_l_stride = 1;
    const int b_j_stride = 1;
    const int b_l_stride = ldb;
    const int c_i_stride = ldc;
    const int c_j_stride = 1;
    int i, j, l;


  for (j = 0; j < n; j++) 
  {
      for (i = 0; i < m; i++) 
      {
        int total(0);
        for (l = 0; l < k; l++) 
        {
          const int a_index = ((i * a_i_stride) + (l * a_l_stride));
          const int a_value = a[a_index];
          const int b_index = ((j * b_j_stride) + (l * b_l_stride));
          const int b_value = b[b_index];
          total += (a_value * b_value);
        }
        const int c_index = ((i * c_i_stride) + (j * c_j_stride));
        c[c_index] = total;
        std::cout<<total<<std::endl;
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
    const int a_data[] = {-7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18};
    // Matrix B (RHS) is:
    // |  1 |  3 |  5 |
    // |  2 |  4 |  6 |
    const int b_data[] = {1, 2, 3, 4, 5, 6};
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
    const int expected_data[] = {76, 100, 103, 136, 130, 172, 157, 208};

    const int c_count = m * n;
    std::unique_ptr<int[]> output_data(new int[c_count]);

    
    const int lda = k;
    const int ldb = n;
    const int ldc = n;

   

    ReferenceEightBitIntGemm22( m, n, k, a_data, lda, b_data, ldb, output_data.get(),ldc);

    
    
    return 0;
  
  
  
  
}



